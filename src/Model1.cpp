#include "Model1.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>
#include <cmath>

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
      jointCounts[sentence->src[i]][sentence->tgt[tgtWord]]++;
      sentence->align.push_back(tgtWord);
    }
    for (size_t i = 0; i < sentence->tgt.size(); i++)
      counts[sentence->tgt[i]]++;
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
      float logProb = log(jointCounts[srcWord][tgt] + alpha)
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
