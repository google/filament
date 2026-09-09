#pragma once
#include <string>
#include <vector>

class ShaderTester {
public:
  virtual ~ShaderTester(){};

  static ShaderTester *New(const std::wstring &file);
  static ShaderTester *New(void *blob);

  virtual void setDevice(const std::wstring &namePrefix) = 0;
  virtual void runShader(int initialShaderId, const std::vector<int> &input,
                         std::vector<int> &output) = 0;
};
