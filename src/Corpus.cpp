#include "Corpus.hpp"
#include "Utils.hpp"

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace boost::iostreams;
using namespace boost::algorithm;

Corpus::Corpus(const string &fileName)
{
  filtering_istream *in = InitInput(fileName);
  if (! in->good())
    Die("Cannot read input file: " + fileName);
  Read(*in);
}

void Corpus::Read(filtering_istream &in)
{
  string line;
  int lineNum = 0;
  totalSourceTokens = 0;
  while (getline(in, line)) {
    lineNum++;
    vector<string> sides;
    split(sides, line, is_any_of("\t"));
    if (sides.size() != 2)
      Die("Wrong format" + lexical_cast<string>(lineNum));
    Sentence *sentence = new Sentence();
    trim_if(sides[0], is_any_of(" "));
    trim_if(sides[1], is_any_of(" "));
    split(sentence->src, sides[0], is_any_of(" "));
    split(sentence->tgt, sides[1], is_any_of(" "));
    if (sides[0].empty() || sides[1].empty())
      Warn("Sentence is empty: " + lexical_cast<string>(lineNum));

    for (size_t i = 0; i < sentence->src.size(); i++) {
      sourceTypes.insert(sentence->src[i]);
      tokensToSentences.push_back(make_pair(lineNum - 1, i)); // 0-based
      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        if (sentence->tgt[j] == sentence->src[i])
          cognates.insert(sentence->tgt[j]);
      }
    }
    sentences.push_back(sentence);
  }
  close(in);
}
