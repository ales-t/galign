#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <string>
#include <vector>

#include "Model1.hpp"
#include "Corpus.hpp"
#include "Utils.hpp"

// output final word alignment
class Writer
{
public:
  Writer(Corpus *corpus) : corpus(corpus) {}

  // this variant outputs the alignment currently in the corpus object
  void WriteAlignment(const std::string &fileName, bool mosesFormat = false);

private:
  // print one line of word alignment
  // without the mosesFormat flag, the format is srcWord{alignedWord} srcWord{alignedWord} ...
  // with mosesFormat, prints out 0-0 1-0 etc. (zero-based, the virtual NULL token is not included)
  void WriteAlignmentLine(OutStreamType &out,
      const WordSequenceType &src,
      const WordSequenceType &tgt,
      const AlignmentType &align, bool mosesFormat);

  Corpus *corpus;
};

#endif // WRITER_HPP_
