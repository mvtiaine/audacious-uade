// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

def parseModlandAuthorAlbum(path: String): Option[(Seq[String], String)] = {
  val UNKNOWN = "- unknown"
  val COOPS = Seq("coop-", "coop - ", "coop ")

  val tokens = path.split("/")
  assert(tokens.length >= 1)

  def authorCoop(author: String, token: String, album: String): Option[(Seq[String], String)] = {
    val coop = COOPS.find(token.startsWith).get
    val coopAuthor = token.substring(coop.length)
    Some((Seq(author, coopAuthor), album))
  }

  tokens.length match {
    case 1 | 4 =>
      val author = tokens(0)
      if (author == UNKNOWN) {
        None
      } else {
        Some((Seq(author), ""))
      }
    case 2 =>
      var author = tokens(0)
      val token1 = tokens(1)
      if (COOPS.exists(token1.startsWith)) {
        authorCoop(author, token1, "")
      } else {
        if (author == UNKNOWN) {
          author = ""
        }
        if (token1.startsWith("not by")) {
            None
        } else {
            Some((Seq(author), token1))
        }
      }
    case 3 =>
      var author = tokens(0)
      val token1 = tokens(1)
      if (COOPS.exists(token1.startsWith)) {
        authorCoop(author, token1, tokens(2))
      } else {
        if (author == UNKNOWN) {
          author = ""
        }
        if (token1 == "unnamed") {
          None
        } else {
          Some((Seq(author), s"${token1} (${tokens(2)})"))
        }
      }
    case _ =>
      assert(false)
      None
  }
}
