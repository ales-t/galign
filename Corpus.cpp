#include "Corpus.hpp"
#include "Utils.hpp"

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/iostreams/filter/gzip.hpp> 
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost::iostreams;
using namespace boost::algorithm;

Corpus::Corpus()
{
  filtering_istream in;
  in.push(cin);
  Read(in);
}

Corpus::Corpus(const string &fileName)
{
  filtering_istream in;
  if (ends_with(fileName, ".gz")) {
    in.push(gzip_decompressor());
  }
  in.push(file_source(fileName));
  if (! in.good())
    Die("Cannot read input file: " + fileName);

  Read(in);
}

void Corpus::Read(filtering_istream &in)
{
  string line;
  int lineNum = 0;
  int totalTokens = 0;
  while (getline(in, line)) {
    lineNum++;
    vector<string> sides;
    split(sides, line, is_any_of("\t"));
    if (sides.size() != 2)
      Die("Wrong format" + to_string(lineNum));
    Sentence *sentence = new Sentence();
    trim_if(sides[0], is_any_of(" "));
    trim_if(sides[1], is_any_of(" "));
    split(sentence->src, sides[0], is_any_of(" "));
    split(sentence->tgt, sides[1], is_any_of(" "));
    if (sides[0].empty() || sides[1].empty()) Warn("Sentence is empty: " + lineNum);
    sentences.push_back(sentence);
  }
  close(in);
}


pair<int, int> Corpus::GetSentenceAndPosition(int positionInCorpus)
{
  boost::unordered_map<int, std::pair<int, int> >::const_iterator it;
  it = tokensToSentences.find(positionInCorpus);
  if (it == tokensToSentences.end()) {
    Die("Requested word beyond corpus size: " + to_string(positionInCorpus));
  } else {
    return it->second;
  }
}
