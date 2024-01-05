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
  val short = md5.substring(0,12)
  if (_md5check.contains(short) && _md5check(short) != md5) {
    System.err.println(s"ERROR: MD5 check failed, short ${short} existing ${_md5check(short)} new ${md5}")
    throw new IllegalStateException
  }
  _md5check(short) = md5
  short
}

def _dedup(entries: Iterable[String], file: String, sortIndex: Int = -1) = {
  // keeps original order
  val keys = entries.map(_.split("\t")(0)).toSeq.distinct
  val dedupped = entries.groupBy(_.split("\t")(0)).map(e =>
    if (e._2.size > 1) {
      System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${e._1} entries: ${e._2}")
    }
    (e._1, e._2.toSeq.sortBy(e =>
      lazy val entries = e.split("\t")
      if (sortIndex >= 0 && entries.size > sortIndex) entries(sortIndex) + e
      else e
    ).head)
  )
  keys.map(k => dedupped(k))
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
      e.format,
      e.channels,
      e.minsubsong,
      e.subsongs.sortBy(_.subsong).map(s =>
        assert(s.songlength >= 0)
        assert(s.songlength <= 3_600_000)
        s"${s.songlength},${s.songend.split("\\+").map(_.take(1)).mkString("+")}"
      ).mkString(" ")
    ).mkString("\t")
  ).distinct
  val dedupped = _dedup(entries, "songlengths.tsv")
  _validate(dedupped, "songlengths.tsv")
  dedupped
}.mkString("\n").getBytes("UTF-8"))}

val modlandTsv = Future { Files.write(Paths.get("/tmp/songdb/modland.tsv"), {
  val entries = sources.modland.sortBy(e => e.path.substring(e.path.indexOf("/") + 1, e.path.length)).flatMap(e =>
    val path = e.path.substring(e.path.indexOf("/") + 1, e.path.lastIndexOf("/"))
    if (path != "- unknown" && path != "_unknown") {
      Some(Buffer(_md5(e.md5), path).mkString("\t"))
    } else None
  ).distinct
  val dedupped = _dedup(entries, "modland.tsv")
  _validate(dedupped, "modland.tsv")
  dedupped
}.mkString("\n").getBytes("UTF-8"))}

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
}.mkString("\n").getBytes("UTF-8"))}

val ampTsv = Future { Files.write(Paths.get("/tmp/songdb/amp.tsv"), {
  val entries = amp.metas.sortBy(_.path).groupBy(m => (m.md5, m.path)).map({case ((md5, path), m) =>
    var best = m.head
    if (m.size > 1) {
      best = m.maxBy(_.extra_authors.size)
    }
    best
  }).toSeq.sortBy(_.path).flatMap(m =>
    val path = m.path.substring(m.path.indexOf("/") + 1, m.path.lastIndexOf("/"))
    if (path != "UnknownComposers") {
      Some(Buffer(
        _md5(m.md5),
        path,
        m.extra_authors.sorted.mkString(",")
      ).mkString("\t"))
    } else None
  ).distinct
  val dedupped = _dedup(entries, "amp.tsv")
  _validate(dedupped, "amp.tsv")
  dedupped
}.mkString("\n").getBytes("UTF-8"))}

val demozooTsv = Future { Files.write(Paths.get("/tmp/songdb/demozoo.tsv"), {
  val entries = demozoo.metas.sortBy(_._2.modDate).flatMap({case (md5, m) =>
    val dates = Seq(m.modDate, m.prodDate).filterNot(_.isEmpty)
    val row = Buffer(
      _md5(md5),
      if (!dates.isEmpty) dates.min.substring(0,4) else "",
      m.authors.sorted.mkString(","),
      ((m.prodPublishers, m.party, m.modPublishers) match {
        case (prod,_,_) if !prod.isEmpty => prod
        case (_,party,_) if !party.isEmpty => party
        case (_,_,mod) if !mod.isEmpty => mod
        case _ => Seq.empty
      }).iterator.to(Seq).sorted.mkString(","),
      m.prod,
      //if (!m.prodPlatforms.isEmpty) m.prodPlatforms.mkString(",") else m.modPlatform
    )
    if (row.forall(r => r == _md5(md5) || r.trim.isEmpty)) None
    else Some(row.mkString("\t"))
  }).distinct
  val dedupped = _dedup(entries, "demozoo.tsv", 1)
  _validate(dedupped, "demozoo.tsv")
  dedupped
}.mkString("\n").getBytes("UTF-8"))}

val future = Future.sequence(
  Seq(songlengthsTsv, modlandTsv, unexoticaTsv, ampTsv, demozooTsv)
)

future onComplete {
  case Failure(e) =>
    e.printStackTrace()
    System.exit(1)
  case Success(value) =>
    System.out.println("Songdb files created to /tmp/songdb/")
}

Await.ready(future, Duration.Inf)
