#ifndef CORPUS_HPP_
#define CORPUS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/iostreams/filtering_stream.hpp> 

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
  pair<int, int> GetSentenceAndPosition(int positionInCorpus);

private:
  void Read(boost::iostreams::filtering_istream &in);

  std::vector<Sentence *> sentences;
  boost::unordered_map<int, std::pair<int, int> > tokensToSentences;
  boost::unordered_set<std::string> srcTypes;
};

#endif // CORPUS_HPP_

