//> using dep org.scala-lang.modules::scala-parallel-collections::1.0.4
//> using dep net.ruippeixotog::scala-scraper::3.1.0

import java.nio.file.Files
import java.nio.file.Paths
import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._
import scala.jdk.CollectionConverters._
import scala.jdk.StreamConverters._
import scala.util.Using

import net.ruippeixotog.scalascraper.browser.JsoupBrowser
import net.ruippeixotog.scalascraper.dsl.DSL._
import net.ruippeixotog.scalascraper.dsl.DSL.Extract._
import net.ruippeixotog.scalascraper.dsl.DSL.Parse._
import net.ruippeixotog.scalascraper.model._

val amp_path = System.getProperty("user.home") + "/AMP/"

case class AMPMod (
  id: Int,
  md5: String,
  path: String,
  filesize: Int,
)

lazy val amp_by_path = sources.amp.groupBy(_.path.toLowerCase)

lazy val amp_mods = Files.list(Paths.get(amp_path + "downmod/")).toScala(Buffer).par.map(f =>
  val loc = Using(scala.io.Source.fromFile(f.toFile())(scala.io.Codec.UTF8))( _.getLines.find(_.startsWith("location:"))).get
  if (loc.isDefined) {
    val url = loc.get.replace("location: ","")
    val path = java.net.URLDecoder.decode(url,"UTF-8").replaceAll("http[s]?://amp.dascene.net/modules/","").replace(".gz","")
    if (amp_by_path.contains(path.toLowerCase)) {
      val e = amp_by_path(path.toLowerCase).head
      Some(AMPMod(f.toString().split("=").last.toInt, e.md5, path, e.filesize))
    } else None
  } else None
).flatten.seq

case class AMPMeta (
  md5: String,
  path: String,
  filesize: Int,
  extra_authors: List[String],
)

lazy val amp_mods_by_id = amp_mods.groupBy(_.id)

val seenIds = scala.collection.mutable.Set[Int]()
lazy val metas = Files.list(Paths.get(amp_path + "detail/")).toScala(Buffer).par.map(f =>
  val doc = JsoupBrowser().parseFile(f.toFile)
  val data = doc >> elementList("#result")
  if (data.length > 1) {
    val bar = data(1) >> elementList("table tbody tr[class^=\"tr\"]")
    val ids = bar >> attrs("href")("td a[href^=\"downmod.php\"]")
    val authors = bar >> texts("td a[href^=\"detail.php\"]")
    ids.lazyZip(authors).filterNot(_._1.isEmpty).flatMap({case (idlink, authors) =>
      val id = idlink.head.trim.split("=").last.toInt
      if (!seenIds.contains(id) && amp_mods_by_id.contains(id)) {
        seenIds += id
        val e = amp_mods_by_id(id).head
        val extra_authors = if (authors.size == 1) List.empty else authors.toList
        Some(AMPMeta(e.md5, e.path, e.filesize, extra_authors))
      } else None
    })
  } else Iterable.empty[AMPMeta]
).flatten.distinct.seq
