// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

import scala.collection.mutable.Buffer

val _md5check = scala.collection.mutable.Map[String,String]()
val _md5idx = scala.collection.mutable.Map[String,String]()
val _idxmd5 = scala.collection.mutable.Map[String,String]()

def md5(md5: String) = synchronized {
  val base64 = base64e(md5)
  if (_md5check.contains(base64) && _md5check(base64) != md5) {
    System.err.println(s"ERROR: MD5 check failed, short ${base64} existing ${_md5check(base64)} new ${md5}")
    throw new IllegalStateException
  }
  _md5check(base64) = md5
  val md5v = java.lang.Long.parseLong(md5.take(12), 16)
  if (base64d(base64) != md5v) {
    System.err.println(s"ERROR: MD5 vs base64 check failed for MD5:${md5.take(12)}/${md5v} base64: ${base64}/${base64d(base64)}")
    throw new IllegalStateException
  }
  base64
}

def base64e(v: Long, minimize: Boolean = false): String =  {
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

def base64e24(v: Int, minimize: Boolean = true): String =  {
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

def base64e(md5: String): String =  {
  base64e(java.lang.Long.parseLong(md5.take(12), 16))
}

def base64d(base64: String) = {
  var v = 0L
  if (base64.length == 8) {
    v |= (base64(0) - 45L) << 42
    v |= (base64(1) - 45L) << 36
    v |= (base64(2) - 45L) << 30
    v |= (base64(3) - 45L) << 24
    v |= (base64(4) - 45L) << 18
    v |= (base64(5) - 45L) << 12
    v |= (base64(6) - 45L) << 6 
    v |= base64(7) - 45L
  } else if (base64.length == 7) {
    v |= (base64(0) - 45L) << 36
    v |= (base64(1) - 45L) << 30
    v |= (base64(2) - 45L) << 24
    v |= (base64(3) - 45L) << 18
    v |= (base64(4) - 45L) << 12
    v |= (base64(5) - 45L) << 6 
    v |= base64(6) - 45L
  } else if (base64.length == 6) {
    v |= (base64(0) - 45L) << 30
    v |= (base64(1) - 45L) << 24
    v |= (base64(2) - 45L) << 18
    v |= (base64(3) - 45L) << 12
    v |= (base64(4) - 45L) << 6 
    v |= base64(5) - 45L
  } else if (base64.length == 5) {
    v |= (base64(0) - 45L) << 24
    v |= (base64(1) - 45L) << 18
    v |= (base64(2) - 45L) << 12
    v |= (base64(3) - 45L) << 6 
    v |= base64(4) - 45L
  } else {
    v = base64d24(base64)
  }
  v
}

def base64d24(base64: String) = {
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

def md5idxdiff(entries: Iterable[Buffer[String]]) = {
  var prevmd5 = 0
  var prevtail = Buffer.empty[String]
  entries.map(e => {
    val tail = e.tail
    val md5v = base64d24(e(0))
    val res = if (tail == prevtail || tail.isEmpty) {
      assert(md5v > prevmd5)
      val diff = md5v - prevmd5
      Buffer(base64e24(diff)) ++ e.tail
    } else {
      e
    }
    prevmd5 = md5v
    prevtail = tail
    res
  })
}
