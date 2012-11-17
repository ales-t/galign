#ifndef MODEL1_HPP_
#define MODEL1_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Corpus.hpp"

class Model1
{
public:
  Model1(Corpus *corpus, float alpha, float cognateAlpha) :
    corpus(corpus), alpha(alpha), cognateAlpha(cognateAlpha) {}
  void AlignRandomly();
  void RunIteration(bool doAggregate);

private:
  boost::random::mt19937 generator;
  Corpus *corpus;
  boost::unordered_map<std::string, boost::unordered_map<std::string, int> > jointCounts, aggregateJoint; 
  boost::unordered_map<std::string, int> counts, aggregateCounts;
  boost::unordered_set<std::string> hasCognate;
  float alpha, cognateAlpha;
};

#endif // MODEL1_HPP_
