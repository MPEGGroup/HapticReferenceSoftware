/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2021, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <catch2/catch.hpp>

#include "../include/InputParser.h"

using haptics::tools::InputParser;

TEST_CASE("haptics::tools") {

  SECTION("Command line arguments") {

    std::vector<const char *> fake_argv = {"fake_prg", "-f", "input_file", "-o", "output_file"};

    InputParser inputParser(fake_argv);
    CHECK(inputParser.cmdOptionExists("-f"));
    CHECK(inputParser.cmdOptionExists("-o"));
    CHECK_FALSE(inputParser.cmdOptionExists("-q"));
  }
}

TEST_CASE("InputParser::getFileExt") {
  const std::vector<std::vector<const char*>> testingValues = {
    {"filename.ext", "ext"},
    {"path/filename.json", "json"},
    {"very/long/path/filename.xml", "xml"},
    {"../../other/folder/filename.h", "h"},
    {"multiple.test.cpp", "cpp"},
    {"no/ext", ""}
  };

  for (std::vector<const char*> v : testingValues) {
    REQUIRE(v.size() == 2);
    
    DYNAMIC_SECTION("Test getFileExtension in every cases") {
      std::string filename = std::string(v[0]);
      std::string res = InputParser::getFileExt(filename);
      std::string expectedResult = std::string(v[1]);

      CHECK_THAT(res, Catch::Matchers::Equals(expectedResult));
    }
  }
}
