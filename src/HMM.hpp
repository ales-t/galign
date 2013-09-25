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
  HMM(Corpus *corpus, float alpha, float nullProb);
  
  virtual void ReadModel(InStreamType &in)
  {
    // TODO some header
    counts = IterableReader<CountType>().Read(in);
    size_t size = ValueReader<size_t>().Read(in);
    for (size_t i = 0; i < size; i++) {
      jointCounts[i].Clear();
      MapReader<CountHashType::InternalHashType,
        size_t, tbb::atomic<int> >().Read(in, jointCounts[i].Expose());
    }
    distortionCounts = IterableReader<DistortionCountType>().Read(in);
  }

  virtual void WriteModel(OutStreamType &out)
  {  
    IterableWriter<CountType>().Write(out, counts);
    ValueWriter<size_t>().Write(out, jointCounts.size());
    BOOST_FOREACH(JointCountType::value_type hash, jointCounts) {
      MapWriter<CountHashType::InternalHashType>().Write(out, hash.Expose());
    }
    IterableWriter<DistortionCountType>().Write(out, distortionCounts);
  }

  virtual void RunIteration(float temp);
  virtual void UpdateFromCorpus();
  virtual void Viterbi();

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
  float alpha, nullProb;
  CountType counts;
  JointCountType jointCounts; 
  DistortionCountType distortionCounts;
};

#endif // HMM_HPP_
