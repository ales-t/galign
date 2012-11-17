#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>
#include <vector>

class Options
{
public:
  static Options &Instance()
  {
    static Options instance;
    return instance;
  }
  void ParseOptions(int argc, char **argv);

  bool GetHelp()                { return help; }
  bool GetCompress()            { return compress; }
  int GetIterations()           { return iterations; }
  int GetAggregateAfter()       { return aggregateAfter; }
  float GetAlpha()              { return alpha; }
  float GetCognateAlpha()       { return alpha * cognateBoost; }
  std::string GetOutputPrefix() { return outputPrefix; }
  std::string GetInputFile()    { return inputFile; }

private:
  Options() {}
  Options(Options const &);
  Options &operator=(Options const &);

  int iterations, aggregateAfter;
  float alpha, cognateBoost;
  std::string outputPrefix, inputFile;
  bool compress, help;
};

#endif // OPTIONS_HPP_
