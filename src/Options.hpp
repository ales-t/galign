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
  size_t GetIBM1Iterations()    { return ibm1Iterations; }
  size_t GetIBM1AggregateFrom() { return ibm1AggregateFrom; }
  size_t GetHMMIterations()     { return hmmIterations; }
  size_t GetHMMAggregateFrom()  { return hmmAggregateFrom; }
  size_t GetCores()             { return cores; }
  float GetLexicalAlpha()       { return alphaLex; }
  float GetDistortionAlpha()    { return alphaDist; }
  float GetCognateAlpha()       { return alphaLex * cognateBoost; }
  std::string GetOutputPrefix() { return outputPrefix; }
  std::string GetInputFile()    { return inputFile; }

private:
  Options() {}
  Options(Options const &);
  Options &operator=(Options const &);

  size_t ibm1Iterations, ibm1AggregateFrom,
         hmmIterations, hmmAggregateFrom, cores;
  float alphaLex, alphaDist, cognateBoost;
  std::string outputPrefix, inputFile;
  bool compress, help;
};

#endif // OPTIONS_HPP_
