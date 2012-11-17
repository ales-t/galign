#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

#include "Options.hpp"
#include "Corpus.hpp"
#include "Model1.hpp"

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
  Model1 model1(corpus, opts.GetAlpha(), opts.GetCognateAlpha());
  model1.AlignRandomly();
  for (int i = 0; i < opts.GetIterations(); i++) {
    model1.RunIteration(i > opts.GetAggregateAfter());
  }
/*
  ofstream lastAlign(outprefix + ".last");
  for (auto line : corpus) {
    for (int i = 0; i < line->src.size(); i++) {
      lastAlign << line->src[i] << "{" << line->tgt[line->align[i]] << "} ";
    }
    lastAlign << endl;
  }
  lastAlign.close();

  ofstream aggregAlign(outprefix + ".aggreg");
  for (auto line : corpus) {
    for (int i = 0; i < line->src.size(); i++) {
      aggregAlign << line->src[i] << "{";
      int best = -1;
      float bestProb = 0;
      for (int j = 0; j < line->tgt.size(); j++) {
        float pairAlpha = alpha;
        float normAlpha = srcTypes.size() * alpha;
        if (boostCognates) {
          if (line->src[i] == line->tgt[j])
            pairAlpha *= 10;
          if (hasCognate.find(line->tgt[j]) != hasCognate.end())
            normAlpha += 9 * alpha;
        } 
        float prob = (pairAlpha + aggregateJoint[line->src[i]][line->tgt[j]])
          / (aggregateCounts[line->tgt[j]] + normAlpha);
        if (prob > bestProb) {
          best = j;
          bestProb = prob;
        }      
      }
      aggregAlign << line->tgt[best] << "} ";
    }
    aggregAlign << endl;
  }
  aggregAlign.close();
*/
  return 0;
}
