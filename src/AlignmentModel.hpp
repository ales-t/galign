#ifndef ALIGNMENT_MODEL_HPP_
#define ALIGNMENT_MODEL_HPP_

#include <vector>
#include "Corpus.hpp"

class AlignmentModel
{
public:
  virtual std::vector<AlignmentType> GetAggregateAlignment() = 0;
};

#endif // ALIGNMENT_MODEL_HPP_
