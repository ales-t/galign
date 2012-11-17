#include "Options.hpp"
#include "Utils.h"

#include <boost/program_options.hpp>

using namespace boost::program_options;

void Options::ParseOptions(int argc, char **argv)
{
  options_description opts("Allowed options");

  opts.add_options()
    ("output-prefix,o", value<string>(&outprefix)->default_value("align"),
     "Prefix of created output files.")
    ("aggregate-after,g", value<int>(&aggregateAfter)->default_value(10),
     "Iteration after which samples are aggregated for the final word alignment.")
    ("cognate-boost,b", value<float>(&cognateBoost)->default_value(1),
     "Prior co-efficient for cognate words (higher number implies stronger prior).")
    ("alpha,a", value<float>(&alpha)->default_value(0.01),
     "Value of the uniform Dirichlet prior.");
  try {
    variables_map vmap;
    store(parse_command_line(argc, argv, opts), vmap);
    notify(vmap);
  } catch (...) {
    std::cerr << opts << std::endl;
    Die("Wrong command-line arguments");
  }
}
