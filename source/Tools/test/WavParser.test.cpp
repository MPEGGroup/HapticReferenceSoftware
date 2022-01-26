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

#include <Tools/include/WavParser.h>
#include <filesystem>
#include <vector>

constexpr int fs = 8000;

auto checkCorrect(std::vector<double> buffer, std::vector<double> buffer_rec) -> bool {
  bool correct = true;
  for (size_t i = 0; i < buffer.size(); i++) {
    if (fabs(buffer[i]) >= 1) {
      if (buffer[i] >= 1) {
        // NOLINTNEXTLINE
        if (!((buffer_rec[i] <= 1) && (buffer_rec[i] > 0.999))) {
          std::cout << "clamping not correct, values: " << buffer[i] << ", " << buffer_rec[i]
                    << std::endl;
          correct = false;
        }
      } else if (fabs(buffer_rec[i]) != 1) {

        std::cout << "clamping not correct, values: " << buffer[i] << ", " << buffer_rec[i]
                  << std::endl;
        correct = false;
      }
    } else {
      if (buffer[i] != buffer_rec[i]) {
        std::cout << "value not correct, values: " << buffer[i] << ", " << buffer_rec[i]
                  << std::endl;
        correct = false;
      }
    }
  }
  return correct;
}

TEST_CASE("haptics::tools::WavParser") {

  using haptics::tools::WavParser;

  SECTION("Output to Input test") {

    std::string filename = "test.wav";
    std::vector<double> buffer{0, 1, 0, 2};
    WavParser::saveFile(filename, buffer, fs);
    CHECK(std::filesystem::is_regular_file(filename));
    WavParser wavParser2;
    wavParser2.loadFile(filename);
    std::vector<double> buffer2 = wavParser2.getSamplesChannel(0);
    CHECK(buffer2.size() == buffer.size());
  }

  SECTION("Output to Input test MD") {

    std::string filename = "test_MD.wav";
    std::vector<double> buffer{0, 0.5, 0.75, 1};  // NOLINT
    std::vector<double> buffer2{1, 0.75, 0.5, 0}; // NOLINT
    std::vector<std::vector<double>> buffer_MD;
    buffer_MD.push_back(buffer);
    buffer_MD.push_back(buffer2);
    WavParser::saveFile(filename, buffer_MD, fs);
    CHECK(std::filesystem::is_regular_file(filename));
    WavParser wavParser2;
    wavParser2.loadFile(filename);
    std::vector<double> buffer_rec = wavParser2.getSamplesChannel(0);
    std::vector<double> buffer_rec2 = wavParser2.getSamplesChannel(1);
  }
}

TEST_CASE("haptics::tools::WavParser overflow") {

  using haptics::tools::SCALING;
  using haptics::tools::WavParser;

  SECTION("Overflow test") {

    std::string filename = "test_overflow.wav";
    std::vector<double> buffer{0, 0.5, 0.75, 1, 1.25};      // NOLINT
    std::vector<double> buffer2{-1, -0.75, -0.5, 0, -1.25}; // NOLINT
    std::vector<std::vector<double>> buffer_MD;
    buffer_MD.push_back(buffer);
    buffer_MD.push_back(buffer2);
    WavParser::saveFile(filename, buffer_MD, fs);
    CHECK(std::filesystem::is_regular_file(filename));
    WavParser wavParser2;
    wavParser2.loadFile(filename);
    std::vector<double> buffer_rec = wavParser2.getSamplesChannel(0);
    std::vector<double> buffer_rec2 = wavParser2.getSamplesChannel(1);
    bool correct = true;
    std::cout << "channel 1" << std::endl;
    correct = checkCorrect(buffer, buffer_rec);
    CHECK(correct);
    std::cout << "channel 2" << std::endl;
    correct = checkCorrect(buffer2, buffer_rec2);
    CHECK(correct);
  }

  SECTION("Overflow test 1D") {

    std::string filename = "test_overflow_1D.wav";
    std::vector<double> buffer{0, 0.5, -1, 1, 1.25}; // NOLINT
    WavParser::saveFile(filename, buffer, fs);
    CHECK(std::filesystem::is_regular_file(filename));
    WavParser wavParser2;
    wavParser2.loadFile(filename);
    std::vector<double> buffer_rec = wavParser2.getSamplesChannel(0);
    bool correct = true;
    correct = checkCorrect(buffer, buffer_rec);
    CHECK(correct);
  }
}
