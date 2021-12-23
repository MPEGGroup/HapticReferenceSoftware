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

#include <Tools/include/InputParser.h>

namespace haptics::tools {

InputParser::InputParser(const std::vector<const char *> &args) {
  for (const auto &a : args) {
    tokens.emplace_back(a);
  }
}

[[nodiscard]] auto InputParser::getCmdOption(const std::string &option) const
    -> const std::string & {
  std::vector<std::string>::const_iterator itr;
  itr = std::find(this->tokens.begin(), this->tokens.end(), option);
  if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
    return *itr;
  }
  static const std::string empty_string;
  return empty_string;
}

[[nodiscard]] auto InputParser::cmdOptionExists(const std::string &option) const -> bool {
  return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
}

void InputParser::help(const std::string &prg_name) {
  std::cout << "usages: " << prg_name << " [-h] [{-v, -q}] -f <FILE> [-o <OUTPUT_FILE>]\n\n"
            << "This piece of software converts binary encoded RM0 files submitted to the MPEG CfP "
               "call for Haptic standardization into their human-readable format\n"
            << "\npositional arguments:\n"
            << "\t-f, --file <FILE>\t\tfile to convert\n"
            << "\noptional arguments:\n"
            << "\t-h, --help\t\t\tshow this help message and exit\n"
            << "\t-v, --verbose\t\t\tbe more verbose\n"
            << "\t-q, --quiet\t\t\tbe more quiet\n"
            << "\t-o, --output<OUTPUT_FILE>\toutput file\n";
}

auto InputParser::getFileExt(std::string &filename) -> std::string {
  size_t i = filename.rfind('.', filename.length());
  if (i != std::string::npos) {
    return (filename.substr(i + 1, filename.length() - i));
  }

  return std::string{""};
}

} // namespace haptics::tools
