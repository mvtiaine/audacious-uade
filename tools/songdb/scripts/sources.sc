//> using dep org.scala-lang.modules::scala-parallel-collections::1.0.4

import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._
import scala.util.Using

enum Source:
  case Modland, AMP, UnExotica, Mods_Anthology, Wanted_Team, Zakalwe, Aminet, Modland_Incoming, Demozoo_Leftovers
import Source._

val tsvfiles = Buffer(
  ("modland.tsv", Modland),
  ("amp.tsv", AMP),
  ("unexotica.tsv", UnExotica),
  ("modsanthology.tsv", Mods_Anthology),
  ("wantedteam.tsv", Wanted_Team),
  ("zakalwe.tsv", Zakalwe),
  ("aminet.tsv", Aminet),
  ("modland_incoming.tsv", Modland_Incoming),
  ("demozoo_leftovers.tsv", Demozoo_Leftovers),
);

case class TsvEntry (
  md5: String,
  subsong: Int,
  songlength: Int,
  songend: String,
  player: String,
  format: String,
  channels: Int,
  filesize: Int,
  path: String,
)

lazy val tsvs = tsvfiles.par.map(tsv => (tsv._2, Using(scala.io.Source.fromFile(s"sources/${tsv._1}"))(_.getLines.map(line =>
  val l = line.split("\t")
  if (l.length > 8) TsvEntry(l(0), l(1).toInt, l(2).toInt, l(3), l(4), l(5), if (l(6).isEmpty) 0 else l(6).toInt, l(7).toInt, l(8))
  else TsvEntry(l(0), l(1).toInt, l(2).toInt, l(3), "", "", 0, -1, "")
).toBuffer).get.groupBy(_.md5))).seq

case class SourceDBEntry (
  md5: String,
  path: String,
  filesize: Int,
)

def readSourceDB(source: Source) = {
  tsvs.filter(_._1 == source).par.flatMap(_._2).map({case (md5,subsongs) =>
    if (subsongs.groupBy(_.subsong).exists(_._2.size > 1)) {
      System.err.println("WARN: duplicate file in " + source + " for " + md5 + ": " + subsongs)
    }
    val e = subsongs.filter(_.path != "").minBy(_.path.length)
    SourceDBEntry(md5, e.path, e.filesize)
  }).seq
}

lazy val modland = readSourceDB(Modland)
lazy val unexotica = readSourceDB(UnExotica)
lazy val amp = readSourceDB(AMP)
lazy val aminet = readSourceDB(Aminet)
lazy val demozoo_leftovers = readSourceDB(Demozoo_Leftovers)
lazy val wantedteam = readSourceDB(Wanted_Team)
