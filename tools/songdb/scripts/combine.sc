// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

//> using dep org.scala-lang.modules::scala-parallel-collections::1.2.0

import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._

import convert._

def combineMetadata(
    amp: Buffer[MetaData],
    modland: Buffer[MetaData],
    unexotica: Buffer[MetaData],
    demozoo: Buffer[MetaData]
) = {
  val md5s = (amp.map(_.md5) ++ modland.map(_.md5) ++ unexotica.map(_.md5) ++ demozoo.map(_.md5)).toSet

  val ag = amp.groupBy(_.md5).par.mapValues(_.head)
  // canonize Falcon (PL) -> Falcon etc.
  val mg = modland.groupBy(_.md5).par.mapValues(v => v.head.copy(
    authors = v.head.authors.map(_.replaceAll(" \\(.*\\)$", "")))).seq
  val dg = demozoo.groupBy(_.md5).par.mapValues(_.head)
  val ug = unexotica.groupBy(_.md5).par.mapValues(_.head)

  md5s.par.map { md5 =>
    val a = ag.get(md5)
    val m = mg.get(md5)
    val d = dg.get(md5)
    val u = ug.get(md5)
 
    def pick[T](a: Option[MetaData], b: Option[MetaData], c: Option[MetaData] = None, d: Option[MetaData] = None, f: MetaData => T) =
      a.map(f).orElse(b.map(f)).orElse(c.map(f)).orElse(d.map(f))

    // authors: AMP > Demozoo > Modland > UnExotica
    val authors = pick(a, d, m, u, _.authors).getOrElse(Buffer.empty)
    // publishers/album/year: UnExotica > Demozoo > Modland
    val publishers = pick(u, d, f = _.publishers).getOrElse(Buffer.empty)
    val album = pick(u, d, m, f = _.album).getOrElse("")
    val year = pick(u, d, f = _.year).getOrElse(0)

    MetaData(md5, authors, publishers, album, year)
  }.toBuffer.sortBy(_.md5)
}
