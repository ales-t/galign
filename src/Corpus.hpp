#ifndef CORPUS_HPP_
#define CORPUS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

typedef std::vector<int> AlignmentType;

struct Sentence
{
  std::vector<std::string> src;
  std::vector<std::string> tgt;
  AlignmentType align;
};

class Corpus
{
public:
  Corpus(); // default ctor reads from stdin
  Corpus(const std::string &fileName);

  std::vector<Sentence *> &GetSentences() { return sentences; }
  std::pair<int, int> GetSentenceAndPosition(int positionInCorpus);
  const boost::unordered_set<std::string> &GetSrcTypes() { return sourceTypes; }
  int GetTotalSourceTokens() { return totalSourceTokens; }

private:
  void Read(boost::iostreams::filtering_istream &in);

  std::vector<Sentence *> sentences;
  boost::unordered_map<int, std::pair<int, int> > tokensToSentences;
  boost::unordered_set<std::string> sourceTypes;
  int totalSourceTokens;
};

#endif // CORPUS_HPP_

