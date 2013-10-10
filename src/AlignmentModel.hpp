#ifndef ALIGNMENT_MODEL_HPP_
#define ALIGNMENT_MODEL_HPP_

#include <vector>
#include <boost/iostreams/filtering_stream.hpp> 
#include "Corpus.hpp"

class AlignmentModel
{
public:
  virtual void RunIteration(float temp) = 0;
  virtual void UpdateFromCorpus() = 0;
  virtual void BoostIdentical(size_t boost) = 0;
  virtual void Viterbi() = 0;
  virtual void ReadModel(InStreamType &inStream) = 0;
  virtual void WriteModel(OutStreamType &outStream) = 0;
};

#endif // ALIGNMENT_MODEL_HPP_
