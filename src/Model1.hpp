#ifndef MODEL1_HPP_
#define MODEL1_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Corpus.hpp"

typedef boost::unordered_map<std::string, boost::unordered_map<std::string, int> > JointCountType;
typedef boost::unordered_map<std::string, int> CountType;

class Model1
{
public:
  Model1(Corpus *corpus, float alpha, float cognateAlpha) :
    corpus(corpus), alpha(alpha), cognateAlpha(cognateAlpha) {}
  void AlignRandomly();
  void RunIteration(bool doAggregate);
  std::vector<AlignmentType> GetAggregateAlignment();
  
private:
  boost::random::mt19937 generator;
  Corpus *corpus;
  JointCountType jointCounts, aggregateJoint; 
  CountType counts, aggregateCounts;
  boost::unordered_set<std::string> hasCognate;
  float alpha, cognateAlpha;
};

#endif // MODEL1_HPP_
