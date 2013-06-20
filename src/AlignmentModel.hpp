#ifndef ALIGNMENT_MODEL_HPP_
#define ALIGNMENT_MODEL_HPP_

#include <vector>
#include <boost/iostreams/filtering_stream.hpp> 
#include "Corpus.hpp"

class AlignmentModel
{
  virtual void ReadModel(InStreamType &inStream) = 0;
  virtual void WriteModel(OutStreamType &outStream) = 0;
};

#endif // ALIGNMENT_MODEL_HPP_
