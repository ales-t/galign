#ifndef MODEL1_HPP_
#define MODEL1_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <tbb/atomic.h>

#include "Corpus.hpp"
#include "Utils.hpp"

typedef std::vector<tbb::atomic<int> > CountType;
typedef std::vector<SafeHash<size_t, tbb::atomic<int> > > JointCountType;

class Model1
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
  void AlignRandomly();
  void RunIteration(bool doAggregate);
  std::vector<AlignmentType> GetAggregateAlignment();
  
  const CountType &GetCounts()           { return counts; }
  const JointCountType &GetJointCounts() { return jointCounts; }
private:
  boost::random::mt19937 generator;
  SentenceMappingType order;
  Corpus *corpus;
  JointCountType jointCounts, aggregateJoint; 
  CountType counts, aggregateCounts;
  float alpha, cognateAlpha;
};

#endif // MODEL1_HPP_
