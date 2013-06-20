#include "Utils.hpp"

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp> 
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filtering_stream.hpp> 
#include <boost/algorithm/string/predicate.hpp>

using namespace boost::algorithm;
using namespace boost::iostreams;
using namespace boost;
using namespace std;

InStreamType *InitInput(const string &fileName)
{
  InStreamType *in = new InStreamType();
  if (fileName.empty()) {
    in->push(cin);
  } else {
    if (ends_with(fileName, ".gz"))
      in->push(gzip_decompressor());
    file_source fileSrc(fileName.c_str());
    if (! fileSrc.is_open())
      Die("Cannot read file: " + fileName);
    in->push(fileSrc);
  }
  return in;
}

OutStreamType *InitOutput(const string &fileName)
{
  OutStreamType *out = new OutStreamType();
  if (fileName.empty()) {
    out->push(cout);
  } else {
    if (ends_with(fileName, ".gz"))
      out->push(gzip_compressor());
    file_sink fileSink(fileName.c_str());
    if (! fileSink.is_open())
      Die("Cannot write to file: " + fileName);
    out->push(fileSink);
  }
  return out;
}

