#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <string>
#include <vector>

#include "Model1.hpp"
#include "Corpus.hpp"
#include "Utils.hpp"

class Writer
{
public:
  Writer(Corpus *corpus) : corpus(corpus) {}
  void WriteAlignment(const std::string &fileName, bool gizaFormat = false);
  void WriteAlignment(const std::string &fileName, const std::vector<AlignmentType> &align,
      bool gizaFormat = false);

private:
  void WriteAlignmentLine(boost::iostreams::filtering_ostream &out,
      const std::vector<std::string> &src,
      const std::vector<std::string> &tgt,
      const AlignmentType &align, bool gizaFormat);

  Corpus *corpus;
};

#endif // WRITER_HPP_
