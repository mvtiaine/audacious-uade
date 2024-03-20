#!/usr/bin/env -S scala-cli shebang

//> using file scripts/sources.sc
//> using file scripts/songlengths.sc
//> using file scripts/unexotica.sc
//> using file scripts/amp.sc
//> using file scripts/demozoo.sc

import java.nio.file.Files
import java.nio.file.Paths
import scala.collection.mutable.Buffer
import scala.concurrent.Await
import scala.concurrent.duration.Duration
import scala.concurrent.Future
import scala.util.Success
import scala.util.Failure

implicit val ec: scala.concurrent.ExecutionContext = scala.concurrent.ExecutionContext.global

val _md5check = scala.collection.mutable.Map[String,String]()
def _md5(md5: String) = {
  val base64 = _base64e(md5)
  if (_md5check.contains(base64) && _md5check(base64) != md5) {
    System.err.println(s"ERROR: MD5 check failed, short ${base64} existing ${_md5check(base64)} new ${md5}")
    throw new IllegalStateException
  }
  _md5check(base64) = md5
  val md5v = java.lang.Long.parseLong(md5.substring(0,12), 16)
  if (_base64d(base64) != md5v) {
    System.err.println(s"ERROR: MD5 vs base64 check failed for MD5:${md5.substring(0,12)}/${md5v} base64: ${base64}/${_base64d(base64)}")
    throw new IllegalStateException
  }
  base64
}

def _base64e(md5: String) =  {
  // convert to 6-bit/base-64 encoding (NOT "proper base64"!)
  // 4*12-bit values
  val v1 = Integer.parseInt(md5.substring(0,3), 16)
  val v2 = Integer.parseInt(md5.substring(3,6), 16)
  val v3 = Integer.parseInt(md5.substring(6,9), 16)
  val v4 = Integer.parseInt(md5.substring(9,12), 16)
  // values start from 45 ('-')
  val chars = Array.fill(8)(45)
  // 8*6-bit values / base-64 / big-endian
  chars(0) += (v1 >> 6) & 0x3F
  chars(1) += v1 & 0x3F
  chars(2) += (v2 >> 6) & 0x3F
  chars(3) += v2 & 0x3F
  chars(4) += (v3 >> 6) & 0x3F
  chars(5) += v3 & 0x3F
  chars(6) += (v4 >> 6) & 0x3F
  chars(7) += v4 & 0x3F
  chars.map(_.toChar).mkString
}

def _base64d(base64: String) = {
  var v = 0L
  v |= ((base64(0) - 45L) << 42)
  v |= ((base64(1) - 45L) << 36)
  v |= ((base64(2) - 45L) << 30)
  v |= ((base64(3) - 45L) << 24)
  v |= ((base64(4) - 45L) << 18)
  v |= ((base64(5) - 45L) << 12)
  v |= ((base64(6) - 45L) << 6) 
  v |= base64(7) - 45L
  v
}
def _dedup(entries: Iterable[String], file: String, sortIndex: Int = -1, minimize: Boolean = true) = {
  // keeps original order
  val keys = entries.map(_.split("\t")(0)).toSeq.distinct
  val dedupped = entries.groupBy(_.split("\t")(0)).map(e =>
    if (e._2.size > 1) {
      System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${_md5check(e._1)} entries: ${e._2}")
    }
    (e._1, e._2.toSeq.sortBy(e =>
      lazy val entries = e.split("\t")
      if (sortIndex >= 0 && entries.size > sortIndex) entries(sortIndex) + e
      else e
    ).head)
  )
  if (minimize) {
    val res = Buffer.empty[String]
    var prev = ""
    for (k <- keys) {
      val dk = dedupped(k)
      val s = dk.split("\t")
      val tail = s.tail.mkString("\t")
      if (tail == prev) {
        res += s.head
      } else {
        res += dk
      }
      prev = tail
    }
    res.toSeq
  } else keys.map(dedupped)
}

def _validate(entries: Iterable[String], file: String) = {
  val check = entries.toSeq.map(_.split("\t")(0))
  if (check.size != check.distinct.size) {
    val dups = check.diff(check.distinct).distinct
    throw new IllegalStateException(s"Duplicate entries in ${file}: ${dups}")
  }
}

