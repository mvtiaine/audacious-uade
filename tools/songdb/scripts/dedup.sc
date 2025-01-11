// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._

import md5._

val REPEAT = "\u007F"

def dedup(entries: Iterable[Buffer[String]], file: String) = {
  import Ordering.Implicits._
  // keeps original order
  val keys = entries.map(_(0)).toSeq.distinct
  val dedupped = entries.groupBy(_(0)).par.map(e =>
    if (e._2.size > 1) {
      System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${_md5check(e._1)} entries: ${e._2}")
    }
    (e._1, e._2.toSeq.sorted.head)
  ).seq
  keys.map(dedupped).toSeq
}

def dedupidx(entries: Iterable[Buffer[String]], file: String, strict: Boolean = false) = {
  // keeps original order
  val keys = entries.par.map(_(0)).seq.toSeq.distinct
  val dedupped = entries.groupBy(_(0)).par.map(e =>
    if (e._2.size > 1) {
      if (strict) {
        assert(e._2.forall(_ == e._2.head))
      } else {
        System.err.println(s"WARN: removing duplicate entries in ${file}, md5: ${e._1} entries: ${e._2}")
      }
    }
    (e._1, e._2.toSeq.sortBy(entries =>
      entries.tail.mkString("###")
    ).head)
  ).seq
  val res = Buffer.empty[Buffer[String]]
  var prev = Buffer.empty[String]
  for (k <- keys) {
    val s = dedupped(k)
    val idx = _md5idx(s.head)
    assert(base64d24(idx) > 0)
    if (s.tail.sameElements(prev)) {
      res += Buffer(idx)
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
          tmp += REPEAT
        } else {
          tmp += s.tail(i)
        }
      }
      res += Buffer(idx) ++ tmp
    } else {
      res += Buffer(idx) ++ s.tail
    }
    prev = s.tail
  }
  res.toSeq
}

def validate(entries: Iterable[Buffer[String]], file: String) = {
  val check = entries.toSeq.par.map(_(0))
  if (check.size != check.distinct.size) {
    val dups = check.diff(check.distinct).distinct
    throw new IllegalStateException(s"Duplicate entries in ${file}: ${dups}")
  }
}
