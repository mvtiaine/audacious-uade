import scala.collection.mutable.Buffer
import scala.collection.parallel.CollectionConverters._

case class Subsong (
  subsong: Int,
  songlength: Int,
  songend: String,
)

case class SonglengthEntry (
  md5: String,
  minsubsong: Int,
  maxsubsong: Int,
  subsongs: Seq[Subsong],
)

lazy val db = sources.tsvs.par.flatMap(_._2).map({case (md5,subsongs) => {
  val minsubsong = subsongs.minBy(_.subsong).subsong
  val maxsubsong = subsongs.maxBy(_.subsong).subsong
  val songs = subsongs.map(s => Subsong(s.subsong, s.songlength, s.songend)).distinct.toSeq
  if (songs.length > maxsubsong - minsubsong + 1) {
      System.err.println("WARN: inconsistent songlengths for " + md5 + ": " + songs)
      val subsongs = Buffer.empty[Subsong]
      for (subsong <- minsubsong to maxsubsong) {
        subsongs.append(songs.filter(_.subsong == subsong).maxBy(_.songlength))
      }
      SonglengthEntry(md5, minsubsong, maxsubsong, subsongs.toSeq)
  } else {
    SonglengthEntry(md5, minsubsong, maxsubsong, songs)
  }
}}).distinct.groupBy(_.md5).map({case (md5, entries) =>
  var best = entries.head
  if (entries.length > 1) {
    val totallens = entries.map(_.subsongs.map(_.songlength).sum)
    val minmax = entries.groupBy(e => (e.minsubsong, e.maxsubsong))
    if (totallens.exists(_ != totallens.head) || minmax.size > 1) {
      System.err.println("WARN: inconsistent songlengths: " + entries)
    }
    val maxsubsongs = entries.maxBy(_.subsongs.size).subsongs.size
    best = entries.filter(_.subsongs.size == maxsubsongs).maxBy(e => e.subsongs.map(_.songlength).sum / e.subsongs.length)
  }
  best
}).toSeq.seq
