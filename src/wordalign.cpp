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

void Run(AlignmentModel &model, int iterations, int coolingFrom, InStreamType *oldModel)
{
  if (oldModel) {
    model.ReadModel(*oldModel);
    close(*oldModel);
  }
  Annealer annealer(iterations, coolingFrom);
  for (int i = 1; i <= iterations; i++) {
    model.RunIteration(annealer.GetTemp(i));
    Log("Finished iteration " + boost::lexical_cast<string>(i));
  }
}

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

  InStreamType *oldModel = NULL;
  Corpus *corpus;
  if (! opts.GetLoadModelFile().empty()) {
    oldModel = InitInput(opts.GetLoadModelFile());
    corpus = new Corpus(opts.GetInputFile(), *oldModel);
  } else {
    corpus = new Corpus(opts.GetInputFile());
  }
  Log("Corpus loaded.");

  // IBM Model 1
  Model1 model1(corpus, opts.GetLexicalAlpha());
  model1.AlignRandomly();
  AlignmentModel *lastModel = &model1;
  Log("Initialized Model1");
  if (opts.GetIBM1Iterations() > 0) {
    if (oldModel && opts.GetHMMIterations() > 0) {
      Warn("Will load existing model, skipping IBM1 training.");
    } else {
      Run(model1, opts.GetIBM1Iterations(), opts.GetIBM1CoolingFrom(), oldModel);
    }
  }

  // HMM
  if (opts.GetHMMIterations() > 0) {
    HMM *hmmModel = new HMM(corpus, opts.GetLexicalAlpha(), opts.GetDistortionAlpha(),
        model1.GetCounts(), model1.GetJointCounts());
    lastModel = hmmModel;
    Log("Initialized HMM");
    Run(*hmmModel, opts.GetHMMIterations(), opts.GetHMMCoolingFrom(), oldModel);
  }

  // optionally store trained model
  string modelFile = opts.GetStoreModelFile();
  if (! modelFile.empty()) {
    OutStreamType *out = InitOutput(modelFile);
    corpus->WriteIndex(*out);
    lastModel->WriteModel(*out);
    close(*out);
  }

  // output last alignment and aggregate alignment
  Log("Writing alignments.");  
  Writer writer(corpus);
  writer.WriteAlignment(opts.GetOutputFile(), opts.GetMosesFormat());
  Log("Done.");

  return 0;
}
