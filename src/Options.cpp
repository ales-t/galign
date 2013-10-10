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
    ("input-file,i", value<string>(&inputFile)->default_value(""),
     "Input file, default is STDIN.")
    ("output-file,o", value<string>(&outputFile)->default_value(""),
     "Output file, default is STDOUT.")
    ("moses-format,m", value<bool>(&mosesFormat)->zero_tokens()->default_value(false),
     "Print alignments in Moses format (0-0 1-0...).")
    ("force-alignment,f", value<bool>(&forceAlign)->zero_tokens()->default_value(false),
     "Do not train, only align using an existing model")
    ("no-viterbi", value<bool>(&noViterbi)->zero_tokens()->default_value(false),
     "Skip the final search for the most probable alignment (slow for HMM), "
     "output the last Gibbs sample instead.")
    ("alpha-lex,a", value<float>(&alphaLex)->default_value(0.01),
     "Value of the uniform Dirichlet prior on lexical probability.")    
    ("ibm1-iterations", value<size_t>(&ibm1Iterations)->default_value(20),
     "Number of iterations.")
    ("ibm1-cooling-from", value<size_t>(&ibm1CoolingFrom)->default_value(15),
     "Iteration from which samples are aggregated for the final word alignment.")
    ("hmm-null-prob", value<float>(&hmmNullProb)->default_value(0.2),
     "Probability of aligning a word to null.")    
    ("hmm-iterations", value<size_t>(&hmmIterations)->default_value(20),
     "Number of iterations.")
    ("hmm-cooling-from", value<size_t>(&hmmCoolingFrom)->default_value(15),
     "Iteration from which samples are aggregated for the final word alignment.")
    ("load-model-file", value<string>(&loadModelFile)->default_value(""),
     "Load an existing model file.")
    ("store-model-file", value<string>(&storeModelFile)->default_value(""),
     "Save the final model file.")
    ("cores", value<size_t>(&cores)->default_value(0),
     "The number of CPU cores to use, default 0 means autodetect.")
    ("boost-identical", value<size_t>(&cores)->default_value(0),
     "Add N artificial occurrences to word being aligned to itself, useful for monolingual alignment.")
    ("help,h", value<bool>(&help)->zero_tokens()->default_value(false),
     "Print this message.");
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
