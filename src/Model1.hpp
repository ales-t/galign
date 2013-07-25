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
typedef SafeHash<size_t, tbb::atomic<int> > CountHashType;
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
  }

  virtual void WriteModel(OutStreamType &out)
  {  
    IterableWriter<CountType>().Write(out, counts);
    ValueWriter<size_t>().Write(out, jointCounts.size());
    BOOST_FOREACH(JointCountType::value_type hash, jointCounts) {
      MapWriter<CountHashType::InternalHashType>().Write(out, hash.Expose());
    }
  }

  // align each source word in corpus to a random target word,
  // initialize counts
  void AlignRandomly();

  // run one iteration of Gibbs sampling over the corpus
  virtual void RunIteration(float temp);

  // add counts from aligned corpus
  virtual void UpdateFromCorpus();

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
