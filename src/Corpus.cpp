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
  // empty name is interpreted as stdin by InitInput
  InStreamType *in = InitInput(fileName);

  // read the file
  Read(*in);
}

void Corpus::Read(InStreamType &in)
{
  string line;
  int lineNum = 0;
  totalSourceTokens = 0;
  size_t nullIndex = GetIndex(tgtIndex, "<NULL>", true);
  while (getline(in, line)) {
    lineNum++;
    vector<string> sides;
    split(sides, line, is_any_of("\t"));
    if (sides.size() != 2)
      Die("Wrong format" + lexical_cast<string>(lineNum));
    Sentence *sentence = new Sentence();
    trim_if(sides[0], is_any_of(" "));
    trim_if(sides[1], is_any_of(" "));
    vector<string> src, tgt;
    split(src, sides[0], is_any_of(" "));
    split(tgt, sides[1], is_any_of(" "));
    sentence->src.resize(src.size());
    sentence->tgt.resize(tgt.size() + 1);
    sentence->tgt[0] = nullIndex;
    if (src.empty() || tgt.empty())
      Warn("Sentence is empty: " + lexical_cast<string>(lineNum));

    // populate the source side, add new words to index
    totalSourceTokens += src.size();
    for (size_t i = 0; i < src.size(); i++) {
      size_t srcWordIndex = GetIndex(srcIndex, src[i], true);
      sentence->src[i] = srcWordIndex;
      tokensToSentences.push_back(make_pair(lineNum - 1, i)); // 0-based
    }
    // populate the target side, add words to index
    for (size_t i = 0; i < tgt.size(); i++) {
      size_t tgtWordIndex = GetIndex(tgtIndex, tgt[i], true);
      sentence->tgt[i + 1] = tgtWordIndex; // there is NULL token on position 0
    }
    sentences.push_back(sentence);
  }
  close(in);
}


const std::string &Corpus::GetWord(IndexType &index, size_t wordIndex)
{
  IndexType::right_map::const_iterator it = index.right.find(wordIndex);
  if (it == index.right.end()) {
    Die("Unknown index: " + lexical_cast<string>(wordIndex));
  }
  return it->second;
}

size_t Corpus::GetIndex(IndexType &index, const std::string &word, bool doInsert)
{
  if (doInsert) {
    index.insert(IndexType::value_type(word, index.size()));
  } 
  IndexType::left_map::const_iterator it = index.left.find(word);
  if (it == index.left.end()) {
    Die("Unknown word: " + word);
  }
  return it->second;
}
