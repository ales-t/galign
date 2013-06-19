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
    ("ibm1-cooling-from", value<size_t>(&ibm1CoolingFrom)->default_value(11),
     "Iteration from which samples are aggregated for the final word alignment.")
    ("ibm1-iterations", value<size_t>(&ibm1Iterations)->default_value(10),
     "Number of iterations.")
    ("hmm-cooling-from", value<size_t>(&hmmCoolingFrom)->default_value(10),
     "Iteration from which samples are aggregated for the final word alignment.")
    ("hmm-iterations", value<size_t>(&hmmIterations)->default_value(20),
     "Number of iterations.")
    ("alpha-lex,a", value<float>(&alphaLex)->default_value(0.01),
     "Value of the uniform Dirichlet prior on lexical probability.")    
    ("alpha-dist,a", value<float>(&alphaDist)->default_value(1),
     "Value of the uniform Dirichlet prior on distortion probability.")    
    ("input-file,i", value<string>(&inputFile)->default_value(""),
     "Input file, default is STDIN.")
    ("output-file,o", value<string>(&outputFile)->default_value(""),
     "Output file, default is STDOUT.")
    ("moses-format,m", value<bool>(&mosesFormat)->zero_tokens()->default_value(false),
     "Print alignments in Moses format (0-0 1-0...).")
    ("help,h", value<bool>(&help)->zero_tokens()->default_value(false),
     "Print this message.")
    ("cores", value<size_t>(&cores)->default_value(0),
     "The number of CPU cores to use, default 0 means autodetect.");
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
