#include "Writer.hpp"

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/foreach.hpp>

using namespace std;
using namespace boost::iostreams;

void Writer::WriteAlignment(const std::string &fileName, bool mosesFormat)
{
  filtering_ostream *out = InitOutput(fileName);
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    WriteAlignmentLine(*out, sentence->src, sentence->tgt, sentence->align, mosesFormat);  
  }
  close(*out);
}

void Writer::WriteAlignmentLine(filtering_ostream &out, const WordSequenceType &src,
  const WordSequenceType &tgt, const AlignmentType &align, bool mosesFormat)
{
  for (size_t i = 0; i < src.size(); i++) {
    if (align[i] != 0) {
      if (mosesFormat) {
        out << i << "-" << align[i];
      } else {
        out << corpus->GetSrcWord(src[i]) << "{" << corpus->GetTgtWord(tgt[align[i]]) << "}";
      }
      if (i != src.size() - 1)
        out << " ";
    }
  }
  out << "\n";
}
