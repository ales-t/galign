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

typedef std::vector<tbb::atomic<int> > CountType;
typedef std::vector<SafeHash<size_t, tbb::atomic<int> > > JointCountType;

// estimate IBM Model 1 using Gibbs sampling
class Model1 : public AlignmentModel
{
public:
  Model1(Corpus *corpus, float alpha, float cognateAlpha) :
    corpus(corpus), alpha(alpha), cognateAlpha(cognateAlpha)
  {
    order = corpus->GetTokensToSentences();
    counts.resize(corpus->GetTotalTargetTypes());
    aggregateCounts.resize(corpus->GetTotalTargetTypes());
    jointCounts.resize(corpus->GetTotalSourceTypes());
    aggregateJoint.resize(corpus->GetTotalSourceTypes());
  }

  // align each source word in corpus to a random target word,
  // initialize counts
  void AlignRandomly();

  // run one iteration of Gibbs sampling over the corpus
  // if doAggregate, the samples are cummulated in aggregate counts
  // (this is used for more robust final word alignment)
  void RunIteration(bool doAggregate);

  // get word alignment aggregated over 
  virtual std::vector<AlignmentType> GetAggregateAlignment();
  
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
  JointCountType jointCounts, aggregateJoint; 
  CountType counts, aggregateCounts;

  // priors on alignment, prior for cognate words can be boosted
  float alpha, cognateAlpha;
};

#endif // MODEL1_HPP_
