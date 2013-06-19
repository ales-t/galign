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
  bool GetMosesFormat()         { return mosesFormat; }
  size_t GetIBM1Iterations()    { return ibm1Iterations; }
  size_t GetIBM1CoolingFrom()   { return ibm1CoolingFrom; }
  size_t GetHMMIterations()     { return hmmIterations; }
  size_t GetHMMCoolingFrom()    { return hmmCoolingFrom; }
  size_t GetCores()             { return cores; }
  float GetLexicalAlpha()       { return alphaLex; }
  float GetDistortionAlpha()    { return alphaDist; }
  std::string GetOutputFile()   { return outputFile; }
  std::string GetInputFile()    { return inputFile; }

private:
  Options() {}
  Options(Options const &);
  Options &operator=(Options const &);

  size_t ibm1Iterations, ibm1CoolingFrom,
         hmmIterations, hmmCoolingFrom, cores;
  float alphaLex, alphaDist, cognateBoost;
  std::string inputFile, outputFile;
  bool help, mosesFormat;
};

#endif // OPTIONS_HPP_
