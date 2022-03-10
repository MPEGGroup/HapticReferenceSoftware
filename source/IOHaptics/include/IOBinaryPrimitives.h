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

#ifndef IOBINARYPRIMITIVES_H
#define IOBINARYPRIMITIVES_H

#include <array>
#include <cstring>
#include <fstream>
#include <string>

namespace haptics::io {

class IOBinaryPrimitives {
public:
  static auto readString(std::ifstream &file) -> std::string;
  static auto readFloat(std::ifstream &file) -> float;
  template <class T, size_t bytesCount> static auto readNBytes(std::ifstream &file) -> T {
    T value = 0;

    std::array<char, bytesCount> bytes{};
    file.read(bytes.data(), bytesCount);

    int i = 0;
    for (auto &byte : bytes) {
      value |= static_cast<T>(byte) << sizeof(char) * i;
      i++;
    }

    return value;
  }

  static auto writeString(const std::string &text, std::ofstream &file) -> void;
  static auto writeFloat(const float &f, std::ofstream &file) -> void;
  template <class T, size_t bytesCount>
  static auto writeNBytes(const T &value, std::ofstream &file) -> void {
    std::array<char, bytesCount> bytes{};
    const int mask = 0x000000ff;
    int i = 0;
    for (auto &byte : bytes) {
      byte = (value & (mask >> i * sizeof(char))) >> i * sizeof(char);
      i++;
    }
    file.write(bytes.data(), bytesCount);
  }
};
} // namespace haptics::io
#endif // IOBINARYPRIMITIVES_H
