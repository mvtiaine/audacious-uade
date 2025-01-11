#!/usr/bin/env -S scala-cli shebang

// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

//> using file scripts/md5.sc
//> using file scripts/dedup.sc
//> using file scripts/convert.sc
//> using file scripts/pretty.sc

//> using file scripts/sources.sc
//> using file scripts/songlengths.sc
//> using file scripts/unexotica.sc
//> using file scripts/amp.sc
//> using file scripts/demozoo.sc
//> using file scripts/modland.sc

import java.nio.file.Files
import java.nio.file.Paths
import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._
import scala.concurrent.Await
import scala.concurrent.duration.Duration
import scala.concurrent.Future
import scala.util.Success
import scala.util.Failure

import md5._
import dedup._
import convert._
import pretty._

implicit val ec: scala.concurrent.ExecutionContext = scala.concurrent.ExecutionContext.global

// 0 entry is special
lazy val idx2md5 = Buffer("0" * 12) ++ songlengths.db.sortBy(_.md5).map(_.md5.take(12)).distinct

def _try[T](f: => T) = try {
  f
} catch {
  case e: Throwable =>
    e.printStackTrace()
    throw e
}

lazy val md5idxTsv = Future(_try {
  idx2md5.zipWithIndex.foreach { case (md5s, idx) =>
    val b64 = md5(md5s)
    val md5v = base64d(b64)
    val b24 = base64e24(idx, true)
    assert(_md5idx.get(md5s).isEmpty)
    _md5idx(md5s) = b24
    assert(_idxmd5.get(b24).isEmpty)
    _idxmd5(b24) = md5s
  }

  val encoded = encodeMd5IdxTsv(idx2md5)
  Files.write(Paths.get("/tmp/songdb/md5idx.tsv"), encoded.getBytes("UTF-8"))
})

lazy val songlengthsTsvs = Future(_try {
  val entries = songlengths.db.sortBy(_.md5).par.map(e =>
    SongInfo(
      e.md5.take(12),
      e.minsubsong,
      e.subsongs.sortBy(_.subsong).map(s =>
        SubsongInfo(
          s.songlength,
          s.songend,
        )
      ).toBuffer
    )
  ).toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeSonglengthsTsv(entries)
  val decoded = decodeSonglengthsTsv(encoded, idx2md5)
  val pretty = createPrettySonglengthsTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/songlengths.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/songlengths.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettySonglengthsTsv(pretty))
  assert(encoded == encodeSonglengthsTsv(decoded))
})

lazy val modinfosTsvs = Future(_try {
  val entries = songlengths.db.sortBy(_.md5).par.map { e =>
    ModInfo(
      e.md5.take(12),
      e.format,
      e.channels
    )
  }.toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeModInfosTsv(entries)
  val decoded = decodeModInfosTsv(encoded, idx2md5)
  val pretty = createPrettyModInfosTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/modinfos.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/modinfos.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettyModInfosTsv(pretty))
  assert(encoded == encodeModInfosTsv(decoded))
})

lazy val ampTsvs = Future(_try {
  val entries = amp.metas.groupBy(m => (m.md5, m.path)).par.map { case ((md5, path), m) =>
    var best = m.head
    if (m.size > 1) {
      best = m.maxBy(_.extra_authors.size)
    }
    best
  }.par.flatMap(m =>
    val path = m.path.substring(m.path.indexOf("/") + 1, m.path.lastIndexOf("/"))
    if (path != "UnknownComposers") {
      Some(AMPInfo(
        m.md5.take(12),
        if (m.extra_authors.isEmpty) Buffer(path)
        else m.extra_authors.sorted.filterNot(_.isEmpty).toBuffer
      ))
    } else None
  ).toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeAMPTsv(entries)
  val decoded = decodeAMPTsv(encoded, idx2md5)
  val pretty = createPrettyAMPTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/amp.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/amp.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettyAMPTsv(pretty))
  assert(encoded == encodeAMPTsv(decoded))
})

