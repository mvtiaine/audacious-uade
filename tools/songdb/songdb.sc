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
def _md5(md5: String) = synchronized {
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

def _base64e(v: Long, minimize: Boolean = false): String =  {
  val chars = Array.fill(8)(45L)
  // 8*6-bit values / base-64 / big-endian
  chars(0) += (v >> 42) & 0x3F
  chars(1) += (v >> 36) & 0x3F
  chars(2) += (v >> 30) & 0x3F
  chars(3) += (v >> 24) & 0x3F
  chars(4) += (v >> 18) & 0x3F
  chars(5) += (v >> 12) & 0x3F
  chars(6) += (v >> 6) & 0x3F
  chars(7) += v & 0x3F
  val res =
    if (minimize) chars.dropWhile(_ == 45).map(_.toChar).mkString
    else chars.map(_.toChar).mkString
  if (res.isEmpty) "\u0045"
  else res
}

def _base64e24(v: Int, minimize: Boolean = true): String =  {
  val chars = Array.fill(4)(45L)
  chars(0) += (v >> 18) & 0x3F
  chars(1) += (v >> 12) & 0x3F
  chars(2) += (v >> 6) & 0x3F
  chars(3) += v & 0x3F
  val res =
    if (minimize) chars.dropWhile(_ == 45).map(_.toChar).mkString
    else chars.map(_.toChar).mkString
  if (res.isEmpty) "\u002D"
  else res
}

def _base64e(md5: String): String =  {
  _base64e(java.lang.Long.parseLong(md5.substring(0,12), 16))
}

def _base64d(base64: String) = {
  var v = 0L
  v |= (base64(0) - 45L) << 42
  v |= (base64(1) - 45L) << 36
  v |= (base64(2) - 45L) << 30
  v |= (base64(3) - 45L) << 24
  v |= (base64(4) - 45L) << 18
  v |= (base64(5) - 45L) << 12
  v |= (base64(6) - 45L) << 6 
  v |= base64(7) - 45L
  v
}

def _base64d24(base64: String) = {
  var v = 0L
  if (base64.length == 4) {
    v |= (base64(0) - 45L) << 18
    v |= (base64(1) - 45L) << 12
    v |= (base64(2) - 45L) << 6
    v |= base64(3) - 45L
  } else if (base64.length == 3) {
    v |= (base64(0) - 45L) << 12
    v |= (base64(1) - 45L) << 6
    v |= base64(2) - 45L
  } else if (base64.length == 2) {
    v |= (base64(0) - 45L) << 6
    v |= base64(1) - 45L
  } else {
    v |= base64(0) - 45L
  }
  v.toInt
}

def _dedup(entries: Iterable[String], file: String) = {
  // keeps original order
  val keys = entries.map(_.split("\t")(0)).toSeq.distinct
  val dedupped = entries.groupBy(_.split("\t")(0)).map(e =>
    if (e._2.size > 1) {
      System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${_md5check(e._1)} entries: ${e._2}")
    }
    (e._1, e._2.toSeq.sortBy(e => e).head)
  )
  keys.map(dedupped).toSeq
}

def _dedupidx(entries: Iterable[String], file: String, strict: Boolean = false) = {
  // keeps original order
  val keys = entries.map(_.split("\t")(0)).toSeq.distinct
  val dedupped = entries.groupBy(_.split("\t")(0)).map(e =>
    if (e._2.size > 1) {
      if (strict) {
        assert(e._2.forall(_ == e._2.head))
      } else {
        System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${e._1} entries: ${e._2}")
      }
    }
    (e._1, e._2.toSeq.sortBy(e =>
      lazy val entries = e.split("\t")
      entries.tail.mkString("###")
    ).head)
  )
  val res = Buffer.empty[String]
  var prev = Array.empty[String]
  for (k <- keys) {
    val dk = dedupped(k)
    val s = dk.split("\t")
    val idx = _md5idx(s.head)
    assert(_base64d24(idx) > 0)
    if (s.tail.sameElements(prev)) {
      res += idx
    } else if (!prev.isEmpty && prev.length <= s.tail.length) {
      val same = Buffer.empty[Int]
      for (i <- prev.indices) {
        if (prev(i) == s.tail(i) && !s.tail(i).isEmpty) {
          same += i
        }
      }
      val tmp = Buffer.empty[String]
      for (i <- s.tail.indices) {
        if (same.contains(i)) {
          tmp += "\u007F"
        } else {
          tmp += s.tail(i)
        }
      }
      res += idx + "\t" + tmp.mkString("\t")
    } else {
      res += idx + "\t" + s.tail.mkString("\t")
    }
    prev = s.tail
  }
  res.toSeq
}

def _md5idxdiff(entries: Seq[String]) = {
  var prevmd5 = 0
  var prevtail = ""
  entries.map(e => {
    val s = e.split("\t")
    val tail = s.tail.mkString("\t")
    val md5v = _base64d24(s(0))
    val res = if (tail == prevtail || tail.isEmpty) {
      assert(md5v > prevmd5)
      val diff = md5v - prevmd5
      (Seq(_base64e24(diff)) ++ s.tail).mkString("\t")
    } else {
      e
    }
    prevmd5 = md5v
    prevtail = tail
    res
  })
}

def _validate(entries: Iterable[String], file: String) = {
  val check = entries.toSeq.map(_.split("\t")(0))
  if (check.size != check.distinct.size) {
    val dups = check.diff(check.distinct).distinct
    throw new IllegalStateException(s"Duplicate entries in ${file}: ${dups}")
  }
}

val _md5idx = scala.collection.mutable.Map[String,String]()
val _idxmd5 = scala.collection.mutable.Map[String,String]()
val md5idxTsv = Future { Files.write(Paths.get("/tmp/songdb/md5idx.tsv"), {
  val entries = songlengths.db.sortBy(_.md5).map(_.md5).distinct
  var prev = 0L
  var md5idx = 1
  // 0 entry is special
  Seq(_base64e(0)) ++ entries.map(md5s => {
    val b64 = _md5(md5s)
    val md5v = _base64d(b64)
    val next = {
      val diff = _base64e(md5v - prev, true)
      assert(diff.length() <= 6)
      assert(diff.length() >= 3)
      diff
    }
    prev = md5v
    val b24 = _base64e24(md5idx, true)
    assert(_md5idx.get(md5s).isEmpty)
    _md5idx(md5s) = b24
    assert(_idxmd5.get(b24).isEmpty)
    _idxmd5(b24) = md5s
    md5idx += 1
    next
  })
}.mkString("\n").concat("\n").getBytes("UTF-8"))}
// needs to be processed first
Await.ready(md5idxTsv, Duration.Inf)

val songlengthsTsv = Future { Files.write(Paths.get("/tmp/songdb/songlengths.tsv"), {
  def songend(s: String) = {
    s match {
      case "player+silence" => "b"
      case "player+volume" => "P"
      case "loop+silence" => "i"
      case "loop+volume" => "L"
      case _ => s.take(1)
    }
  }
  val entries = songlengths.db.sortBy(_.md5).map(e => {
    var prev = ""
    Buffer(
      e.md5,
      e.subsongs.sortBy(_.subsong).map(s => {
        val _songend = if (s.songend != "player") songend(s.songend) else ""
        assert(s.songlength >= 0)
        assert(s.songlength <= 3_600_000)
        assert(s.songlength > 0 || s.songend != "player")
        val sl = if (s.songlength > 0 && s.songlength < 20) 20 else s.songlength
        val _songlength = ""+_base64e24((sl + 10) / 20) // 20ms accuracy in songlengths
        val next = if (_songend.isEmpty) _songlength else _songlength + "," + _songend
        if (prev == next) ""
        else {
          prev = next
          next
        }
      }).mkString(" "),
      if (e.minsubsong != 1) e.minsubsong else "",
    ).mkString("\t")
  }).distinct
  val dedupped = _dedup(entries, "songlengths.tsv")
  _validate(dedupped, "songlengths.tsv")
  assert(dedupped.size == _md5idx.size)
  dedupped.map(_.split("\t").tail.mkString("\t")) // drop md5 as line # == md5idx
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val modinfosTsv = Future { Files.write(Paths.get("/tmp/songdb/modinfos.tsv"), {
  var prev = 0
  val entries = songlengths.db.sortBy(e => e.format + "###" + e.channels + "###" + e.md5).map(e =>
    if (!e.format.isEmpty()) {
      if (e.channels > 0) {
        Buffer(
          e.md5,
          e.format,
          e.channels.toString,
        ).mkString("\t")
      } else {
        Buffer(
          e.md5,
          e.format
        ).mkString("\t")
      }
    } else {
      e.md5
    }
  )
  val dedupped = _dedupidx(entries, "modinfos.tsv", strict=true)
  assert(dedupped.size == _md5idx.size)
  _validate(dedupped, "modinfos.tsv")
  _md5idxdiff(dedupped)
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val modlandTsv = Future { Files.write(Paths.get("/tmp/songdb/modland.tsv"), {
  val entries = sources.modland.flatMap(e =>
    val path = e.path.substring(e.path.indexOf("/") + 1, e.path.lastIndexOf("/"))
    if (path != "- unknown" && path != "_unknown") {
      Some(Buffer(e.md5, path))
    } else None
  ).sortBy(e => e(1) + "###" + e(0)).map(_.mkString("\t")).distinct
  val dedupped = _dedupidx(entries, "modland.tsv")
  _validate(dedupped, "modland.tsv")
  _md5idxdiff(dedupped)
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val unexoticaTsv = Future { Files.write(Paths.get("/tmp/songdb/unexotica.tsv"), {
  val entries = unexotica.metas.map(m =>
    val md5 = m._1
    val path = m._2
    val authoralbum = path.substring(path.indexOf("/") + 1, path.lastIndexOf("/")).split("/")
    val author = authoralbum(0)
    val album = if (authoralbum.size > 1) authoralbum(1) else ""
    val filesize = m._3
    val meta = m._4
    val publisher = meta.group.getOrElse(meta.publisher.getOrElse(meta.team.getOrElse(Right(List(""))))) match {
      case Left(publisher) => publisher
      case Right(publishers) => publishers.head
    }
    val year = meta.year.fold(_.toString, _.toString)
    Buffer(
      md5,
      author,
      publisher,
      album,
      if (year != "Unknown") year else "",
    )
  ).sortBy(e => e(1) + "###" + e(2) + "###" + e(3) + "###" + e(4) + "###" + e(0)).map(_.mkString("\t")).distinct
  val dedupped = _dedupidx(entries, "unexotica.tsv")
  _validate(dedupped, "unexotica.tsv")
  _md5idxdiff(dedupped)
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val ampTsv = Future { Files.write(Paths.get("/tmp/songdb/amp.tsv"), {
  val entries = amp.metas.groupBy(m => (m.md5, m.path)).map({case ((md5, path), m) =>
    var best = m.head
    if (m.size > 1) {
      best = m.maxBy(_.extra_authors.size)
    }
    best
  }).toSeq.flatMap(m =>
    val path = m.path.substring(m.path.indexOf("/") + 1, m.path.lastIndexOf("/"))
    if (path != "UnknownComposers") {
      Some(Buffer(
        m.md5,
        if (m.extra_authors.isEmpty) path else m.extra_authors.sorted.filterNot(_.isEmpty).mkString(" & ")
      ))
    } else None
  ).sortBy(e => e(1) + "###" + e(0)).map(_.mkString("\t")).distinct
  val dedupped = _dedupidx(entries, "amp.tsv")
  _validate(dedupped, "amp.tsv")
  _md5idxdiff(dedupped)
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val demozooTsv = Future { Files.write(Paths.get("/tmp/songdb/demozoo.tsv"), {
  val entries = demozoo.metas.flatMap({case (md5, m) =>
    val dates = Seq(m.modDate, m.prodDate).filterNot(_.isEmpty)
    val row = Buffer(
      md5,
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
    if (row.forall(r => r == md5 || r.trim.isEmpty)) None
    else Some(row)
  }).sortBy(e => e(1) + "###" + e(2) + "###" + e(3) + "###" + e(4) + "###" + e(0)).map(_.mkString("\t")).distinct
  val dedupped = _dedupidx(entries, "demozoo.tsv")
  _validate(dedupped, "demozoo.tsv")
  _md5idxdiff(dedupped)
}.mkString("\n").concat("\n").getBytes("UTF-8"))}

val future = Future.sequence(
  Seq(md5idxTsv, songlengthsTsv, modinfosTsv, modlandTsv, unexoticaTsv, ampTsv, demozooTsv)
)

future onComplete {
  case Failure(e) =>
    e.printStackTrace()
    System.exit(1)
  case Success(value) =>
    System.out.println("Songdb files created to /tmp/songdb/")
}

Await.ready(future, Duration.Inf)
