#ifndef CORPUS_HPP_
#define CORPUS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>

#include "Serialization.hpp"

typedef std::vector<int> AlignmentType;
typedef std::vector<size_t> WordSequenceType;
typedef std::vector<std::pair<size_t, size_t> > SentenceMappingType;
typedef boost::bimap<std::string, size_t> IndexType;

// representation of sentence -- source and target side (as word indices)
// and the current alignment
struct Sentence
{
  WordSequenceType src, tgt;
  AlignmentType align;
};

class Corpus
{
public:
  // initialize input stream
  // filename can be empty, corpus is then read from stdin
  // each line in the file must contain 2 columns separated by \t,
  // with word separated by spaces only
  Corpus(const std::string &fileName)
  {
    Read(*InitInput(fileName));
  }

  // loads word index from inStream and calls the standard ctor
  Corpus(const std::string &fileName, InStreamType &inStream)
  {
    MapReader<IndexType, std::string, size_t>().Read(inStream, srcIndex);
    MapReader<IndexType, std::string, size_t>().Read(inStream, tgtIndex);
    Read(*InitInput(fileName));
  }

  // get the corpus; not const as models write the alignment directly in the corpus
  std::vector<Sentence *> &GetSentences() { return sentences; }

  // get all <sentence, word> pairs in corpus
  const SentenceMappingType &GetTokensToSentences() { return tokensToSentences; }

  // corpus statistics
  size_t GetTotalSourceTokens() { return totalSourceTokens; }
  size_t GetTotalSourceTypes() { return srcIndex.left.size(); }
  size_t GetTotalTargetTypes() { return tgtIndex.left.size(); }

  // look up word given index
  const std::string &GetSrcWord(size_t index) { return GetWord(srcIndex, index); }
  const std::string &GetTgtWord(size_t index) { return GetWord(tgtIndex, index); }

  // look up index given word
  size_t GetSrcIndex(const std::string &word) { return GetIndex(srcIndex, word); }
  size_t GetTgtIndex(const std::string &word) { return GetIndex(tgtIndex, word); }

  // write word index into outStream
  void WriteIndex(OutStreamType &outStream)
  {
    MapWriter<IndexType::left_map>().Write(outStream, srcIndex.left);
    MapWriter<IndexType::left_map>().Write(outStream, tgtIndex.left);
  }

private:
  // read the corpus, the stream is initialized in ctor
  void Read(InStreamType &in);

  // returns the word corresponding to given index
  const std::string &GetWord(IndexType &index, size_t wordIndex);

  // returns the index of the word
  size_t GetIndex(IndexType &index, const std::string &word, bool doInsert = false);

  // the corpus: vector of sentences, each containing the words on source and target side
  // and their current alignment
  std::vector<Sentence *> sentences;

  // contains pairs of <sentence number, word-within-sentence number>
  // this is used when corpus is randomly shuffled to maintain efficient access
  SentenceMappingType tokensToSentences;

  // bidirectional mapping between words and their number indices
  IndexType srcIndex, tgtIndex;

  // size of the corpus source side
  size_t totalSourceTokens;
};

#endif // CORPUS_HPP_

