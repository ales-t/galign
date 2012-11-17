#include "Model1.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace boost::random;

void Model1::AlignRandomly()
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    if (! sentence->align.empty())
      Die("Attempted to overwrite existing alignment with random initialization");
    sentence->align.reserve(sentence->src.size());

    // align and collect counts
    uniform_int_distribution<int> dist(0, sentence->tgt.size() - 1);
    for (int i = 0; i < sentence->src.size(); i++) {
      int tgtWord = dist(generator);
      jointCounts[sentence->src[i]][sentence->tgt[tgtWord]]++;
      sentence->align.push_back(tgtWord);
      for (int j = 0; j < sentence->tgt.size(); j++) {
        if (sentence->tgt[j] == sentence->src[i])
          hasCognate.insert(sentence->tgt[j]);
      }
    }
    for (int i = 0; i < sentence->tgt.size(); i++)
      counts[sentence->tgt[i]]++;
  }
}

void Model1::RunIteration(bool doAggregate)
{
  vector<int> order;
  order.reserve(corpus->GetTotalSourceTokens());
  for (int i = 0; i < corpus->GetTotalSourceTokens(); i++)
    order.push_back(i);

  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  BOOST_FOREACH(int position, order) {
    pair<int, int> sentPos = corpus->GetSentenceAndPosition(position);
    Sentence *sentence = sentences[sentPos.first];
    const string &srcWord = sentence->src[sentPos.second];
    const string &prevTgtWord = sentence->tgt[sentence->align[sentPos.second]];
    
    // discount removed alignment link
    if (--jointCounts[srcWord][prevTgtWord] <= 0) jointCounts[srcWord].erase(prevTgtWord);
    counts[prevTgtWord]--;

    // generate a sample
    vector<float> distParams;
    distParams.reserve(sentence->tgt.size());
    BOOST_FOREACH(string tgt, sentence->tgt) {
      float pairAlpha = alpha;
      float normAlpha = alpha * corpus->GetSrcTypes().size();
      if (srcWord == tgt)
        pairAlpha = cognateAlpha;
      if (hasCognate.find(tgt) != hasCognate.end())
        normAlpha += cognateAlpha - alpha;

      float prob = (jointCounts[srcWord][tgt] + pairAlpha) / (counts[tgt] + normAlpha);
      distParams.push_back(prob);
    }
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;
    if (doAggregate) {
      aggregateJoint[srcWord][sentence->tgt[sample]]++;
      aggregateCounts[sentence->tgt[sample]]++;
    }
  }
}

vector<AlignmentType> Model1::GetAggregateAlignment()
{
  vector<AlignmentType> out;
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    AlignmentType aggregAlign(sentence->src.size());
    for (int i = 0; i < sentence->src.size(); i++) {
      int best = -1;
      float bestProb = 0;
      for (int j = 0; j < sentence->tgt.size(); j++) {
        float pairAlpha = alpha;
        float normAlpha = alpha * corpus->GetSrcTypes().size();
        if (sentence->src[i] == sentence->tgt[j])
          pairAlpha = cognateAlpha;
        if (hasCognate.find(sentence->tgt[j]) != hasCognate.end())
          normAlpha += cognateAlpha - alpha;

        float prob = (aggregateJoint[sentence->src[i]][sentence->tgt[j]] + pairAlpha)
          / (aggregateCounts[sentence->tgt[j]] + normAlpha);
        if (prob > bestProb) {
          best = j;
          bestProb = prob;
        }      
      }
      aggregAlign[i] = best;
    }
    out.push_back(aggregAlign);
  }
  return out;
}