lazy val modlandTsvs = Future(_try {
  val entries = sources.modland.sortBy(_.md5).par.flatMap { e =>
    val path = e.path.substring(e.path.indexOf("/") + 1, e.path.lastIndexOf("/"))
    if (path != "- unknown" && path != "_unknown") {
      modland.parseModlandAuthorAlbum(path).map { case (authors, album) =>
        ModlandInfo(
          e.md5.take(12),
          authors.sorted.toBuffer,
          album
        )
      }
    } else None
  }.toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeModlandTsv(entries)
  val decoded = decodeModlandTsv(encoded, idx2md5)
  val pretty = createPrettyModlandTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/modland.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/modland.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettyModlandTsv(pretty))
  assert(encoded == encodeModlandTsv(decoded))
})

lazy val unexoticaTsvs = Future(_try {
  val entries = unexotica.metas.par.map { m =>
    val md5 = m._1
    val path = m._2
    val authorAlbum = path.substring(path.indexOf("/") + 1, path.lastIndexOf("/")).split("/")
    val authors = Buffer(unexotica.transformAuthor(authorAlbum(0)))
    val filesize = m._3
    val meta = m._4
    val album = unexotica.transformAlbum(meta, authorAlbum)
    val publishers = Buffer(unexotica.transformPublisher(meta))
    val year = meta.year.fold(_.toString, _.toString)
    FullInfo(
      md5.take(12),
      authors,
      publishers,
      album,
      if (year != "Unknown") year.toInt else 0
    )
  }.toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeGenericTsv(entries, "unexotica.tsv")
  val decoded = decodeGenericTsv(encoded, idx2md5)
  val pretty = createPrettyGenericTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/unexotica.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/unexotica.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettyGenericTsv(pretty))
  assert(encoded == encodeGenericTsv(decoded, "unexotica.tsv"))
})

lazy val demozooTsvs = Future(_try {
  val entries = demozoo.metas.par.flatMap { case (md5, m) =>
    val dates = Seq(m.modDate, m.prodDate).filterNot(_.isEmpty)
    val authors = m.authors.filterNot(_ == "?").sorted.toBuffer
    val info = FullInfo(
      md5.take(12),
      if (authors.forall(_.trim.isEmpty)) Buffer.empty else authors,
      ((m.prodPublishers, m.party, m.modPublishers) match {
        case (prod,_,_) if !prod.isEmpty =>
          if (prod.forall(_.trim.isEmpty)) Buffer.empty else prod.toBuffer
        case (_,party,_) if !party.isEmpty => Buffer(party.get)
        case (_,_,mod) if !mod.isEmpty =>
          if (mod.forall(_.trim.isEmpty)) Buffer.empty else mod.toBuffer
        case _ => Buffer.empty
      }).sorted,
      m.prod.trim,
      if (!dates.isEmpty) dates.min.substring(0,4).toInt else 0
      //if (!m.prodPlatforms.isEmpty) m.prodPlatforms else m.modPlatform
    )
    info match {
      case FullInfo(_, Buffer(), Buffer(), "", 0) => None
      case _ => Some(info)
    }
  }.toBuffer.distinct

  // encoding does also deduplication
  val encoded = encodeGenericTsv(entries, "demozoo.tsv")
  val decoded = decodeGenericTsv(encoded, idx2md5)
  val pretty = createPrettyGenericTsv(decoded)
  Files.write(Paths.get("/tmp/songdb/demozoo.tsv"), encoded.getBytes("UTF-8"))
  Files.write(Paths.get("/tmp/songdb/pretty/demozoo.tsv"), pretty.getBytes("UTF-8"))
  assert(decoded == parsePrettyGenericTsv(pretty))
  assert(encoded == encodeGenericTsv(decoded, "demozoo.tsv"))
})

// needs to be processed first
Await.ready(md5idxTsv, Duration.Inf)
val future = Future.sequence(
  Seq(md5idxTsv, songlengthsTsvs, modinfosTsvs, ampTsvs, modlandTsvs, unexoticaTsvs, demozooTsvs)
)

future onComplete {
  case Failure(e) =>
    e.printStackTrace()
    System.exit(1)
  case Success(value) =>
    System.out.println("Songdb files created to /tmp/songdb/")
}

Await.ready(future, Duration.Inf)
