#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

#include "Options.hpp"
#include "Reader.hpp"

using namespace std;
using namespace boost;
using namespace boost::random;

unordered_map<string, unordered_map<string, int> > jointCounts, aggregateJoint; 
unordered_map<string, int> counts, aggregateCounts;
set<string> hasCognate;

// main

int main(int argc, char **argv)
{

    entry->align.reserve(entry->src.size());

    // align and collect counts
    uniform_int_distribution<int> dist(0, entry->tgt.size() - 1);
    for (int i = 0; i < entry->src.size(); i++) {
      srcTypes.insert(entry->src[i]);
      tokensToSents.insert(make_pair(totalTokens++, make_pair(lineNum - 1, i))); // 0-based
      int tgtWord = dist(generator);
      jointCounts[entry->src[i]][entry->tgt[tgtWord]]++;
      entry->align.push_back(tgtWord);
      for (auto t : entry->tgt)
        if (t == entry->src[i]) hasCognate.insert(t);
    }
    for (auto w : entry->tgt) counts[w]++;
    corpus.push_back(entry);
  }
  cerr << "Input read" << endl;

  vector<int> order;
  order.reserve(totalTokens);
  for (int i = 0; i < totalTokens; i++) order.push_back(i);

  for (int i = 0; i < iterations; i++) {
    cerr << "At iteration " << i << endl;
    random_shuffle(order.begin(), order.end());

    // over all words in corpus (in random order)
    for (auto pos : order) {
      pair<int, int> sentPos = tokensToSents[pos];      
      Entry *entry = corpus[sentPos.first];
      const string &srcWord = entry->src[sentPos.second];
      const string &tgtWord = entry->tgt[entry->align[sentPos.second]];
      
      // discount removed alignment link
      if (--jointCounts[srcWord][tgtWord] <= 0) jointCounts[srcWord].erase(tgtWord);
      counts[tgtWord]--;

      // generate a sample
      vector<float> distParams;
      distParams.reserve(entry->tgt.size());
      for (auto tgt : entry->tgt) {
        float pairAlpha = alpha;
        if (boostCognates && srcWord == tgt) pairAlpha *= 10;
        float prob = (pairAlpha + jointCounts[srcWord][tgt]) /
          (counts[tgt] + alpha*(srcTypes.size() - 1) + pairAlpha);
        distParams.push_back(prob);
      }
      discrete_distribution<int> dist(distParams.begin(), distParams.end());
      int sample = dist(generator);

      // update counts with new alignment link
      entry->align[sentPos.second] = sample;
      jointCounts[srcWord][entry->tgt[sample]]++;
      counts[entry->tgt[sample]]++;
      if (i > aggregate_after) {
        aggregateJoint[srcWord][entry->tgt[sample]]++;
        aggregateCounts[entry->tgt[sample]]++;
      }
    }
  }

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

  return 0;
}
