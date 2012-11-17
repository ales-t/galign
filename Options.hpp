#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>
#include <vector>

class Options
{
public:
  static Options* Instance()
  {
    if (! instance) instance = new Options();
  }
  void ParseOptions(int argc, char **argv);

  int GetIterations()           { return iterations; }
  int GetAggregateAfter()       { return aggregateAfter; }
  float GetAlpha()              { return alpha; }
  float GetCognateAlpha()       { return alpha * cognateBoost; }
  std::string GetOutputPrefix() { return outputPrefix; }

private:
  Options() {}
  Options(Options const &) {}
  Options &operator=(Options const &) {}

  static Options *instance;
  int iterations, aggregateAfter;
  float alpha, cognateBoost;
  std::string outputPrefix;
};

#endif // OPTIONS_HPP_
