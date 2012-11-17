#ifndef MODEL1_HPP_
#define MODEL1_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

class Model1
{
public:

private:
  boost::random::mt19937 generator;
  std::vector<Entry *> *corpus;
};

#endif // MODEL1_HPP_
