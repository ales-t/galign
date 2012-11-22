#include "Model1.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>
#include <tbb/concurrent_vector.h>
#include <cmath>
#include <omp.h>

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
    for (size_t i = 0; i < sentence->src.size(); i++) {
      int tgtWord = dist(generator);
      jointCounts.at(sentence->src[i]).at(sentence->tgt[tgtWord])++;
      sentence->align.push_back(tgtWord);
    }
    for (size_t i = 0; i < sentence->tgt.size(); i++)
      counts.at(sentence->tgt[i])++;
  }
}

void Model1::RunIteration(bool doAggregate)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  #pragma omp parallel for
  for (size_t posIt = 0; posIt < order.size(); posIt++) {
    pair<size_t, size_t> sentPos = order[posIt];
    Sentence *sentence = sentences[sentPos.first];
    const string &srcWord = sentence->src[sentPos.second];
    const string &oldTgtWord = sentence->tgt[sentence->align[sentPos.second]];
    
    // discount removed alignment link
    if (--jointCounts.at(srcWord).at(oldTgtWord) <= 0) jointCounts.at(srcWord).erase(oldTgtWord);
    counts.at(oldTgtWord)--;

    // generate a sample
    LogDistribution lexicalProbs;
    BOOST_FOREACH(const string &tgt, sentence->tgt) {
      float pairAlpha = alpha;
      float normAlpha = alpha * corpus->GetSrcTypes().size();
      if (srcWord == tgt)
        pairAlpha = cognateAlpha;
      if (corpus->HasCognate(tgt))
        normAlpha += cognateAlpha - alpha;

      //float logProb = log(jointCounts[srcWord][tgt] + pairAlpha) - log(counts[tgt] + normAlpha);
      float logProb = log(jointCounts[srcWord][tgt] + pairAlpha) - log(counts[tgt] + normAlpha);
      lexicalProbs.Add(logProb);
    }
    lexicalProbs.Normalize();
    vector<float> distParams = lexicalProbs.Exp();
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts.at(srcWord).at(sentence->tgt[sample])++;
    counts.at(sentence->tgt[sample])++;
    if (doAggregate) {
      aggregateJoint.at(srcWord).at(sentence->tgt[sample])++;
      aggregateCounts.at(sentence->tgt[sample])++;
    }
  }
}

vector<AlignmentType> Model1::GetAggregateAlignment()
{
  vector<AlignmentType> out;
  int lineNum = 0;
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    lineNum++;
    AlignmentType aggregAlign(sentence->src.size());
    for (size_t i = 0; i < sentence->src.size(); i++) {
      int best = -1;
      float bestProb = -numeric_limits<float>::infinity();
      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        float pairAlpha = alpha;
        float normAlpha = alpha * corpus->GetSrcTypes().size();
        if (sentence->src[i] == sentence->tgt[j])
          pairAlpha = cognateAlpha;
        if (corpus->HasCognate(sentence->tgt[j]))
          normAlpha += cognateAlpha - alpha;

        float logProb = log(aggregateJoint[sentence->src[i]][sentence->tgt[j]] + pairAlpha)
          - log(aggregateCounts[sentence->tgt[j]] + normAlpha);
        if (logProb > bestProb) {
          best = j;
          bestProb = logProb;
        }      
      }
      if (best == -1) {
        Die("Zero probability for word '" + sentence->src[i] +
            "' in sentence " + lexical_cast<string>(lineNum));
      }
      aggregAlign[i] = best;
    }
    out.push_back(aggregAlign);
  }
  return out;
}
