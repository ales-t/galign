#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <boost/iostreams/filtering_stream.hpp> 

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

boost::iostreams::filtering_istream *InitInput(const std::string &fileName = "");
boost::iostreams::filtering_ostream *InitOutput(const std::string &fileName = "");

#endif // UTILS_HPP_
