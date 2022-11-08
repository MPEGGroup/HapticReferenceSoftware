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

#include <Types/include/Track.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace haptics::io {
constexpr float MAX_FLOAT = 10000;
constexpr float MAX_FREQUENCY = 10000;
constexpr float MAX_AMPLITUDE = 1;
constexpr float MAX_PHASE = 3.14159;
constexpr int BYTE_SIZE = 8;
class IOBinaryPrimitives {
public:
  static auto readVector(std::istream &file) -> haptics::types::Vector;
  static auto readString(std::istream &file) -> std::string;

  template <class T, size_t bytesCount> static auto readNBytes(std::istream &file) -> T {
    std::array<char, bytesCount> bytes{};
    file.read(bytes.data(), bytesCount);
    T value = 0;
    for (size_t i = 0; i < bytes.size(); i++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      auto byteVal = static_cast<uint8_t>(bytes[i]);
      value |= static_cast<T>(byteVal) << BYTE_SIZE * i;
    }
    return value;
  }
  template <class T, size_t bytesCount>
  static auto readFloatNBytes(std::istream &file, float minValue, float maxValue) -> float {
    auto intValue = readNBytes<T, bytesCount>(file);
    auto maxIntValue = static_cast<uint64_t>(std::pow(2, bytesCount * BYTE_SIZE) - 1);
    auto normalizedValue = intValue / static_cast<float>(maxIntValue);
    normalizedValue = std::clamp<float>(normalizedValue, 0, 1);
    auto value = normalizedValue * (maxValue - minValue) + minValue;
    return value;
  }

  static auto writeString(const std::string &text, std::ostream &file) -> void;
  template <class T, size_t bytesCount>
  static auto writeNBytes(T value, std::ostream &file) -> void {
    std::array<char, bytesCount> bytes{};
    for (size_t i = 0; i < bytes.size(); i++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      bytes[i] = static_cast<uint8_t>(value >> i * BYTE_SIZE);
    }
    file.write(bytes.data(), bytesCount);
  }
  template <class T, size_t bytesCount>
  static auto writeFloatNBytes(float value, std::ostream &file, float minValue, float maxValue)
      -> void {
    auto normalizedValue = (value - minValue) / (maxValue - minValue);
    normalizedValue = std::clamp<float>(normalizedValue, 0, 1);
    auto maxIntValue = static_cast<T>(std::pow(2, bytesCount * BYTE_SIZE) - 1);
    auto intValue = static_cast<T>((double)(normalizedValue)*maxIntValue);
    writeNBytes<T, bytesCount>(intValue, file);
  }
};
} // namespace haptics::io
#endif // IOBINARYPRIMITIVES_H
