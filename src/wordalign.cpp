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
  Model1 model1(corpus, opts.GetLexicalAlpha());
  model1.AlignRandomly();
  AlignmentModel *lastModel = &model1;
  Annealer model1Annealer(opts.GetIBM1Iterations(), opts.GetIBM1CoolingFrom());
  Log("Initialized Model1");

  // run Model 1 iterations
  for (size_t i = 1; i <= opts.GetIBM1Iterations(); i++) {
    model1.RunIteration(model1Annealer.GetTemp(i));
    Log("Model1: Finished iteration " + boost::lexical_cast<string>(i));
  }

  // initialize HMM model, use counts from IBM Model 1
  if (opts.GetHMMIterations() > 0) {
    HMM *hmmModel = new HMM(corpus, opts.GetLexicalAlpha(), opts.GetDistortionAlpha(),
        model1.GetCounts(), model1.GetJointCounts());
    lastModel = hmmModel;
    Annealer hmmAnnealer(opts.GetHMMIterations(), opts.GetHMMCoolingFrom());
    Log("Initialized HMM");

    // run HMM model iterations
    for (size_t i = 1; i <= opts.GetHMMIterations(); i++) {
      hmmModel->RunIteration(hmmAnnealer.GetTemp(i));
      Log("HMM: Finished iteration " + boost::lexical_cast<string>(i));
    }
  }

  // output last alignment and aggregate alignment
  Log("Writing alignments.");
  Writer writer(corpus);
  writer.WriteAlignment(opts.GetOutputFile(), opts.GetMosesFormat());
  Log("Done.");

  return 0;
}
