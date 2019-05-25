// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace oidn {

  // Command-line argument parser
  class ArgParser
  {
  private:
    int argc;
    char** argv;
    int pos;

  public:
    ArgParser(int argc, char* argv[])
      : argc(argc), argv(argv),
        pos(1)
    {}

    bool hasNext() const
    {
      return pos < argc;
    }

    std::string getNext()
    {
      if (pos < argc)
        return argv[pos++];
      else
        throw std::invalid_argument("argument expected");
    }

    std::string getNextOpt()
    {
      std::string str = getNext();
      if (str.empty() || str[0] != '-')
        throw std::invalid_argument("option expected");
      return str.substr(str.find_first_not_of("-"));
    }

    std::string getNextValue()
    {
      std::string str = getNext();
      if (!str.empty() && str[0] == '-')
        throw std::invalid_argument("value expected");
      return str;
    }

    int getNextValueInt()
    {
      std::string str = getNextValue();
      return atoi(str.c_str());
    }
  };

} // namespace oidn

