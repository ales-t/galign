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
  int GetAggregateFrom()        { return aggregateFrom; }
  float GetLexicalAlpha()       { return alphaLex; }
  float GetDistortionAlpha()    { return alphaDist; }
  float GetCognateAlpha()       { return alphaLex * cognateBoost; }
  std::string GetOutputPrefix() { return outputPrefix; }
  std::string GetInputFile()    { return inputFile; }

private:
  Options() {}
  Options(Options const &);
  Options &operator=(Options const &);

  int iterations, aggregateFrom;
  float alphaLex, alphaDist, cognateBoost;
  std::string outputPrefix, inputFile;
  bool compress, help;
};

#endif // OPTIONS_HPP_
