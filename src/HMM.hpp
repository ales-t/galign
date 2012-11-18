#ifndef HMM_HPP_
#define HMM_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Corpus.hpp"
#include "Model1.hpp"

typedef boost::unordered_map<int, int> DistortionCountType;

class HMM
{
public:
  HMM(Corpus *corpus, float alpha, float cognateAlpha, const CountType &prevCounts,
      const JointCountType &prevJoint); 
  void RunIteration(bool doAggregate);
  std::vector<AlignmentType> GetAggregateAlignment();
  
private:
  boost::random::mt19937 generator;
  Corpus *corpus;
  JointCountType jointCounts, aggregateJoint; 
  CountType counts, aggregateCounts;
  DistortionCountType distortionCounts, aggregateDistortion;
  boost::unordered_set<std::string> hasCognate;
  float alpha, cognateAlpha;
};

#endif // HMM_HPP_
