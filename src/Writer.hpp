#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <string>
#include <vector>

#include "Model1.hpp"
#include "Corpus.hpp"
#include "Utils.hpp"

// output final word alignment
// TODO output a model file
class Writer
{
public:
  Writer(Corpus *corpus) : corpus(corpus) {}

  // this variant outputs the alignment currently in the corpus object
  // (i.e. the last Gibbs sample)
  void WriteAlignment(const std::string &fileName, bool gizaFormat = false);

  // outputs alignment given in align
  // used for writing aggregate alignments
  void WriteAlignment(const std::string &fileName, const std::vector<AlignmentType> &align,
      bool gizaFormat = false);

private:
  // print one line of word alignment
  // without the gizaFormat flag, the format is srcWord{alignedWord} srcWord{alignedWord} ...
  // with gizaFormat, prints out 0-0 1-0 etc. (zero-based, the virtual NULL token is not included)
  void WriteAlignmentLine(boost::iostreams::filtering_ostream &out,
      const WordSequenceType &src,
      const WordSequenceType &tgt,
      const AlignmentType &align, bool gizaFormat);

  Corpus *corpus;
};

#endif // WRITER_HPP_
