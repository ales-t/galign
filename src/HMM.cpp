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

HMM::HMM(Corpus *corpus, float alpha, float cognateAlpha, const CountType &prevCounts,
    const JointCountType &prevJoint)
: corpus(corpus), alpha(alpha), cognateAlpha(cognateAlpha), counts(prevCounts),
  jointCounts(prevJoint)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (int i = 0; i < sentence->src.size(); i++) {
      int distortion = 1;
      if (i > 0)
        distortion = sentence->align[i] - sentence->align[i - 1];
      distortionCounts[distortion]++;
    }
  }
}

void HMM::RunIteration(bool doAggregate)
{
  vector<int> order;
  order.reserve(corpus->GetTotalSourceTokens());
  for (int i = 0; i < corpus->GetTotalSourceTokens(); i++)
    order.push_back(i);

  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  for (size_t i = 0; i < order.size(); i++) {
    int position = order[i];
    pair<int, int> sentPos = corpus->GetSentenceAndPosition(position);
    Sentence *sentence = sentences[sentPos.first];
    const string &srcWord = sentence->src[sentPos.second];
    const string &oldTgtWord = sentence->tgt[sentence->align[sentPos.second]];
    
    // discount removed alignment link
    if (--jointCounts[srcWord][oldTgtWord] <= 0) jointCounts[srcWord].erase(oldTgtWord);
    counts[oldTgtWord]--;
    int distortion = 1;
    if (sentPos.second > 0) {
      distortion = sentence->align[sentPos.second] - sentence->align[sentPos.second - 1];
      cerr << distortion << "\n";
    }
    distortionCounts[distortion] = max(0, distortionCounts[distortion] - 1);

    // generate a sample
    vector<float> distParams;
    distParams.reserve(sentence->tgt.size());
    float lexicalProbNorm = 0;
    float distortionProbNorm = 0;

    for (int i = 0; i < sentence->tgt.size(); i++) {
      const string &tgt = sentence->tgt[i];
      float pairAlpha = alpha;
      float normAlpha = alpha * corpus->GetSrcTypes().size();
      if (srcWord == tgt)
        pairAlpha = cognateAlpha;
      if (hasCognate.find(tgt) != hasCognate.end())
        normAlpha += cognateAlpha - alpha;

      float prob = (jointCounts[srcWord][tgt] + pairAlpha) / (counts[tgt] + normAlpha);
      lexicalProbNorm += prob;
      int distortion = 1;
      if (sentPos.second > 0) {
        distortion = i - sentence->align[sentPos.second - 1];
      }
      int distCount = 0;
      if (distortionCounts.find(distortion) != distortionCounts.end())
        distCount = distortionCounts[distortion];
      prob *= (distCount + alpha);
      distortionProbNorm += distCount + alpha;
    }

    // normalize
    for (int i = 0; i < sentence->tgt.size(); i++) {
      distParams[i] /= (lexicalProbNorm * distortionProbNorm);
    }

    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;
    distortion = 1;
    if (sentPos.second > 0) {
      distortion = sample - sentence->align[sentPos.second - 1];
    }
    distortionCounts[distortion]++;
    if (doAggregate) {
      aggregateJoint[srcWord][sentence->tgt[sample]]++;
      aggregateCounts[sentence->tgt[sample]]++;
      aggregateDistortion[distortion]++;
    }
  }
}

vector<AlignmentType> HMM::GetAggregateAlignment()
{
  vector<AlignmentType> out;
  BOOST_FOREACH(DistortionCountType::value_type dist, aggregateDistortion) {
    cerr << dist.first << ":\t" << dist.second << endl;
  }
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    AlignmentType aggregAlign(sentence->src.size());
    for (int i = 0; i < sentence->src.size(); i++) {
      vector<float> probs(sentence->tgt.size());
      float lexicalProbNorm = 0;
      float distortionProbNorm = 0;

      for (int j = 0; j < sentence->tgt.size(); j++) {
        float pairAlpha = alpha;
        float normAlpha = alpha * corpus->GetSrcTypes().size();
        if (sentence->src[i] == sentence->tgt[j])
          pairAlpha = cognateAlpha;
        if (hasCognate.find(sentence->tgt[j]) != hasCognate.end())
          normAlpha += cognateAlpha - alpha;

        float prob = (aggregateJoint[sentence->src[i]][sentence->tgt[j]] + pairAlpha)        
          / (aggregateCounts[sentence->tgt[j]] + normAlpha);

        lexicalProbNorm += prob;
        int distortion = 1;
        if (i > 0) {
          distortion = j - sentence->align[i - 1];
        }
        int distCount = 0;
        if (aggregateDistortion.find(distortion) != aggregateDistortion.end())
          distCount = aggregateDistortion[distortion];
        prob *= (distCount + alpha);
        distortionProbNorm += distCount + alpha;
        probs[j] = prob;
      }

      int best = -1;
      float bestProb = 0;
      // normalize
      for (int j = 0; j < sentence->tgt.size(); j++) {
        probs[j] /= (lexicalProbNorm * distortionProbNorm);
        if (probs[j] > bestProb) {
          best = j;
          bestProb = probs[j];
        }      
      }
      aggregAlign[i] = best;
    }
    out.push_back(aggregAlign);
  }
  return out;
}
