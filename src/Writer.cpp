#include "Writer.hpp"

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/foreach.hpp>

using namespace std;
using namespace boost::iostreams;

void Writer::WriteAlignment(const std::string &fileName)
{
  filtering_ostream *out = InitOutput(fileName);
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    WriteAlignmentLine(*out, sentence->src, sentence->tgt, sentence->align);  
  }
  close(*out);
}

void Writer::WriteAlignment(const std::string &fileName, const vector<AlignmentType> &align)
{
  filtering_ostream *out = InitOutput(fileName);
  vector<AlignmentType>::const_iterator alignIt = align.begin();
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    WriteAlignmentLine(*out, sentence->src, sentence->tgt, *(alignIt++));
  }
  close(*out);
}

void Writer::WriteAlignmentLine(filtering_ostream &out, const std::vector<std::string> &src,
  const std::vector<std::string> &tgt, const AlignmentType &align)
{
  for (int i = 0; i < src.size(); i++) {
    out << src[i] << "{" << tgt[align[i]] << "}";
    if (i != src.size() - 1)
      out << " ";
  }
  out << "\n";
}
