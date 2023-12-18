
import java.nio.file.Files
import java.nio.file.Paths
import scala.collection.mutable.Buffer
import scala.jdk.CollectionConverters._
import scala.jdk.StreamConverters._
import scala.util.Try
import scala.util.Using

enum Precision:
  case UNKNOWN, YEAR, MONTH, DATE

case class DemozooMeta (
  id: Int,
  prodId: Option[Int],
  modDate: String,
  modDatePrecision: Precision,
  prodDate: String,
  prodDatePrecision: Precision,
  modPlatform: String,
  prodPlatforms: Seq[String],
  prod: String,
  authors: Seq[String],
  modPublishers: Seq[String],
  prodPublishers: Seq[String],
  imageUrls: Seq[String],
  party: Option[String],
  partyDate: Option[String],
  partyDatePrecision: Option[Precision],
)

lazy val ml_by_path = sources.modland.groupBy(_.path)

lazy val metas = Using(scala.io.Source.fromFile("sources/demozoo.tsv"))(_.getLines.flatMap(line =>
  def split(s: String) = s.replaceFirst("\\{","").replaceAll("\\}$","").split(",").filterNot(_ == "NULL")
  def trim(s: String) = s.trim.replaceFirst("\"","").replaceAll("\"$","")
  def precision(s: String) = s match {
    case "y" => Precision.YEAR
    case "m" => Precision.MONTH
    case "d" => Precision.DATE
    case _ => Precision.UNKNOWN
  }

  val l = line.split("\t")
  val id = l(0).toInt
  val prodId = l(1).toIntOption
  val modDate = l(2)
  val modDatePrecision = precision(l(3))
  val prodDate = l(4)
  val prodDatePrecision = precision(l(5))
  val modPlatform = l(6)
  val prodPlatforms = split(l(7)) map trim
  val prod = l(8)
  val url = l(9)
  val authors = split(l(10)) map trim
  val modPublishers = split(l(11)) map trim
  val prodPublishers = split(l(12)) map trim
  val imageUrls = split(l(13))
  val party = if (l.length > 14) Some(l(14)) else None
  val partyDate = if (l.length > 15) Some(l(15)) else None
  val partyDatePrecision = if (l.length > 16) Some(precision(l(16))) else None

  val meta = DemozooMeta(id, prodId, modDate, modDatePrecision, prodDate, prodDatePrecision,
    modPlatform, prodPlatforms.toSeq, prod, authors.toSeq, modPublishers.toSeq, prodPublishers.toSeq, imageUrls.toSeq,
    party, partyDate, partyDatePrecision)

  if (url.contains("://amp.dascene.net/downmod.php?index=")) {
    val id = url.split("=").last.toInt
    if (amp.amp_mods_by_id.contains(id)) {
      val md5 = amp.amp_mods_by_id(id).head.md5
      Some(md5, meta)
    } else None
  } else if (url.contains("://amp.dascene.net/modules/")) {
    // url should have been decoded already
    val path = url.replaceAll("http[s]?://amp.dascene.net/modules/","").replace(".gz","")
    if (amp.amp_by_path.contains(path)) {
      val md5 = amp.amp_by_path(path).head.md5
      Some(md5, meta)
    } else None
  } else if (url.startsWith("/pub/modules/")) {
    val path = url.replaceFirst("/pub/modules/", "")
    if (ml_by_path.contains(path)) {
      val md5 = ml_by_path(path).head.md5
      Some(md5, meta)
    } else None
  } else None

).distinct.toBuffer).get.groupBy(_._1).flatMap({case (md5, metas) =>
  var best: Option[DemozooMeta] = None
  if (metas.size == 1) {
    val meta = metas.head._2
    if (meta.prodId.isEmpty) {
      best = Some(meta)
    } else if (meta.prodDatePrecision.ordinal >= Precision.MONTH.ordinal && meta.modDatePrecision.ordinal >= Precision.MONTH.ordinal &&
               meta.prodDate.substring(0,7) <= meta.modDate.substring(0,7)) {               
      best = Some(meta)
    }
  } else for (meta <- metas.map(_._2)) {
    if (best.isDefined) {
    } else if (meta.modDate == meta.prodDate && meta.modDatePrecision == meta.prodDatePrecision) {
      best = Some(meta)
    } else if (meta.modDate == meta.partyDate.getOrElse("") &&
               meta.modDatePrecision == meta.partyDatePrecision.getOrElse(Precision.UNKNOWN)) {
      best = Some(meta)
    }
  }

  if (!best.isDefined) for (meta <- metas.map(_._2)) {
    if (best.isDefined || !meta.prodId.isDefined) {
    } else if (meta.prodDatePrecision.ordinal >= Precision.MONTH.ordinal && meta.modDatePrecision.ordinal >= Precision.MONTH.ordinal &&
               meta.prodDate.substring(0,7) <= meta.modDate.substring(0,7)) {
      best = Some(meta)
    }
  }
  if (best.isDefined && metas.size > 1 && best.get.prodId.isDefined) for (meta <- metas.map(_._2)) {
    if (best.isDefined && meta.prodId.isDefined &&
        meta.prodDatePrecision.ordinal >= Precision.MONTH.ordinal && best.get.prodDatePrecision.ordinal >= Precision.MONTH.ordinal &&
        meta.prodDatePrecision.ordinal >= best.get.prodDatePrecision.ordinal &&
        meta.prodDate < best.get.prodDate) {
      best = None
    }
  }
  if (!best.isDefined){
    best = metas.map(_._2).filter(_.prodId.isEmpty).headOption
    if (!best.isDefined) {
      val prodDates = metas.map(_._2).filterNot(_.prodDate.isEmpty).map(_.prodDate)
      val partyDates = metas.map(_._2).filterNot(d => d.partyDate.isEmpty || d.partyDate.get.isEmpty()).map(_.partyDate.get)
      val minProdDate = if (prodDates.isEmpty) "" else prodDates.min
      val minPartyDate = if (partyDates.isEmpty) "" else partyDates.min
      best = metas.map(_._2).headOption.map(m =>
        val earliestDate = Seq(minProdDate, minPartyDate, m.modDate).filterNot(_.isEmpty).min
        m.copy(prodId = None, prod = "", prodPublishers = Seq.empty, modDate = earliestDate))
    }
  }
  best.map((md5, _))
}).toSeq
