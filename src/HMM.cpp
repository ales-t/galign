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
}

void HMM::RunIteration(bool doAggregate)
{
  vector<int> order;
  order.reserve(corpus->GetTotalSourceTokens());
  for (size_t i = 0; i < corpus->GetTotalSourceTokens(); i++)
    order.push_back(i);

  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  for (size_t posIdx = 0; posIdx < order.size(); posIdx++) {
    int position = order[posIdx];
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
    }
    distortionCounts[distortion] = max(0, distortionCounts[distortion] - 1);

    // generate a sample
    LogDistribution lexicalProbs;
    LogDistribution distortionPotentials;

    for (size_t i = 0; i < sentence->tgt.size(); i++) {
      const string &tgt = sentence->tgt[i];
      float pairAlpha = alpha;
      float normAlpha = alpha * corpus->GetSrcTypes().size();
      if (srcWord == tgt)
        pairAlpha = cognateAlpha;
      if (corpus->HasCognate(tgt))
        normAlpha += cognateAlpha - alpha;

      float logLexProb = log(jointCounts[srcWord][tgt] + pairAlpha) - log(counts[tgt] + normAlpha);
      lexicalProbs.Add(logLexProb);
      int distortion = 1;
      if (sentPos.second > 0) {
        distortion = i - sentence->align[sentPos.second - 1];
      }
      int distCount = 0;
      if (distortionCounts.find(distortion) != distortionCounts.end())
        distCount = distortionCounts[distortion];
      distortionPotentials.Add(log(distCount + distAlpha)
          - log(corpus->GetTotalSourceTokens() + sentence->tgt.size()*distAlpha));
    }

    // get distribution parameters
    vector<float> distParams;
    distParams.reserve(sentence->tgt.size());
    lexicalProbs.Normalize();
    distortionPotentials.Normalize();
    for (size_t i = 0; i < sentence->tgt.size(); i++) {
      distParams.push_back(exp(lexicalProbs[i] + distortionPotentials[i]));
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
  int lineNum = 0;
  BOOST_FOREACH(Sentence *sentence, corpus->GetSentences()) {
    lineNum++;
    AlignmentType aggregAlign(sentence->src.size());
    for (size_t i = 0; i < sentence->src.size(); i++) {
      LogDistribution lexicalProbs;
      LogDistribution distortionPotentials;

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

        int distortion = 1;
        if (i > 0) {
          distortion = j - sentence->align[i - 1];
        }
        int distCount = 0;
        if (aggregateDistortion.find(distortion) != aggregateDistortion.end())
          distCount = aggregateDistortion[distortion];
        distortionPotentials.Add(log(distCount + distAlpha)
          - log(corpus->GetTotalSourceTokens() + sentence->tgt.size()*distAlpha));
      }

      lexicalProbs.Normalize();
      distortionPotentials.Normalize();
      int best = -1;
      float bestProb = 0;
      for (size_t j = 0; j < sentence->tgt.size(); j++) {
        float prob = (exp(lexicalProbs[j] + distortionPotentials[j]));
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
