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

void RunModel(AlignmentModel &model, int iterations, int coolingFrom,
     InStreamType *oldModel, bool force)
{
  if (oldModel) {
    model.ReadModel(*oldModel);
    close(*oldModel);
  }
  if (force) {
    Log("Asked for forced alignment, skipping training");
  } else {
    model.UpdateFromCorpus();
    Annealer annealer(iterations, coolingFrom);
    for (int i = 1; i <= iterations; i++) {
      model.RunIteration(annealer.GetTemp(i));
      Log("Finished iteration " + boost::lexical_cast<string>(i));
    }
  }
}

// modes:
// - train, align (+ dump models)
// - load models, continue training, align (+dump updated models)
// - load models, force align, don't train

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
  if (opts.GetForceAlign() && opts.GetLoadModelFile().empty()) {
    Die("Asking for forced alignment but no existing model given");
  }

  // load corpus
  InStreamType *oldModel = NULL;
  Corpus *corpus;
  if (! opts.GetLoadModelFile().empty()) {
    oldModel = InitInput(opts.GetLoadModelFile());
    corpus = new Corpus(opts.GetInputFile(), *oldModel);
  } else {
    corpus = new Corpus(opts.GetInputFile());
  }
  Log("Corpus loaded.");

  AlignmentModel *lastModel = NULL;

  // IBM model 1
  if (opts.GetIBM1Iterations() > 0) {
    Model1 *model1 = new Model1(corpus, opts.GetLexicalAlpha());
    lastModel = model1;
    if (oldModel && opts.GetHMMIterations() > 0) {
      Warn("Will load existing model, skipping IBM1 training.");
    } else {
      RunModel(*model1, opts.GetIBM1Iterations(), opts.GetIBM1CoolingFrom(), oldModel, opts.GetForceAlign());
    }
  }

  // HMM
  if (opts.GetHMMIterations() > 0) {
    HMM *hmmModel = new HMM(corpus, opts.GetLexicalAlpha(), opts.GetHMMNullProb());
    lastModel = hmmModel;
    Log("Initialized HMM");
    RunModel(*hmmModel, opts.GetHMMIterations(), opts.GetHMMCoolingFrom(), oldModel, opts.GetForceAlign());
  }
  
  // optionally store trained model
  string modelFile = opts.GetStoreModelFile();
  if (! modelFile.empty()) {
    Log("Writing model file");
    OutStreamType *out = InitOutput(modelFile);
    corpus->WriteIndex(*out);
    lastModel->WriteModel(*out);
    close(*out);
  }
  
  // calculate Viterbi alignment
  Log("Computing Viterbi alignments");
  lastModel->Viterbi();
 
  // output final word alignment
  Log("Writing alignments.");  
  Writer writer(corpus);
  writer.WriteAlignment(opts.GetOutputFile(), opts.GetMosesFormat());
  Log("Done.");

  return 0;
}
