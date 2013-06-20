#ifndef MODEL1_HPP_
#define MODEL1_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <tbb/atomic.h>

#include "Corpus.hpp"
#include "Utils.hpp"
#include "AlignmentModel.hpp"
#include "Serialization.hpp"

typedef std::vector<tbb::atomic<int> > CountType;
typedef std::vector<SafeHash<size_t, tbb::atomic<int> > > JointCountType;

// estimate IBM Model 1 using Gibbs sampling
class Model1 : public AlignmentModel
{
public:
  Model1(Corpus *corpus, float alpha) :
    corpus(corpus), alpha(alpha)
  {
    order = corpus->GetTokensToSentences();
    counts.resize(corpus->GetTotalTargetTypes());
    jointCounts.resize(corpus->GetTotalSourceTypes());
  }

  // initialize with existing model
  void ReadModel(InStreamType &in)
  {
    // TODO some header
    counts = IterableReader>CountType>().Read(in);
    jointCounts.Expose() = IterableReader<JointCountType>().Read(in);
  }

  void WriteModel(OutStreamType &out)
  {
    IterableWriter<CountType>().Write(out, counts);
    IterableWriter<JointCountType>().Write(out, jointCounts.Expose());
  }

  // align each source word in corpus to a random target word,
  // initialize counts
  void AlignRandomly();

  // run one iteration of Gibbs sampling over the corpus
  void RunIteration(float temp);

  // get counts and joint counts, used as initial parameters by
  // subsequent models
  const CountType &GetCounts()           { return counts; }
  const JointCountType &GetJointCounts() { return jointCounts; }
private:

  // random generator
  boost::random::mt19937 generator;

  // order of words (pairs of <sentence, word position> in corpus)
  SentenceMappingType order;
  Corpus *corpus;
  JointCountType jointCounts; 
  CountType counts;

  // prior on alignment
  float alpha;
};

#endif // MODEL1_HPP_