val songlengthsTsv = Future { Files.write(Paths.get("/tmp/songdb/songlengths.tsv"), {
  val entries = songlengths.db.sortBy(_.md5).map(e =>
    Buffer(
      _md5(e.md5),
      e.minsubsong,
      e.subsongs.sortBy(_.subsong).map(s =>
        assert(s.songlength >= 0)
        assert(s.songlength <= 3_600_000)
        s"${s.songlength},${s.songend.split("\\+").map(_.take(1)).mkString("+")}"
      ).mkString(" ")
    ).mkString("\t")
  ).distinct
  val dedupped = _dedup(entries, "songlengths.tsv", minimize = false)
  _validate(dedupped, "songlengths.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val modinfosTsv = Future { Files.write(Paths.get("/tmp/songdb/modinfos.tsv"), {
  val entries = songlengths.db.sortBy(e => e.format + "###" + e.channels).flatMap(e =>
    if (!e.format.isEmpty() && e.format != "???") {
      Some(Buffer(
        _md5(e.md5),
        e.format,
        e.channels,
      ).mkString("\t"))
    } else None
  ).distinct
  val dedupped = _dedup(entries, "modinfos.tsv")
  _validate(dedupped, "modinfos.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val modlandTsv = Future { Files.write(Paths.get("/tmp/songdb/modland.tsv"), {
  val entries = sources.modland.sortBy(e => e.path.substring(e.path.indexOf("/") + 1, e.path.length)).flatMap(e =>
    val path = e.path.substring(e.path.indexOf("/") + 1, e.path.lastIndexOf("/"))
    if (path != "- unknown" && path != "_unknown") {
      Some(Buffer(_md5(e.md5), path))
    } else None
  ).sortBy(_(1)).map(_.mkString("\t")).distinct
  val dedupped = _dedup(entries, "modland.tsv")
  _validate(dedupped, "modland.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val unexoticaTsv = Future { Files.write(Paths.get("/tmp/songdb/unexotica.tsv"), {
  val entries = unexotica.metas.sortBy(_._2).map(m =>
    val md5 = m._1
    val path = m._2
    val filesize = m._3
    val meta = m._4
    val publisher = meta.group.getOrElse(meta.publisher.getOrElse(meta.team.getOrElse(Right(List(""))))) match {
      case Left(publisher) => publisher
      case Right(publishers) => publishers.head
    }
    val year = meta.year.fold(_.toString, _.toString)
    Buffer(
      _md5(md5),
      path.substring(path.indexOf("/") + 1, path.lastIndexOf("/")),
      publisher,
      if (year != "Unknown") year else "",
    ).mkString("\t")
  ).distinct
  val dedupped = _dedup(entries, "unexotica.tsv", 3)
  _validate(dedupped, "unexotica.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val ampTsv = Future { Files.write(Paths.get("/tmp/songdb/amp.tsv"), {
  val entries = amp.metas.sortBy(_.path).groupBy(m => (m.md5, m.path)).map({case ((md5, path), m) =>
    var best = m.head
    if (m.size > 1) {
      best = m.maxBy(_.extra_authors.size)
    }
    best
  }).toSeq.flatMap(m =>
    val path = m.path.substring(m.path.indexOf("/") + 1, m.path.lastIndexOf("/"))
    if (path != "UnknownComposers") {
      Some(Buffer(
        _md5(m.md5),
        if (m.extra_authors.isEmpty) path else m.extra_authors.sorted.filterNot(_.isEmpty).mkString(" & ")
      ))
    } else None
  ).sortBy(_(1)).map(_.mkString("\t")).distinct
  val dedupped = _dedup(entries, "amp.tsv")
  _validate(dedupped, "amp.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val demozooTsv = Future { Files.write(Paths.get("/tmp/songdb/demozoo.tsv"), {
  val entries = demozoo.metas.flatMap({case (md5, m) =>
    val dates = Seq(m.modDate, m.prodDate).filterNot(_.isEmpty)
    val row = Buffer(
      _md5(md5),
      m.authors.sorted.mkString(","),
      ((m.prodPublishers, m.party, m.modPublishers) match {
        case (prod,_,_) if !prod.isEmpty => prod
        case (_,party,_) if !party.isEmpty => party
        case (_,_,mod) if !mod.isEmpty => mod
        case _ => Seq.empty
      }).iterator.to(Seq).sorted.mkString(","),
      m.prod,
      if (!dates.isEmpty) dates.min.substring(0,4) else "",
      //if (!m.prodPlatforms.isEmpty) m.prodPlatforms.mkString(",") else m.modPlatform
    )
    if (row.forall(r => r == _md5(md5) || r.trim.isEmpty)) None
    else Some(row.mkString("\t"))
  }).distinct.sortBy(_.substring(8))
  val dedupped = _dedup(entries, "demozoo.tsv", 1)
  _validate(dedupped, "demozoo.tsv")
  dedupped
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val future = Future.sequence(
  Seq(songlengthsTsv, modinfosTsv, modlandTsv, unexoticaTsv, ampTsv, demozooTsv)
)

future onComplete {
  case Failure(e) =>
    e.printStackTrace()
    System.exit(1)
  case Success(value) =>
    System.out.println("Songdb files created to /tmp/songdb/")
}

Await.ready(future, Duration.Inf)
