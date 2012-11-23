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
  aggregateCounts.resize(corpus->GetTotalTargetTypes());
  aggregateJoint.resize(corpus->GetTotalSourceTypes());
  distortionCounts.resize(2*BUCKET_LIMIT + 1);
  aggregateDistortion.resize(2*BUCKET_LIMIT + 1);
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (size_t i = 0; i < sentence->src.size(); i++) {
      int distortion = 1;
      if (i > 0)
        distortion = sentence->align[i] - sentence->align[i - 1];
      distortionCounts[GetBucket(distortion)]++;
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
    if (--jointCounts[srcWord][oldTgtWord] <= 0) jointCounts[srcWord].Erase(oldTgtWord);
    counts[oldTgtWord]--;
    size_t inDistBucket = GetBucket(GetInputDistortion(sentence, sentPos.second,
          sentence->align[sentPos.second]));
    size_t outDistBucket = GetBucket(GetOutputDistortion(sentence, sentPos.second,
          sentence->align[sentPos.second]));
    distortionCounts[inDistBucket] = max(0, distortionCounts[inDistBucket] - 1);
    distortionCounts[outDistBucket] = max(0, distortionCounts[outDistBucket] - 1);

    // generate a sample
    vector<float> distParams = GetDistribution(sentence, sentPos.second, counts, 
        jointCounts, distortionCounts);
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;
    inDistBucket = GetBucket(GetInputDistortion(sentence, sentPos.second, sample));
    outDistBucket = GetBucket(GetOutputDistortion(sentence, sentPos.second, sample));
    distortionCounts[inDistBucket]++;
    distortionCounts[outDistBucket]++;
    if (doAggregate) {
      aggregateJoint[srcWord][sentence->tgt[sample]]++;
      aggregateCounts[sentence->tgt[sample]]++;
      aggregateDistortion[inDistBucket]++;
      aggregateDistortion[outDistBucket]++;
    }
  }
}

vector<AlignmentType> HMM::GetAggregateAlignment()
{
  vector<AlignmentType> out;
  int lineNum = 0;
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    lineNum++;
    AlignmentType aggregAlign(sentence->src.size());
    for (size_t i = 0; i < sentence->src.size(); i++) {
      vector<float> dist = GetDistribution(sentence, i, aggregateCounts,
          aggregateJoint, aggregateDistortion);
      int best = -1;
      float bestProb = 0;
      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        if (dist[j] > bestProb) {
          best = j;
          bestProb = dist[j];
        }      
      }
      if (best == -1) {
        Die("Zero probability for word '" + corpus->GetSrcWord(sentence->src[i]) +
            "' in sentence " + lexical_cast<string>(lineNum));
      }
      aggregAlign[i] = best;
    }
    out.push_back(aggregAlign);
  }
  return out;
}

std::vector<float> HMM::GetDistribution(Sentence *sentence, size_t srcPosition, CountType &a_counts,
    JointCountType &a_jointCounts, DistortionCountType &a_distCounts)
{
  LogDistribution lexicalProbs;
  LogDistribution inDistortionPotentials;
  LogDistribution outDistortionPotentials;

  for (size_t tgtPosition = 0; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    float pairAlpha = alpha;
    float normAlpha = alpha * corpus->GetTotalSourceTokens();
    if (sentence->src[srcPosition] == sentence->tgt[tgtPosition])
      pairAlpha = cognateAlpha;
    if (corpus->HasCognate(sentence->tgt[tgtPosition]))
      normAlpha += cognateAlpha - alpha;

    float logLexProb = log(a_jointCounts[sentence->src[srcPosition]][sentence->tgt[tgtPosition]] + pairAlpha)
      - log(a_counts[sentence->tgt[tgtPosition]] + normAlpha);
    lexicalProbs.Add(logLexProb);

    size_t inDistBucket = GetBucket(GetInputDistortion(sentence, srcPosition, tgtPosition));
    size_t outDistBucket = GetBucket(GetOutputDistortion(sentence, srcPosition, tgtPosition));
    inDistortionPotentials.Add(log(a_distCounts[inDistBucket] + distAlpha));
    outDistortionPotentials.Add(log(a_distCounts[outDistBucket] + distAlpha));
  }

  lexicalProbs.Normalize();
  inDistortionPotentials.Normalize();
  outDistortionPotentials.Normalize();
  vector<float> out(sentence->tgt.size());
  for (size_t i = 0; i < sentence->tgt.size(); i++) {
    out[i] = exp(lexicalProbs[i] + inDistortionPotentials[i] + outDistortionPotentials[i]);
  }
  return out;
}
