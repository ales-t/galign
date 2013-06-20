#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <string>
#include <vector>
#include <boost/iostreams/filtering_stream.hpp> 
#include <iterator>
#include <tbb/atomic.h>

#include "Utils.hpp"

//
// serialization
//

template<typename T>
class ValueWriter
{
public:
  void Write(OutStreamType &outStream, const T &what)
  {
    outStream << what << "\n";
  }
};

template<typename T>
class MapWriter
{
public:
  void Write(OutStreamType &outStream, const T &what)
  {
    outStream << what.size() << "\n";
    typename T::const_iterator it = what.begin();
    while (it != what.end()) {
      outStream << it->first << "\t" << it->second << "\n";
      ++it;
    }
  }
};

template<typename T>
class IterableWriter
{
public:
  void Write(OutStreamType &outStream, const T &what)
  {
    outStream << what.size() << "\n";
    typename T::const_iterator it = what.begin();
    while (it != what.end()) outStream << *it << "\n";
  }
};

//
// deserialization
//

template<typename T>
class ValueReader
{
public:
  T Read(InStreamType &inStream)
  {
    T out;
    inStream >> out;
    return out;
  }
};

template<>
class ValueReader <tbb::atomic<int> >
{
public:
  tbb::atomic<int> Read(InStreamType &inStream)
  {
    int tmp;
    tbb::atomic<int> out;
    inStream >> tmp;
    out = tmp;
    return out;
  }
};

// why do we pass KeyT and ValueT? Boost::bimap craziness
template<typename T, typename KeyT, typename ValueT>
class MapReader
{
public:
  void Read(InStreamType &inStream, T &what)
  {
    size_t size = ValueReader<size_t>().Read(inStream);

    for (size_t i = 0; i < size; i++) {
      KeyT key = ValueReader<KeyT>().Read(inStream);
      ValueT value = ValueReader<ValueT>().Read(inStream);
      what.insert(typename T::value_type(key, value));
    }
  }
};

template<typename T>
class IterableReader
{
public:
  T Read(InStreamType &inStream)
  {
    size_t size = ValueReader<size_t>().Read(inStream);

    T out;
    typename std::insert_iterator<T> inserter(out, out.begin());
    ValueReader<typename T::value_type> reader;
    for (size_t i = 0; i < size; i++)
      inserter = reader.Read(inStream);
    return out;
  }
};

#endif // SERIALIZER_HPP_
