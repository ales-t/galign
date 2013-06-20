#include "HMM.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace boost::random;

HMM::HMM(Corpus *corpus, float alpha, float distAlpha, const CountType &prevCounts,
    const JointCountType &prevJoint)
: corpus(corpus), alpha(alpha), distAlpha(distAlpha), counts(prevCounts),
  jointCounts(prevJoint)
{
  distortionCounts.resize(2*BUCKET_LIMIT + 1);
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (size_t i = 0; i < sentence->src.size(); i++) {
      UpdateTransition(sentence, i, +1);
    }
  }
  order = corpus->GetTokensToSentences();
}

void HMM::RunIteration(double temp)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());

  // over all words in corpus (in random order)
  #pragma omp parallel for
  for (size_t posIt = 0; posIt < order.size(); posIt++) {
    pair<size_t, size_t> sentPos = order[posIt];
    Sentence *sentence = sentences[sentPos.first];
    size_t srcPos = sentPos.second;
    size_t srcWord = sentence->src[srcPos];
    size_t oldTgtWord = sentence->tgt[sentence->align[srcPos]];
    
    // discount removed alignment link
    if (--jointCounts[srcWord][oldTgtWord] <= 0) jointCounts[srcWord].Erase(oldTgtWord);
    counts[oldTgtWord]--;
    UpdateTransition(sentence, srcPos, -1);

    // generate a sample
    vector<float> distParams = GetDistribution(sentence, srcPos);
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[srcPos] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;

    UpdateTransition(sentence, srcPos, +1);
  }
}

vector<int> HMM::GetTransitions(const Sentence *sentence, size_t srcPos)
{
  vector<int> transitions;
  int prev = srcPos - 1;
  int next = srcPos + 1;
  while (prev >= 0 && sentence->align[prev] == 0) prev--;
  while (next < (int)sentence->src.size() && sentence->align[next] == 0) next++;
  if (sentence->align[srcPos] == 0) {
    if (prev == -1 || next == (int)sentence->src.size())
      transitions.push_back(1);
    else
      transitions.push_back(sentence->align[next] - sentence->align[prev]);
  } else {
    if (prev == -1)
      transitions.push_back(1);
    else 
      transitions.push_back(sentence->align[srcPos] - sentence->align[prev]);

    if (next == (int)sentence->src.size())
      transitions.push_back(1);
    else
      transitions.push_back(sentence->align[next] - sentence->align[srcPos]);
  }
  return transitions;
}

void HMM::UpdateTransition(const Sentence *sentence, size_t srcPos, int diff)
{
  vector<int> transitions = GetTransitions(sentence, srcPos);
  BOOST_FOREACH(int t, transitions) {
    distortionCounts[GetBucket(t)] += diff;
  }
}

std::vector<float> HMM::GetDistribution(Sentence *sentence, size_t srcPosition)
{
  LogDistribution dist;

  float nullProb = 1 / (float)sentence->tgt.size();
  for (size_t tgtPosition = 0; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    dist.Add(log(jointCounts[sentence->src[srcPosition]][sentence->tgt[tgtPosition]] + alpha)
      - log(counts[sentence->tgt[tgtPosition]] + alpha * corpus->GetTotalSourceTokens()));
  }

  LogDistribution transitionDist; 
  for (size_t tgtPosition = 1; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    sentence->align[srcPosition] = tgtPosition; // so that GetTransitions() is correct
    vector<int> transitions = GetTransitions(sentence, srcPosition);
    float logProb = 0;
    BOOST_FOREACH(int t, transitions) {
      logProb += log(distortionCounts[GetBucket(t)] + distAlpha);
    }
    transitionDist.Add(logProb);
  }
  transitionDist.Normalize();

  for (size_t tgtPosition = 0; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    if (tgtPosition == 0) {
      dist[tgtPosition] += log(nullProb);
    } else {
      dist[tgtPosition] += log(1 - nullProb) + transitionDist[tgtPosition - 1];
    }
  }

  dist.Normalize();
  return dist.Exp();
}
