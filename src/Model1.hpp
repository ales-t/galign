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
  Model1(Corpus *corpus, float alpha) :
    corpus(corpus), alpha(alpha)
  {
    order = corpus->GetTokensToSentences();
    counts.resize(corpus->GetTotalTargetTypes());
    jointCounts.resize(corpus->GetTotalSourceTypes());
  }

  // align each source word in corpus to a random target word,
  // initialize counts
  void AlignRandomly();

  // run one iteration of Gibbs sampling over the corpus
  void RunIteration(double temp);

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
