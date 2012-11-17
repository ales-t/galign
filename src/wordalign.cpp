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

using namespace std;

// main

int main(int argc, char **argv)
{
  Options &opts = Options::Instance();
  opts.ParseOptions(argc, argv);
  Corpus *corpus;
  string fileName = opts.GetInputFile();
  if (fileName.empty()) {
    corpus = new Corpus(fileName);
  } else {
    corpus = new Corpus();
  }
  Log("Corpus loaded.");
  Model1 model1(corpus, opts.GetAlpha(), opts.GetCognateAlpha());
  model1.AlignRandomly();
  for (int i = 0; i < opts.GetIterations(); i++) {
    model1.RunIteration(i > opts.GetAggregateAfter());
    Log("Finished iteration " + boost::lexical_cast<string>(i));
  }

  Writer writer(corpus);
  string suffix = opts.GetCompress() ? ".gz" : "";
  writer.WriteAlignment(opts.GetOutputPrefix() + ".last" + suffix);
  writer.WriteAlignment(opts.GetOutputPrefix() + ".aggregate" + suffix, model1.GetAggregateAlignment());

  return 0;
}
