#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <string>
#include <vector>
#include <boost/iostreams/filtering_stream.hpp> 
#include <iterator>

#include "Utils.hpp"

//
// serialization
//
template<typename T>
class IterableWriter
{
public:
  void Write(OutStreamType &outStream, const T &obj)
  {
    ValueWriter<size_t> sizeWriter;
    sizeWriter.Write(outStream, obj.size());

    typename T::const_iterator it = obj.begin();
    ValueWriter<typename T::value_type> writer;
    while (it != obj.end()) writer.Write(outStream, *it++);
  }
};

template<typename T>
class ValueWriter
{
public:
  void Write(OutStreamType &outStream, const T &obj)
  {
    outStream << obj << "\n";
  }
};

template<typename KeyT, typename ValueT>
class ValueWriter <std::pair<KeyT, ValueT> >
{
public:
  void Write(OutStreamType &outStream, const std::pair<KeyT, ValueT> &obj)
  {
    outStream << obj.first << "\t" << obj.second << "\n";
  }
};

//
// deserialization
//
template<typename T>
class IterableReader
{
public:
  T Read(InStreamType &inStream)
  {
    size_t size = ValueReader<size_t>().read(inStream);

    T out;
    typename T::iterator it = out.begin();
    typename std::insert_iterator<T> inserter(out, begin);
    ValueReader<typename T::value_type> reader;
    for (size_t i = 0; i < size; i++)
      inserter = reader.Read(inStream);
    return out;
  }
};

template<typename T>
class ValueReader
{
public:
  void Read(InStreamType &inStream)
  {
    T out;
    inStream << out;
    return out;
  }
};

template<>
class ValueReader <tbb::atomic<int> >
{
public:
  void Read(InStreamType &inStream)
  {
    tbb::atomic<int> out;
    inStream << (int)out;
    return out;
  }
};

template<typename KeyT, typename ValueT>
class ValueReader <std::pair<KeyT, ValueT> >
{
public:
  void Read(InStreamType &inStream, const std::pair<KeyT, ValueT> &obj)
  {
    KeyT key = ValueReader<KeyT>().Read(inStream);
    ValueT value = ValueReader<ValueT>.Read(inStream);
    return make_pair<KeyT, ValueT>(key, value);
  }
};

#endif // SERIALIZER_HPP_
