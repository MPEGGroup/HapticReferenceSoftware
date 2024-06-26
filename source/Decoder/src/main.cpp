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
#include <IOHaptics/include/IOJson.h>
#include <IOHaptics/include/IOStream.h>
#include <Tools/include/InputParser.h>
#include <Types/include/Haptics.h>

using haptics::io::IOBinary;
using haptics::io::IOJson;
using haptics::io::IOStream;
using haptics::tools::InputParser;
using haptics::types::Haptics;

void help() {
  std::cout << "usages: Decoder [-h] -f <FILE> -o <OUTPUT_FILE>" << std::endl
            << std::endl
            << "This piece of software converts MPEG Haptics binary encoded RM1 files into their "
               "human-readable format"
            << std::endl
            << "positional arguments:" << std::endl
            << "\t-f, --file <FILE>\t\tfile to convert" << std::endl
            << "\t-o, --output <OUTPUT_FILE>\toutput file" << std::endl
            << std::endl
            << "optional arguments:" << std::endl
            << "\t-h, --help\t\t\tshow this help message and exit" << std::endl;
  //<< "\t-s, --streaming\t\t\t use the decoder for the binary streaming format. Output a "
  //   "file in a human-readable format"
  //<< std::endl;
}

// NOLINTNEXTLINE(bugprone-exception-escape)
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
  if (filename.empty()) {
    help();
    return EXIT_FAILURE;
  }

  std::cout << "The file to process is : " << filename << "\n";
  std::string output = inputParser.getCmdOption("-o");
  if (output.empty()) {
    output = inputParser.getCmdOption("--output");
  }
  if (output.empty()) {
    help();
    return EXIT_FAILURE;
  }

  Haptics hapticFile;
  // if (inputParser.cmdOptionExists("-s") || inputParser.cmdOptionExists("--streaming")) {

  if (!IOStream::readFile(filename, hapticFile)) {
    return EXIT_FAILURE;
  }
  //} else {
  //  if (!IOBinary::loadFile(filename, hapticFile)) {
  //    return EXIT_FAILURE;
  //  }
  //}

  IOJson::writeFile(hapticFile, output);
  return EXIT_SUCCESS;
}
