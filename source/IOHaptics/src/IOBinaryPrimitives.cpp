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

#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <array>
#include <fstream>

namespace haptics::io {

auto IOBinaryPrimitives::writeString(const std::string &text, std::ofstream &file) -> void {
  std::string str = text;
  str.append(1, '\x00');
  file.write(str.c_str(), static_cast<int>(str.size()));
}

auto IOBinaryPrimitives::readString(std::ifstream &file) -> std::string {
  char c = 0;
  std::string str;
  while (file.get(c), c != '\0') {
    str += c;
  }
  return str;
}

auto IOBinaryPrimitives::writeFloat(const float &f, std::ofstream &file) -> void {
  writeNBytes<float, 4>(f, file);
}

auto IOBinaryPrimitives::readFloat(std::ifstream &file) -> float {
  return readNBytes<float, 4>(file);
}

template <class T, size_t bytesCount>
auto IOBinaryPrimitives::writeNBytes(const T &value, std::ofstream &file) -> void {
  std::array<char, bytesCount> bytes{};
  memcpy(&bytes, &value, sizeof(value));
  std::reverse(bytes.begin(), bytes.end());
  file.write(bytes.data(), bytesCount);
}

template <class T, size_t bytesCount>
auto IOBinaryPrimitives::readNBytes(std::ifstream &file) -> T {
  T value = 0;

  std::array<char, bytesCount> bytes{};
  file.read(bytes.data(), bytesCount);
  std::reverse(bytes.begin(), bytes.end());
  memcpy(&value, &bytes, sizeof(value));

  return value;
}

template auto IOBinaryPrimitives::writeNBytes<unsigned char, 1>(const unsigned char &value,
                                                                std::ofstream &file) -> void;
template auto IOBinaryPrimitives::writeNBytes<unsigned short, 2>(const unsigned short &value,
                                                                 std::ofstream &file) -> void;
template auto IOBinaryPrimitives::writeNBytes<short, 2>(const short &value, std::ofstream &file)
    -> void;
template auto IOBinaryPrimitives::writeNBytes<uint16_t, 2>(const uint16_t &value,
                                                           std::ofstream &file) -> void;
template auto IOBinaryPrimitives::writeNBytes<int, 4>(const int &value, std::ofstream &file)
    -> void;
template auto IOBinaryPrimitives::writeNBytes<uint32_t, 4>(const uint32_t &value,
                                                           std::ofstream &file) -> void;

template auto IOBinaryPrimitives::readNBytes<unsigned char, 1>(std::ifstream &file)
    -> unsigned char;
template auto IOBinaryPrimitives::readNBytes<unsigned short, 2>(std::ifstream &file)
    -> unsigned short;
template auto IOBinaryPrimitives::readNBytes<short, 2>(std::ifstream &file) -> short;
template auto IOBinaryPrimitives::readNBytes<uint16_t, 2>(std::ifstream &file) -> uint16_t;
template auto IOBinaryPrimitives::readNBytes<int, 4>(std::ifstream &file) -> int;
template auto IOBinaryPrimitives::readNBytes<uint32_t, 4>(std::ifstream &file) -> uint32_t;
} // namespace haptics::types