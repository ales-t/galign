#ifndef HMM_HPP_
#define HMM_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <tbb/atomic.h>

#include "Corpus.hpp"
#include "Model1.hpp"

#define BUCKET_LIMIT 6

typedef std::vector<tbb::atomic<int> > DistortionCountType;

class HMM
{
public:
  HMM(Corpus *corpus, float alpha, float cognateAlpha, float distAlpha, const CountType &prevCounts,
      const JointCountType &prevJoint); 
  void RunIteration(bool doAggregate);
  std::vector<AlignmentType> GetAggregateAlignment();

private:
  std::vector<float> GetDistribution(Sentence *sentence, size_t srcPosition, CountType &a_counts,
    JointCountType &a_jointCounts, DistortionCountType &a_distCounts);
  int GetBucket(int distortion)
  {
    if (distortion > BUCKET_LIMIT) distortion = BUCKET_LIMIT;
    if (distortion < -BUCKET_LIMIT) distortion = -BUCKET_LIMIT;
    return BUCKET_LIMIT + distortion;
  }

  int GetInputDistortion(Sentence *sentence, size_t srcPosition, size_t tgtPosition) 
  {
    int inDistortion = 1;
    if (srcPosition > 0) {
      inDistortion = (int)tgtPosition - (int)sentence->align[srcPosition - 1];
    }
    return inDistortion;
  }

  int GetOutputDistortion(Sentence *sentence, size_t srcPosition, size_t tgtPosition) 
  {
    int outDistortion = 1;
    if (srcPosition < sentence->src.size() - 1) {
      outDistortion = (int)sentence->align[srcPosition + 1] - (int)tgtPosition;
    }
    return outDistortion;
  }

  SentenceMappingType order;
  boost::random::mt19937 generator;
  Corpus *corpus;
  float alpha, cognateAlpha, distAlpha;
  CountType counts, aggregateCounts;
  JointCountType jointCounts, aggregateJoint; 
  DistortionCountType distortionCounts, aggregateDistortion;
};

#endif // HMM_HPP_
