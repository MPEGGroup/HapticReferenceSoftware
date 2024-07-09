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

#include <IOHaptics/include/IOBinary.h>
#include <IOHaptics/include/IOCompatibility.h>
#include <IOHaptics/include/IOJson.h>
#include <IOHaptics/include/IOStream.h>
#include <Tools/include/InputParser.h>
#include <Tools/include/OHMData.h>
#include <Types/include/Haptics.h>
#include <Types/include/Perception.h>
#include <filesystem>
#include <functional>
#include <optional>

using haptics::io::IOBinary;
using haptics::io::IOJson;
using haptics::io::IOStream;
using haptics::tools::InputParser;
using haptics::tools::OHMData;
using haptics::types::Haptics;
using haptics::types::Perception;

auto help() -> void {
  std::cout
      << "usages: Conformance [-h] -f <FILE>" << std::endl
      << std::endl
      << "This piece of software checks the conformance of an input file based on ISO/IEC 23090-31"
      << std::endl
      << "positional arguments:" << std::endl
      << "\t-f, --file <FILE>\t\tfile to convert" << std::endl
      << std::endl
      << "optional arguments:" << std::endl
      << "\t-h, --help\t\t\tshow this help message and exit" << std::endl
      << std::endl
      << "\t-c, --comparison\t\t\tthe input file will be compared to the file provided for "
         "comparison."
      << std::endl;
}

// NOLINTNEXTLINE
auto main(int argc, char *argv[]) -> int {
  const auto args = std::vector<const char *>(argv, argv + argc);
  InputParser inputParser(args);
  if (inputParser.cmdOptionExists("-h") || inputParser.cmdOptionExists("--help")) {
    help();
    return EXIT_SUCCESS;
  }

  std::string filename = inputParser.getCmdOption("-f");
  if (filename.empty()) {
    filename = inputParser.getCmdOption("--file");
  }
  if (filename.empty() || !std::filesystem::is_regular_file(filename)) {
    help();
    return EXIT_FAILURE;
  }

  std::string comparisonFilename = inputParser.getCmdOption("-c");
  if (comparisonFilename.empty()) {
    comparisonFilename = inputParser.getCmdOption("--comparison");
  }

  Haptics hapticFile;
  Perception myPerception(0, 0, std::string(), haptics::types::PerceptionModality::Other);
  std::string ext = InputParser::getFileExt(filename);
  int codeExit = -1;
  if (ext == "hjif") {
    std::cout << "The HJIF file to check: " << filename << std::endl;
    IOJson::loadFile(filename, hapticFile);
    auto logs = haptics::io::IOCompatibility::checkHaptics(hapticFile);
    if (!logs.empty()) {
      for (auto &l : logs) {
        std::cerr << l << std::endl;
      }
    }
    codeExit = EXIT_SUCCESS;
  } else if (ext == "hmpg") {
    std::cout << "The HMPG file to check: " << filename << std::endl;
    IOStream::readFile(filename, hapticFile);
    auto logs = haptics::io::IOCompatibility::checkHaptics(hapticFile);
    if (!logs.empty()) {
      for (auto &l : logs) {
        std::cerr << l << std::endl;
      }
    }
    codeExit = EXIT_SUCCESS;
  } else {
    codeExit = EXIT_FAILURE;
  }
  if (!comparisonFilename.empty()) {
    Haptics comparisonHapticFile;
    std::string extComp = InputParser::getFileExt(comparisonFilename);
    int codeExit = -1;
    if (extComp == "hjif") {
      std::cout << "The HJIF file used for comparison: " << comparisonFilename << std::endl;
      IOJson::loadFile(comparisonFilename, comparisonHapticFile);
      codeExit = EXIT_SUCCESS;
    } else if (extComp == "hmpg") {
      std::cout << "The HMPG file used for comparison: " << comparisonFilename << std::endl;
      IOStream::readFile(comparisonFilename, comparisonHapticFile);
      codeExit = EXIT_SUCCESS;
    } else {
      codeExit = EXIT_FAILURE;
    }
    if (codeExit == EXIT_SUCCESS) {
      bool equals = hapticFile.equals(comparisonHapticFile);
      if (equals) {
        std::cerr << filename << " and " << comparisonFilename << " contain the same data."
                  << std::endl;
      } else {
        std::cerr << filename << " and " << comparisonFilename << " do not contain the same data."
                  << std::endl;
      }
    }
  }
  if (codeExit == EXIT_FAILURE) {
    help();
    return codeExit;
  }
  return codeExit;
}