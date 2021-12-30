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

#include <Encoder/include/AhapEncoder.h>
#include <Encoder/include/IvsEncoder.h>
#include <Encoder/include/PcmEncoder.h>
#include <Encoder/include/IOJson.h>
#include <Tools/include/InputParser.h>
#include <Tools/include/OHMData.h>
#include <Types/include/Haptics.h>
#include <Types/include/Perception.h>
#include <functional>
#include <filesystem>

using haptics::encoder::AhapEncoder;
using haptics::encoder::IvsEncoder;
using haptics::encoder::PcmEncoder;
using haptics::encoder::IOJson;
using haptics::tools::InputParser;
using haptics::tools::OHMData;
using haptics::types::Haptics;
using haptics::types::Perception;

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
    output = "out.impg";
  }
  std::cout << "The generated file will be : " << output << "\n";

  Haptics hapticFile;
  Perception myPerception(0, 0, std::string(), haptics::types::PerceptionModality::Other);
  std::string ext = InputParser::getFileExt(filename);
  int codeExit = -1;
  if (ext == "ohm") {
    std::cout << "The OHM file to process : " << filename << std::endl;
    OHMData ohmData;
    if (!ohmData.loadFile(filename)) {
      std::cerr << "ERROR : impossible to read the OHM file : " << std::endl << filename << std::endl;
      return EXIT_FAILURE;
    }
    hapticFile.loadMetadataFromOHM(ohmData);

    codeExit = EXIT_SUCCESS;
    if (ohmData.getHapticElementMetadataSize() != hapticFile.getPerceptionsSize()) {
      codeExit = EXIT_FAILURE;
    }
    std::filesystem::path folderPath = std::filesystem::path(filename);
    folderPath = folderPath.parent_path();
    for (int i = 0; i < ohmData.getHapticElementMetadataSize() && codeExit == EXIT_SUCCESS; i++) {
      OHMData::HapticElementMetadata metadata = ohmData.getHapticElementMetadataAt(i);

      filename = (folderPath / metadata.elementFilename).string();
      if (!std::filesystem::is_regular_file(filename)) {
        codeExit = EXIT_FAILURE;
        break;
      }

      ext = InputParser::getFileExt(filename);
      std::function<int(std::string &, Perception &)> encodingFunction =
          // NOLINTNEXTLINE(misc-unused-parameters)
          [](std::string &filename, Perception &out) { return EXIT_FAILURE; };
      if (ext == "json" || ext == "ahap") {
        std::cout << "The AHAP file to encode : " << filename << std::endl;
        encodingFunction = AhapEncoder::encode;
      } else if (ext == "xml" || ext == "ivs") {
        std::cout << "The IVS file to encode : " << filename << std::endl;
        encodingFunction = IvsEncoder::encode;
      } else if (ext == "wav") {
        std::cout << "The WAV file to encode : " << filename << std::endl;
        encodingFunction = [](std::string &filename, Perception &out) {
          const double curveFrequency = 72.5;
          return PcmEncoder::encode(filename, curveFrequency, out);
        };

        myPerception = hapticFile.getPerceptionAt(i);
        codeExit = encodingFunction(filename, myPerception);
        if (codeExit == EXIT_SUCCESS) {
          hapticFile.replacePerceptionAt(i, myPerception);
        }
      }
    }
  } else if (ext == "json" || ext == "ahap") {
    std::cout << "The AHAP file to encode : " << filename << std::endl;
    codeExit = AhapEncoder::encode(filename, myPerception);
    hapticFile.addPerception(myPerception);
  } else if (ext == "xml" || ext == "ivs") {
    std::cout << "The IVS file to encode : " << filename << std::endl;
    codeExit = IvsEncoder::encode(filename, myPerception);
    hapticFile.addPerception(myPerception);
  } else if (ext == "wav") {
    std::cout << "The WAV file to encode : " << filename << std::endl;
    const double curveFrequency = 72.5;
    codeExit = PcmEncoder::encode(filename, curveFrequency, myPerception);
    hapticFile.addPerception(myPerception);
  }
  else {
    codeExit = EXIT_FAILURE;
  }

  if (codeExit == EXIT_FAILURE) {
    InputParser::help(args[0]);
    return codeExit;
  }

  IOJson::writeFile(hapticFile, output);
  return codeExit;
}
