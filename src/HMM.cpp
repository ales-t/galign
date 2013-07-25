#include "HMM.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace boost::random;

HMM::HMM(Corpus *corpus, float alpha, float nullProb)
: corpus(corpus), alpha(alpha), nullProb(nullProb)
{
  distortionCounts.resize(2*BUCKET_LIMIT + 1);
  order = corpus->GetTokensToSentences();
  counts.resize(corpus->GetTotalTargetTypes());
  jointCounts.resize(corpus->GetTotalSourceTypes());
}

void HMM::UpdateFromCorpus()
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (size_t i = 0; i < sentence->src.size(); i++) {
      size_t tgtWord = sentence->tgt[sentence->align[i]];
      counts[tgtWord]++;
      jointCounts[sentence->src[i]][tgtWord]++;
      UpdateTransition(sentence, i, +1);
    }
  }
}

void HMM::RunIteration(float temp)
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
    Annealer::Anneal(temp, distParams.begin(), distParams.end());
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

  for (size_t tgtPosition = 0; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    const CountHashType &jointCountsWord = jointCounts[sentence->src[srcPosition]];
    dist.Add(log(jointCountsWord[sentence->tgt[tgtPosition]] + alpha)
      - log(counts[sentence->tgt[tgtPosition]] + alpha * corpus->GetTotalSourceTypes()));
  }

  LogDistribution transitionDist; 
  for (size_t tgtPosition = 1; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    sentence->align[srcPosition] = tgtPosition; // so that GetTransitions() is correct
    vector<int> transitions = GetTransitions(sentence, srcPosition);
    float logProb = 0;
    BOOST_FOREACH(int t, transitions) {
      logProb += log((float)distortionCounts[GetBucket(t)]);
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
