#include "Options.hpp"
#include "Utils.hpp"

#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace boost::algorithm;
using namespace boost::program_options;
using namespace std;

void Options::ParseOptions(int argc, char **argv)
{
  options_description opts("Allowed options");

  opts.add_options()
    ("output-prefix,o", value<string>(&outputPrefix)->default_value("align"),
     "Prefix of created output files.")
    ("aggregate-from,g", value<size_t>(&aggregateFrom)->default_value(10),
     "Iteration from which samples are aggregated for the final word alignment.")
    ("iterations,t", value<size_t>(&iterations)->default_value(20),
     "Number of iterations.")
    ("cognate-boost,b", value<float>(&cognateBoost)->default_value(1),
     "Prior co-efficient for cognate words (higher number implies stronger prior).")
    ("alpha-lex,a", value<float>(&alphaLex)->default_value(0.01),
     "Value of the uniform Dirichlet prior on lexical probability.")    
    ("alpha-dist,a", value<float>(&alphaDist)->default_value(1),
     "Value of the uniform Dirichlet prior on distortion probability.")    
    ("input-file,i", value<string>(&inputFile)->default_value(""),
     "Input file, default is STDIN.")
    ("help,h", value<bool>(&help)->zero_tokens()->default_value(false),
     "Print this message.")
    ("compress,c", value(&compress)->zero_tokens()->default_value(false),
     "Gzip output files.");
  try {
    variables_map vmap;
    store(parse_command_line(argc, argv, opts), vmap);
    notify(vmap);
  } catch (...) {
    std::cerr << opts << std::endl;
    Die("Wrong command-line arguments");
  }
  if (help) {
    std::cerr << opts << std::endl;
    exit(0);
  }
}
