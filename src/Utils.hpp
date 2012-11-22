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

template <typename KeyT, typename ValueT>
class SafeHash
{
  typedef tbb::concurrent_hash_map<KeyT, ValueT> InternalHashType;

public:
  typedef ValueT value_type;

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

class LogDistribution
{
public:
  LogDistribution() 
  {
    maxProb = -std::numeric_limits<float>::infinity();
  }

  void Add(float logProb)
  {
    logProbs.push_back(logProb);
    maxProb = std::max(maxProb, logProb);
  }

  std::vector<float> Exp()
  {
    std::vector<float> out;
    out.reserve(logProbs.size());
    BOOST_FOREACH(float logProb, logProbs) {
      out.push_back(exp(logProb));
    }
    return out;
  }

  float operator[] (int pos) { return logProbs[pos]; }

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
  std::vector<float> logProbs;
  float maxProb;
};

boost::iostreams::filtering_istream *InitInput(const std::string &fileName = "");
boost::iostreams::filtering_ostream *InitOutput(const std::string &fileName = "");

#endif // UTILS_HPP_
