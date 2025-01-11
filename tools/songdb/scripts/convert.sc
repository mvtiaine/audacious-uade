// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

//> using dep org.scala-lang.modules::scala-parallel-collections::1.2.0

import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._
import scala.reflect.ClassTag
import scala.util.Using

import md5._
import dedup._

val SEPARATOR = "\u007E" // ~
val SORT = "\u0001"

case class SubsongInfo (
  songlength: Int, // in ms
  songend: String, // ...
)

trait BaseInfo {
  def md5: String // 48-bits of md5 hex encoded (lower case)
  def copyWithMd5(newMd5: String): BaseInfo
}

case class SongInfo (
  override val md5: String, // 48-bits of md5 hex encoded (lower case)
  minsubsong: Int,
  subsongs: Buffer[SubsongInfo],
) extends BaseInfo {
  override def copyWithMd5(newMd5: String) = copy(md5 = newMd5)
  override def toString = s"SongInfo(${md5},${minsubsong},${subsongs.mkString(",")})"
}

case class ModInfo (
  override val md5: String, // 48-bits of md5 hex encoded (lower case)
  format: String, // tracker or player name (freeform)
  channels: Int, // channels or 0 if not known/defined
) extends BaseInfo {
  override def copyWithMd5(newMd5: String) = copy(md5 = newMd5)
}

case class MetaData (
  override val md5: String, // 48-bits of md5 hex encoded (lower case)
  authors: Buffer[String],
  publishers: Buffer[String],
  album: String,
  year: Int,
) extends BaseInfo {
  override def copyWithMd5(newMd5: String) = copy(md5 = newMd5)
  override def toString =
    s"MetaData(${md5},${authors.mkString(",")},${publishers.mkString(",")},${album},${year})"
}

def decodeMd5IdxTsv(tsv: String) = {
  val idx2md5 = Buffer[String]() // idx -> 48-bit md5
  val rows = tsv.split('\n')
  assert(rows(0) == base64e(0)) // 0 entry is special
  var prevmd5 = 0L
  rows.tail.foreach { b64 =>
    val md5 = prevmd5 + base64d(b64)
    assert(prevmd5 == 0 || md5 > prevmd5)
    idx2md5 += f"${md5}%012x"
    prevmd5 = md5
  }
  idx2md5
}

def encodeMd5IdxTsv(idx2md5: Buffer[String]) = {
  val res = Buffer(base64e(idx2md5.head)) // 0 entry is special
  var prev = 0L
  idx2md5.tail.foreach { md5 =>
    val b64 = base64e(md5)
    val md5v = base64d(b64)
    val diff = base64e(md5v - prev, true)
    assert(diff.length() <= 6)
    assert(diff.length() >= 2)
    prev = md5v
    res += diff
  }
  res.mkString("\n").concat("\n")
}

def decodeSonglengthsTsv(tsv: String, idx2md5: Buffer[String]) = {
  assert(idx2md5.nonEmpty)
  def decodeRow(cols: Array[String], md5: String) = {
    def decodeSongend(songend: String) : String = songend match {
      case "e" => "error"
      case "p" => "player"
      case "t" => "timeout"
      case "s" => "silence"
      case "l" => "loop"
      case "v" => "volume"
      case "r" => "repeat"
      case "b" => "player+silence"
      case "P" => "player+volume"
      case "i" => "loop+silence"
      case "L" => "loop+volume"
      case "n" => "nosound"
      case _ => assert(false)
    }
    val subsongs = Buffer.empty[SubsongInfo]
    val minsubsong = if (cols.length == 1 || cols(1).isEmpty) 1 else cols(1).toInt

    var prev = SubsongInfo(0, "error")
    cols(0).split(" ", -1).foreach(ss =>
      if (ss.isEmpty()) {
        assert(prev != null)
      } else {
        val e = ss.split(',')
        val songlength =
          if (e(0).isEmpty) 0
          else base64d24(e(0)) * 20 // 20ms accuracy in encoded tsv
        val songend =
          if (e.length == 1 || e(1).isEmpty) "player"
          else decodeSongend(e(1))
        val subsong = SubsongInfo(songlength, songend)
        prev = subsong
      }
      subsongs += prev
    )
    SongInfo(md5, minsubsong, subsongs)
  }
  val songlengths = tsv.split('\n').zipWithIndex.par.map {
    // 0 entry is special in md5 list
    case (row, index) => decodeRow(row.split('\t'), idx2md5(index + 1))
  }
  assert(songlengths.map(_.md5).distinct.size == songlengths.size)
  songlengths.toBuffer.sortBy(_.md5)
}

