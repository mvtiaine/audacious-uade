// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

//> using dep org.scala-lang.modules::scala-parallel-collections::1.2.0
//> using dep io.circe::circe-generic::0.14.6
//> using dep io.circe::circe-yaml::1.15.0

import java.nio.file.Files
import java.nio.file.Paths
import scala.collection.parallel.CollectionConverters._
import scala.util.Using

import cats.syntax.either._
import io.circe._
import io.circe.generic.auto._
import io.circe.yaml
import io.circe.yaml.parser

val unexotica_path = System.getProperty("user.home") + "/UnExotica/"

implicit def h[A,B](implicit a: Decoder[A], b: Decoder[B]): Decoder[Either[A,B]] = {
  val l: Decoder[Either[A,B]]= a.map(Left.apply)
  val r: Decoder[Either[A,B]]= b.map(Right.apply)
  l or r
}

type StringOrList = Either[String,List[String]]
type IntOrList = Either[Int,List[Int]]
case class UnExoticaMeta (
  `type`: String,
  title: Either[String,Int],
  `alternative titles`: Option[StringOrList],
  composer: StringOrList,
  format: StringOrList,
  year: Either[Int,String],
  team: Option[StringOrList],
  publisher: Option[StringOrList],
  group: Option[StringOrList],
  party: Option[String],
  `box scan`: Option[String],
  `hol id`: Option[IntOrList],
  `lemon id`: Option[IntOrList],
  `rip type`: String,
  `ripped by`: StringOrList,
  comments: Option[String],
)

lazy val metas = sources.unexotica.par.flatMap(e =>
  val tokens = e.path.split("/")
  val parent1 = e.path.split("/").dropRight(1)
  val parent2 = e.path.split("/").dropRight(2)
  val file1 = unexotica_path + parent1.mkString("/") + ".txt"
  val file2 = unexotica_path + parent2.mkString("/") + ".txt"

  def parse(file: String) = {
    val yaml = parser.parse(Using(scala.io.Source.fromFile(file)(scala.io.Codec.ISO8859))(_.mkString).get)
    val meta = yaml
      .leftMap(err => err: Error)
      .flatMap(_.as[UnExoticaMeta])
      .valueOr(throw _)
    Some(e.md5, e.path, e.filesize, meta)
  }
  if (Files.exists(Paths.get(file1))) parse(file1)
  else if (Files.exists(Paths.get(file2))) parse(file2)
  else None
).groupBy(_._1).map({case (md5, metas) =>
  // pick oldest for duplicates
  if (metas.size > 1) {
    System.err.println(s"WARN: removing duplicate UnExotica entries for md5: ${md5} entries: ${metas}")
  }
  val year = metas.map(m => if (m._4.year.fold(_.toString, _.toString) == "Unknown") 9999 else m._4.year.left.get).min
  metas.filter(m => {
    val cmp = if (m._4.year.fold(_.toString, _.toString) == "Unknown") 9999 else m._4.year.left.get
    year == cmp
  }).seq.sortBy(_._2).head // secondarily sort by path for consistency
}).seq.toSeq

def transformAuthor(author: String): String = {
  val pseudonyms = Set(
    "Creative_Thought",
    "DJ_Braincrack",
    "Digital_Masters",
    "MC_Slack",
    "Mad_Jack",
    "Mad_Phantom",
    "McMullan_and_Low",
    "Pipe_Smokers_Cough",
    "Private_Affair",
    "Radio_Poland",
    "Sonic_Boom_Boy",
    "Sonicom_Music",
    "Ten_Pin_Alley",
    "Urban_Shakedown"
  )

  if (author == "Unknown") return ""

  val authorTokens = author.split("_").toList
  val first = authorTokens.head
  val pseudonym = pseudonyms.contains(author)

  if (!pseudonym && (first == "Da" || first == "de" || first == "van" || first == "Pieket")) {
    val last = authorTokens.last
    (last :: authorTokens.init).mkString(" ")
  } else if (!pseudonym && first != "The") {
    (authorTokens.tail :+ first).mkString(" ")
  } else {
    authorTokens.mkString(" ")
  }
}

def transformAlbum(meta: UnExoticaMeta, authorAlbum: Array[String]): String = {
  val title = meta.title.fold(
    title => title,
    title => title.toString
  // remove subtitle parts to try avoid overly long titles
  ).split(" - ").head.trim
  if (title.isEmpty) if (authorAlbum.size > 1) authorAlbum(1) else ""
  else title
}

def transformPublisher(meta: UnExoticaMeta): String = {
  val publisher = meta.group.getOrElse(meta.publisher.getOrElse(meta.team.getOrElse(Right(List(""))))) match {
    case Left(publisher) => publisher
    case Right(publishers) => publishers.head
  }
  publisher.trim
}
