#include "HMM.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace boost::random;

HMM::HMM(Corpus *corpus, float alpha, float cognateAlpha, float distAlpha, const CountType &prevCounts,
    const JointCountType &prevJoint)
: corpus(corpus), alpha(alpha), cognateAlpha(cognateAlpha), distAlpha(distAlpha), counts(prevCounts),
  jointCounts(prevJoint)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (size_t i = 0; i < sentence->src.size(); i++) {
      int distortion = 1;
      if (i > 0)
        distortion = sentence->align[i] - sentence->align[i - 1];
      distortionCounts[distortion]++;
    }
  }
  order = corpus->GetTokensToSentences();
}

void HMM::RunIteration(bool doAggregate)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  #pragma omp parallel for
  for (size_t posIt = 0; posIt < order.size(); posIt++) {
    pair<size_t, size_t> sentPos = order[posIt];
    Sentence *sentence = sentences[sentPos.first];
    size_t srcWord = sentence->src[sentPos.second];
    size_t oldTgtWord = sentence->tgt[sentence->align[sentPos.second]];
    
    // discount removed alignment link
    if (--jointCounts[srcWord][oldTgtWord] <= 0) jointCounts[srcWord].erase(oldTgtWord);
    counts[oldTgtWord]--;
    int inDistortion = 1;
    if (sentPos.second > 0) {
      inDistortion = sentence->align[sentPos.second] - sentence->align[sentPos.second - 1];
    }
    distortionCounts[inDistortion] = max(0, distortionCounts[inDistortion] - 1);
    int outDistortion = 1;
    if (sentPos.second < sentence->src.size() - 1) {
      outDistortion = sentence->align[sentPos.second + 1] - sentence->align[sentPos.second];
    }
    distortionCounts[outDistortion] = max(0, distortionCounts[outDistortion] - 1);

    // generate a sample
    LogDistribution lexicalProbs;
    LogDistribution inDistortionPotentials;
    LogDistribution outDistortionPotentials;

    for (size_t i = 0; i < sentence->tgt.size(); i++) {
      size_t tgt = sentence->tgt[i];
      float pairAlpha = alpha;
      float normAlpha = alpha * corpus->GetSrcTypes().size();
      if (srcWord == tgt)
        pairAlpha = cognateAlpha;
      if (corpus->HasCognate(tgt))
        normAlpha += cognateAlpha - alpha;

      float logLexProb = log(jointCounts[srcWord][tgt] + pairAlpha) - log(counts[tgt] + normAlpha);
      lexicalProbs.Add(logLexProb);
      int inDistortion = 1;
      if (sentPos.second > 0) {
        inDistortion = i - sentence->align[sentPos.second - 1];
      }
      int outDistortion = 1;
      if (sentPos.second < sentence->src.size() - 1) {
        outDistortion = sentence->align[sentPos.second + 1] - i;
      }
      int inDistCount = 0;
      if (distortionCounts.contains(inDistortion))
        inDistCount = distortionCounts[inDistortion];
      inDistortionPotentials.Add(log(inDistCount + distAlpha));
      int outDistCount = 0;
      if (distortionCounts.contains(outDistortion))
        outDistCount = distortionCounts[outDistortion];
      outDistortionPotentials.Add(log(outDistCount + distAlpha));
    }

    // get distribution parameters
    vector<float> distParams;
    distParams.reserve(sentence->tgt.size());
    lexicalProbs.Normalize();
    inDistortionPotentials.Normalize();
    outDistortionPotentials.Normalize();
    for (size_t i = 0; i < sentence->tgt.size(); i++) {
      distParams.push_back(exp(lexicalProbs[i] + inDistortionPotentials[i] + outDistortionPotentials[i]));
    }

    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;
    inDistortion = 1;
    outDistortion = 1;
    if (sentPos.second > 0) {
      inDistortion = sample - sentence->align[sentPos.second - 1];
    }
    if (sentPos.second < sentence->src.size() - 1) {
      outDistortion = sentence->align[sentPos.second + 1] - sample;
    }
    distortionCounts[inDistortion]++;
    distortionCounts[outDistortion]++;
    if (doAggregate) {
      aggregateJoint[srcWord][sentence->tgt[sample]]++;
      aggregateCounts[sentence->tgt[sample]]++;
      aggregateDistortion[inDistortion]++;
      aggregateDistortion[outDistortion]++;
    }
  }
}

vector<AlignmentType> HMM::GetAggregateAlignment()
{
  vector<AlignmentType> out;
//  BOOST_FOREACH(DistortionCountType::value_type dist, aggregateDistortion) {
//    cerr << dist.first << ":\t" << dist.second << endl;
//  }
  int lineNum = 0;
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    lineNum++;
    AlignmentType aggregAlign(sentence->src.size());
    for (size_t i = 0; i < sentence->src.size(); i++) {
      LogDistribution lexicalProbs;
      LogDistribution inDistortionPotentials;
      LogDistribution outDistortionPotentials;

      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        float pairAlpha = alpha;
        float normAlpha = alpha * corpus->GetSrcTypes().size();
        if (sentence->src[i] == sentence->tgt[j])
          pairAlpha = cognateAlpha;
        if (corpus->HasCognate(sentence->tgt[j]))
          normAlpha += cognateAlpha - alpha;

        float logLexProb = log(aggregateJoint[sentence->src[i]][sentence->tgt[j]] + pairAlpha)        
          - log(aggregateCounts[sentence->tgt[j]] + normAlpha);
        lexicalProbs.Add(logLexProb);

        int inDistortion = 1;
        if (i > 0) {
          inDistortion = j - sentence->align[i - 1];
        }
        int outDistortion = 1;
        if (i < sentence->src.size() - 1) {
          outDistortion = sentence->align[i + 1] - j;
        }
        int inDistCount = 0;
        if (aggregateDistortion.contains(inDistortion))
          inDistCount = aggregateDistortion[inDistortion];
        inDistortionPotentials.Add(log(inDistCount + distAlpha));
        int outDistCount = 0;
        if (aggregateDistortion.contains(outDistortion))
          outDistCount = aggregateDistortion[outDistortion];
        outDistortionPotentials.Add(log(outDistCount + distAlpha));
      }

      lexicalProbs.Normalize();
      inDistortionPotentials.Normalize();
      outDistortionPotentials.Normalize();
      int best = -1;
      float bestProb = 0;
      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        float prob = (exp(lexicalProbs[j] + inDistortionPotentials[j] + outDistortionPotentials[j]));
        if (prob > bestProb) {
          best = j;
          bestProb = prob;
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
