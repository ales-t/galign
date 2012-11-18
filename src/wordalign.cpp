#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include "Options.hpp"
#include "Corpus.hpp"
#include "Model1.hpp"
#include "Writer.hpp"
#include "Utils.hpp"
#include "HMM.hpp"

using namespace std;

int main(int argc, char **argv)
{
  // parse options
  Options &opts = Options::Instance();
  opts.ParseOptions(argc, argv);

  // load corpus
  Corpus *corpus = new Corpus(opts.GetInputFile());
  Log("Corpus loaded.");

  // initialize IBM Model 1
  Model1 model1(corpus, opts.GetLexicalAlpha(), opts.GetCognateAlpha());
  model1.AlignRandomly();
  Log("Initialized Model1");

  // run Model 1 iterations
  for (size_t i = 1; i <= opts.GetIterations(); i++) {
    model1.RunIteration(i >= opts.GetAggregateFrom());
    Log("Model1: Finished iteration " + boost::lexical_cast<string>(i));
  }

  // initialize HMM model, use counts from IBM Model 1
  HMM hmmModel(corpus, opts.GetLexicalAlpha(), opts.GetCognateAlpha(),
      opts.GetDistortionAlpha(), model1.GetCounts(), model1.GetJointCounts());
  Log("Initialized HMM");

  // run HMM model iterations
  for (size_t i = 1; i <= opts.GetIterations(); i++) {
    hmmModel.RunIteration(i >= opts.GetAggregateFrom());
    Log("HMM: Finished iteration " + boost::lexical_cast<string>(i));
  }

  // output last alignment and aggregate alignment
  Log("Writing alignments to disk.");
  Writer writer(corpus);
  string suffix = opts.GetCompress() ? ".gz" : "";
  writer.WriteAlignment(opts.GetOutputPrefix() + ".last" + suffix);
  writer.WriteAlignment(opts.GetOutputPrefix() + ".aggregate"
      + suffix, hmmModel.GetAggregateAlignment());
  Log("Done.");

  return 0;
}
