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
  Model1 model1(corpus, opts.GetAlpha(), opts.GetCognateAlpha());
  model1.AlignRandomly();

  // run model iterations
  for (int i = 0; i < opts.GetIterations(); i++) {
    model1.RunIteration(i > opts.GetAggregateAfter());
    Log("Model1: Finished iteration " + boost::lexical_cast<string>(i));
  }

  HMM hmmModel(corpus, opts.GetAlpha(), opts.GetCognateAlpha(), model1.GetCounts(),
      model1.GetJointCounts());

  // run model iterations
  for (int i = 0; i < opts.GetIterations(); i++) {
    hmmModel.RunIteration(i > opts.GetAggregateAfter());
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
