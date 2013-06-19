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
      int distortion = 1;
      // TODO spravna inicializace
      if (i > 0)
        distortion = sentence->align[i] - sentence->align[i - 1];
      distortionCounts[GetBucket(distortion)]++;
    }
  }
  order = corpus->GetTokensToSentences();
}

void HMM::RunIteration(double temp)
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  random_shuffle(order.begin(), order.end());
  cerr << counts[0] << endl;

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
    UpdateTransition(distortionCounts, sentence, srcPos, -1);

    // generate a sample
    vector<float> distParams = GetDistribution(sentence, srcPos, counts, 
        jointCounts, distortionCounts);
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[srcPos] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;

    UpdateTransition(distortionCounts, sentence, srcPos, +1);
  }
}

void HMM::UpdateTransition(DistortionCountType &distCounts, const Sentence *sentence, size_t srcPos, int diff)
{
  int prev = srcPos - 1;
  int next = srcPos + 1;
  while (prev >= 0 && sentence->align[prev] == 0) prev--;
  while (next < (int)sentence->src.size() && sentence->align[next] == 0) next++;
  if (sentence->align[srcPos] == 0) {
    if (prev == -1 || next == (int)sentence->src.size())
      distCounts[GetBucket(1)] += diff;
    else {
      distCounts[GetBucket(sentence->align[next] - sentence->align[prev])] += diff;
    }
  } else {
    if (prev == -1)
      distCounts[GetBucket(1)] += diff;
    else 
      distCounts[GetBucket(sentence->align[srcPos] - sentence->align[prev])] += diff;

    if (next == (int)sentence->src.size())
      distCounts[GetBucket(1)] += diff;
    else
      distCounts[GetBucket(sentence->align[next] - sentence->align[srcPos])] += diff;
  }
}

std::vector<float> HMM::GetDistribution(Sentence *sentence, size_t srcPosition, CountType &a_counts,
    JointCountType &a_jointCounts, DistortionCountType &a_distCounts)
{
  LogDistribution lexicalProbs;
  LogDistribution inDistortionPotentials;
  LogDistribution outDistortionPotentials;

  float nullProb = 1 / (float)sentence->tgt.size();
  for (size_t tgtPosition = 0; tgtPosition < sentence->tgt.size(); tgtPosition++) {
    float logLexProb = log(a_jointCounts[sentence->src[srcPosition]][sentence->tgt[tgtPosition]] + alpha)
      - log(a_counts[sentence->tgt[tgtPosition]] + alpha * corpus->GetTotalSourceTokens());
    lexicalProbs.Add(logLexProb);

    if (tgtPosition == 0) {
      inDistortionPotentials.Add(log(nullProb));
      outDistortionPotentials.Add(0);
    } else {
      int inDist = (srcPosition > 0) ? tgtPosition - sentence->align[srcPosition - 1] : 1;
      int outDist = (srcPosition < sentence->src.size() - 1)
        ? sentence->align[srcPosition + 1] - tgtPosition : 1;
      size_t inDistBucket = GetBucket(inDist);
      inDistortionPotentials.Add(log(1 - nullProb) + log(a_distCounts[inDistBucket] + distAlpha));
      size_t outDistBucket = GetBucket(outDist);
      outDistortionPotentials.Add(log(a_distCounts[outDistBucket] + distAlpha));
    }
  }

  assert(lexicalProbs.GetSize() == inDistortionPotentials.GetSize());
  assert(inDistortionPotentials.GetSize() == outDistortionPotentials.GetSize());
  lexicalProbs.Normalize();
  inDistortionPotentials.Normalize();
  outDistortionPotentials.Normalize();
  vector<float> out(sentence->tgt.size());
  for (size_t i = 0; i < sentence->tgt.size(); i++) {
    out[i] = exp(lexicalProbs[i] + inDistortionPotentials[i] + outDistortionPotentials[i]);
  }
  return out;
}
