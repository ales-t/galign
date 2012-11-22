#ifndef CORPUS_HPP_
#define CORPUS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>

typedef std::vector<int> AlignmentType;
typedef std::vector<size_t> WordSequenceType;
typedef std::vector<std::pair<size_t, size_t> > SentenceMappingType;
typedef boost::bimap<std::string, size_t> IndexType;

struct Sentence
{
  WordSequenceType src, tgt;
  AlignmentType align;
};

class Corpus
{
public:
  // filename can be empty, corpus is then read from stdin
  Corpus(const std::string &fileName);

  std::vector<Sentence *> &GetSentences() { return sentences; }
  const SentenceMappingType &GetTokensToSentences() { return tokensToSentences; }
  size_t GetTotalSourceTokens() { return totalSourceTokens; }
  size_t GetTotalSourceTypes() { return srcIndex.left.size(); }
  bool HasCognate(size_t wordIdx) { return cognates.find(wordIdx) != cognates.end(); }
  const std::string &GetSrcWord(size_t index) { return GetWord(srcIndex, index); }
  const std::string &GetTgtWord(size_t index) { return GetWord(tgtIndex, index); }
  size_t GetSrcIndex(const std::string &word) { return GetIndex(srcIndex, word); }
  size_t GetTgtIndex(const std::string &word) { return GetIndex(tgtIndex, word); }

private:
  void Read(boost::iostreams::filtering_istream &in);
  const std::string &GetWord(IndexType &index, size_t wordIndex);
  size_t GetIndex(IndexType &index, const std::string &word, bool doInsert = false);

  std::vector<Sentence *> sentences;
  SentenceMappingType tokensToSentences; // for random shuffling
  boost::unordered_set<size_t> cognates;
  IndexType srcIndex, tgtIndex;
  size_t totalSourceTokens;
};

#endif // CORPUS_HPP_

