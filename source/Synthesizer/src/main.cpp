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

#include <Synthesizer/include/Helper.h>
#include <Tools/include/InputParser.h>
#include <Tools/include/OHMData.h>
#include <Types/include/Haptics.h>
#include <IOHaptics/include/IOJson.h>
#include <filesystem>

using haptics::io::IOJson;
using haptics::synthesizer::Helper;
using haptics::tools::InputParser;
using haptics::types::Haptics;

const int DEFAULT_FS = 8000;

// NOLINTNEXTLINE(bugprone-exception-escape)
auto main(int argc, char *argv[]) -> int {
  const auto args = std::vector<const char *>(argv, argv + argc);
  InputParser inputParser(args);
  if (inputParser.cmdOptionExists("-h") || inputParser.cmdOptionExists("--help")) {
    InputParser::help(args[0]);
    return EXIT_SUCCESS;
  }

  std::string filename = inputParser.getCmdOption("-f");
  if (filename.empty()) {
    filename = inputParser.getCmdOption("--file");
  }
  if (filename.empty() || !std::filesystem::is_regular_file(filename)) {
    InputParser::help(args[0]);
    return EXIT_FAILURE;
  }

  std::string output = inputParser.getCmdOption("-o");
  if (output.empty()) {
    output = inputParser.getCmdOption("--output");
  }
  if (output.empty()) {
    output = "out.wav";
  }
  std::cout << "The generated file will be : " << output << "\n";

  std::string fsStr = inputParser.getCmdOption("-fs");
  if (fsStr.empty()) {
    fsStr = inputParser.getCmdOption("--sampling_frequency");
  }
  int fs = 0;
  if (fsStr.empty()) {
    fs = DEFAULT_FS;
  } else {
    fs = std::stoi(fsStr);
    if (fs <= 0) {
      InputParser::help(args[0]);
      return EXIT_FAILURE;
    }
  }

  std::cout << "The sampling frequency used will be : " << fs << "\n";

  std::string padStr = inputParser.getCmdOption("--pad");
  int pad = 0;
  if (!padStr.empty()) {
    pad = std::max(std::stoi(padStr), 0);
    std::cout << "The padding used will be : " << pad << "ms\n";
  }

  Haptics hapticFile = IOJson::loadFile(filename);
  const double timeLength = Helper::getTimeLength(hapticFile);

  if (!Helper::playFile(hapticFile, timeLength, fs, pad, output)) {
    return EXIT_FAILURE;
  }

  if (inputParser.cmdOptionExists("--generate_ohm")) {
    std::filesystem::path p(output);
    std::string str = p.filename().u8string();
    haptics::tools::OHMData ohm = hapticFile.extractMetadataToOHM(str);
    std::string ohmPath = output.substr(0, filename.find_last_of('.')) + ".ohm";
    ohm.writeFile(ohmPath);
  }

  return EXIT_SUCCESS;
}
