#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <omp.h>

#include "Options.hpp"
#include "Corpus.hpp"
#include "Model1.hpp"
#include "Writer.hpp"
#include "Utils.hpp"
#include "HMM.hpp"
#include "AlignmentModel.hpp"

using namespace std;

int main(int argc, char **argv)
{
  // parse options
  Options &opts = Options::Instance();
  opts.ParseOptions(argc, argv);
  int cores = opts.GetCores();
  if (cores == 0) {
    cores = omp_get_num_procs();
  }
  omp_set_num_threads(cores);
  Log("Using " + boost::lexical_cast<string>(cores) + " CPU cores");

  // load corpus
  Corpus *corpus = new Corpus(opts.GetInputFile());
  Log("Corpus loaded.");

  // initialize IBM Model 1
  Model1 model1(corpus, opts.GetLexicalAlpha(), opts.GetCognateAlpha());
  model1.AlignRandomly();
  AlignmentModel *lastModel = &model1;
  Log("Initialized Model1");

  // run Model 1 iterations
  for (size_t i = 1; i <= opts.GetIBM1Iterations(); i++) {
    model1.RunIteration(i >= opts.GetIBM1AggregateFrom());
    Log("Model1: Finished iteration " + boost::lexical_cast<string>(i));
  }

  // initialize HMM model, use counts from IBM Model 1
  if (opts.GetHMMIterations() > 0) {
    HMM *hmmModel = new HMM(corpus, opts.GetLexicalAlpha(), opts.GetCognateAlpha(),
        opts.GetDistortionAlpha(), model1.GetCounts(), model1.GetJointCounts());
    lastModel = hmmModel;
    Log("Initialized HMM");

    // run HMM model iterations
    for (size_t i = 1; i <= opts.GetHMMIterations(); i++) {
      hmmModel->RunIteration(i >= opts.GetHMMAggregateFrom());
      Log("HMM: Finished iteration " + boost::lexical_cast<string>(i));
    }
  }

  // output last alignment and aggregate alignment
  Log("Writing alignments to disk.");
  Writer writer(corpus);
  string suffix = opts.GetCompress() ? ".gz" : "";
  writer.WriteAlignment(opts.GetOutputPrefix() + ".last" + suffix);
  vector<AlignmentType> aggregAlign = lastModel->GetAggregateAlignment();
  writer.WriteAlignment(opts.GetOutputPrefix() + ".aggregate"
      + suffix, aggregAlign);
  writer.WriteAlignment(opts.GetOutputPrefix() + ".aggregate.giza"
      + suffix, aggregAlign, true);
  Log("Done.");

  return 0;
}
