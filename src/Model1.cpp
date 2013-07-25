#include "Model1.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>
#include <cmath>

using namespace std;
using namespace boost;
using namespace boost::random;

void Model1::UpdateFromCorpus()
{
  vector<Sentence *> &sentences = corpus->GetSentences();
  BOOST_FOREACH(Sentence *sentence, sentences) {
    for (size_t i = 0; i < sentence->src.size(); i++) {
      size_t tgtWord = sentence->tgt[sentence->align[i]];
      counts[tgtWord]++;
      jointCounts[sentence->src[i]][tgtWord]++;
    }
  }
}

void Model1::RunIteration(float temp)
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

    // calculate distribution parameters
    LogDistribution lexicalProbs;
    BOOST_FOREACH(size_t tgt, sentence->tgt) {
      const CountHashType &jointCountsWord = jointCounts[srcWord];
      float logProb = log(jointCountsWord[tgt] + alpha)
        - log(counts[tgt] + alpha * corpus->GetTotalSourceTypes());
      lexicalProbs.Add(logProb);
    }
    lexicalProbs.Normalize();
    vector<float> distParams = lexicalProbs.Exp();
    Annealer::Anneal(temp, distParams.begin(), distParams.end());

    // generate a sample
    discrete_distribution<int> dist(distParams.begin(), distParams.end());
    int sample = dist(generator);

    // update counts with new alignment link
    sentence->align[sentPos.second] = sample;
    jointCounts[srcWord][sentence->tgt[sample]]++;
    counts[sentence->tgt[sample]]++;
  }
}
