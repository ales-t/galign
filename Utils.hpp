#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <string>
#include <vector>

inline void Warn(const std::string &msg)
{
  std::cerr << "WARNING: " << msg << std::endl;
}

inline void Die(const std::string &msg)
{
  std::cerr << "ERROR: " << msg << std::endl;
  exit(1);
}

#endif // UTILS_HPP_
