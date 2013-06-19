#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/lexical_cast.hpp>
#include <tbb/concurrent_hash_map.h>

inline void Warn(const std::string &msg)
{
  std::cerr << "WARNING: " << msg << std::endl;
}

inline void Log(const std::string &msg)
{
  std::cerr << msg << std::endl;
}

inline void Die(const std::string &msg)
{
  std::cerr << "ERROR: " << msg << std::endl;
  exit(1);
}

// a thin wrapper around TBB concurrent hash
// implements [] operator (behaves the same way as in std::map),
// simple querying of existence and deletion
template <typename KeyT, typename ValueT>
class SafeHash
{
  typedef tbb::concurrent_hash_map<KeyT, ValueT> InternalHashType;

public:
  typedef ValueT value_type;

  const ValueT &operator[](const KeyT &key) const
  {
    typename InternalHashType::const_accessor a;
    internalHash.find(a, key);
    return a->second;
  }

  ValueT &operator[](const KeyT &key)
  {
    typename InternalHashType::accessor a;
    if (! internalHash.find(a, key)) {
      internalHash.insert(a, key);
    }
    return a->second;
  }

  bool Contains(const KeyT &key) const
  {
    typename InternalHashType::const_accessor a;
    return internalHash.find(a, key);
  }

  void Erase(const KeyT &key)
  {
    typename InternalHashType::accessor a;
    internalHash.find(a, key);
    internalHash.erase(a);    
  }

private:
  InternalHashType internalHash;
};

// crude, simple simulated annealing
// decreases temperature once cooling starts
class Annealer
{
public:
  Annealer(size_t total, size_t coolingFrom) : coolingFrom(coolingFrom)
  {
    step = 1.0 / (1 + total - coolingFrom);
  }

  double GetTemp(size_t iter)
  {
    if (iter <= coolingFrom) {
      return 1;
    } else {
      return std::max(step, 1 - (iter - coolingFrom)*step);
    }
  }

private:
  size_t coolingFrom;
  double step;
};


// a categorical/discrete distribution in log-space
class LogDistribution
{
public:
  LogDistribution() 
  {
    maxProb = -std::numeric_limits<float>::infinity();
  }

  // add a new value
  void Add(float logProb)
  {
    logProbs.push_back(logProb);
    maxProb = std::max(maxProb, logProb);
  }

  // return all probabilities, exponentiated
  std::vector<float> Exp() const
  {
    std::vector<float> out;
    out.reserve(logProbs.size());
    BOOST_FOREACH(float logProb, logProbs) {
      out.push_back(exp(logProb));
    }
    return out;
  }

  size_t GetSize() const { return logProbs.size(); }

  // get/set the probability of pos-th element
  float &operator[] (int pos) { return logProbs[pos]; }

  // normalize the distribution (in-place)
  void Normalize()
  {
    float normConst = 0;
    BOOST_FOREACH(float logProb, logProbs) {
      normConst += exp(logProb - maxProb);
    }
    normConst = maxProb + log(normConst);
    for (size_t i = 0; i < logProbs.size(); i++) {
      logProbs[i] -= normConst;
    }
  }

private:
  std::vector<float> logProbs; // log probabilities
  float maxProb; // highest probability in the distribution, used in normalization
};

// initialize input stream, supports plain and gzipped files
// (distinguished by file extension .gz)
// empty fileName is interpreted as STDIN
boost::iostreams::filtering_istream *InitInput(const std::string &fileName = "");

// initialize output stream, supports plain and gzipped files
// (distinguished by file extension .gz)
// empty fileName is interpreted as STDOUT
boost::iostreams::filtering_ostream *InitOutput(const std::string &fileName = "");

#endif // UTILS_HPP_