def encodeSonglengthsTsv(songlengths: Buffer[SongInfo]) = {
  assert(songlengths.map(_.md5).distinct.size == songlengths.size)
  def encode(songend: String) : String = songend match {
      case "player+silence" => "b"
      case "player+volume" => "P"
      case "loop+silence" => "i"
      case "loop+volume" => "L"
      case _ => songend.take(1)
  }
  val entries = songlengths.sortBy(_.md5).par.map(e =>
    var prev = ""
    val subsongs = e.subsongs.map(s =>
      val _songend = if (s.songend != "player") encode(s.songend) else ""
      val sl = if (s.songlength > 0 && s.songlength < 20) 20 else s.songlength
      val _songlength = ""+base64e24((sl + 10) / 20) // 20ms accuracy in songlengths
      val next = if (_songend.isEmpty) _songlength else _songlength + "," + _songend
      if (prev == next) ""
      else {
        prev = next
        next
      }
    )
    val minsubsong = if (e.minsubsong != 1) e.minsubsong.toString else ""
    Buffer(e.md5.take(12), subsongs.mkString(" "), minsubsong)
  ).map(_ match {
    case Buffer(md5, "", "") => Buffer(md5)
    case Buffer(md5, subsongs, "") => Buffer(md5, subsongs)
    case b => b
  }).seq
  val dedupped = dedup(entries, "songlengths.tsv")
  validate(dedupped, "songlengths.tsv")
  // drop md5 as line # == md5idx
  dedupped.map(b => b.tail.mkString("\t")).mkString("\n").concat("\n")
}

def decodeTsv[T <: BaseInfo : ClassTag] (
  tsv: String,
  idx2md5: Buffer[String],
  decodeRow: (Array[String], String, T) => T,
  initial: T
): Buffer[T] = {
  assert(idx2md5.nonEmpty)
  var idx = 0
  var info = initial
  var i = 0
  val split = tsv.split('\n')
  val infos = split.zipWithIndex.map {
    case (row, index) =>
      val cols = row.split('\t')
      val next_idx = base64d24(cols(0))
      if (cols.length == 1) {
        idx += next_idx
        info = info.copyWithMd5(idx2md5(idx)).asInstanceOf[T]
      } else {
        idx = next_idx
        info = decodeRow(cols, idx2md5(idx), info)
      }
      i += 1
      assert(idx < idx2md5.size)
      info
  }
  assert(infos.map(_.md5).distinct.size == split.size)
  infos.toBuffer.sortBy(_.md5)
}

def decodeModInfosTsv(tsv: String, idx2md5: Buffer[String]) = {
  def decodeRow(cols: Array[String], md5: String, prev: ModInfo) = {
    val format = if (cols.length > 1) {
      if (cols(1) == REPEAT) prev.format
      else cols(1)
    } else ""

    val channels = if (cols.length > 2) {
      if (cols(2) == REPEAT) prev.channels
      else cols(2).toInt
    } else 0

    ModInfo(md5, format, channels)
  }
  decodeTsv(tsv, idx2md5, decodeRow, ModInfo("", "", 0))
}

def encodeModInfosTsv(modinfos: Buffer[ModInfo]) = {
  assert(modinfos.map(_.md5).distinct.size == modinfos.size)
  def sorter = (e: ModInfo) => e.format + SORT + e.channels + SORT + e.md5
  val infos = modinfos.sortBy(sorter).par.map(m =>
    Buffer(
      m.md5.take(12),
      m.format,
      if (m.channels > 0) m.channels.toString else ""
    )
  ).seq
  val dedupped = dedupidx(infos, "modinfos.tsv", strict=true)
  validate(dedupped, "modinfos.tsv")
  md5idxdiff(dedupped).map(_.mkString("\t").trim).mkString("\n").concat("\n")
}

def decodeMetaTsv(tsv: String, idx2md5: Buffer[String]) = {
  def decodeRow(cols: Array[String], md5: String, prev: MetaData) = {
    val authors = if (cols.length > 1) {
      if (cols(1) == REPEAT) prev.authors
      else if (cols(1).isEmpty) Buffer.empty
      else cols(1).split(SEPARATOR).toBuffer
    } else Buffer.empty
    
    val publishers = if (cols.length > 2) {
      if (cols(2) == REPEAT) prev.publishers
      else if (cols(2).isEmpty) Buffer.empty
      else cols(2).split(SEPARATOR).toBuffer
    } else Buffer.empty

    val album = if (cols.length > 3) {
      if (cols(3) == REPEAT) prev.album
      else cols(3)
    } else ""

    val year = if (cols.length > 4) {
      if (cols(4) == REPEAT) prev.year
      else cols(4).toInt
    } else 0

    MetaData(md5, authors, publishers, album, year)
  }
  decodeTsv(tsv, idx2md5, decodeRow, MetaData("", Buffer.empty, Buffer.empty, "", 0))
}

def encodeMetaTsv(meta: Buffer[MetaData], name: String) = {
  def sorter = (e: MetaData) =>
    e.authors.mkString(SEPARATOR) + SORT +
    e.publishers.mkString(SEPARATOR) + SORT +
    e.album + SORT + 
    e.year + SORT +
    e.md5
  val infos = meta.sortBy(sorter).par.map(i =>
    Buffer(
      i.md5.take(12),
      i.authors.mkString(SEPARATOR),
      i.publishers.mkString(SEPARATOR),
      i.album,
      if (i.year > 0) i.year.toString else ""
    )
  ).seq
  val dedupped = dedupidx(infos, name)
  validate(dedupped, name)
  md5idxdiff(dedupped).map(_.mkString("\t").trim).mkString("\n").concat("\n")
}
