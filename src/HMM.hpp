#ifndef HMM_HPP_
#define HMM_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <tbb/atomic.h>

#include "Corpus.hpp"
#include "Model1.hpp"
#include "AlignmentModel.hpp"

#define BUCKET_LIMIT 6

typedef std::vector<tbb::atomic<int> > DistortionCountType;

class HMM : public AlignmentModel
{
public:
  HMM(Corpus *corpus, float alpha, float distAlpha, const CountType &prevCounts,
      const JointCountType &prevJoint); 
  void RunIteration(double temp);

private:
  std::vector<float> GetDistribution(Sentence *sentence, size_t srcPosition, CountType &a_counts,
    JointCountType &a_jointCounts, DistortionCountType &a_distCounts);
  void UpdateTransition(DistortionCountType &distCounts, const Sentence *sentence, size_t srcPos,
      int diff);
  int GetBucket(int distortion)
  {
    if (distortion > BUCKET_LIMIT) distortion = BUCKET_LIMIT;
    if (distortion < -BUCKET_LIMIT) distortion = -BUCKET_LIMIT;
    return BUCKET_LIMIT + distortion;
  }

  SentenceMappingType order;
  boost::random::mt19937 generator;
  Corpus *corpus;
  float alpha, distAlpha;
  CountType counts;
  JointCountType jointCounts; 
  DistortionCountType distortionCounts;
};

#endif // HMM_HPP_
