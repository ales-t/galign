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
#include "Serialization.hpp"

#define BUCKET_LIMIT 6

typedef std::vector<tbb::atomic<int> > DistortionCountType;

class HMM : public AlignmentModel
{
public:
  HMM(Corpus *corpus, float alpha, float distAlpha, const CountType &prevCounts,
      const JointCountType &prevJoint); 
  
  // initialize with existing model
  void ReadModel(InStreamType &in)
  {
    // TODO some header
    counts = IterableReader>CountType>(in);
    jointCounts.Expose() = IterableReader<JointCountType>(in);
    distortionCounts = IterableReader<DistortionCountType>(in);
  }

  void WriteModel(OutStreamType &out)
  {
    IterableWriter<CountType>(out, counts);
    IterableWriter<JointCountType>(out, jointCounts.Expose());
    IterableWriter<DistortionCountType>(out, distortionCounts);
  }

  void RunIteration(float temp);

private:
  std::vector<float> GetDistribution(Sentence *sentence, size_t srcPosition);
  std::vector<int> GetTransitions(const Sentence *sentence, size_t srcPos);
  void UpdateTransition(const Sentence *sentence, size_t srcPos, int diff);
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
