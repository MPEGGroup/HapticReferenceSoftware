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

#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace haptics::io {
constexpr float MAX_FLOAT = 10000;
constexpr float MAX_FREQUENCY = 10000;
constexpr float MAX_AMPLITUDE = 1;
constexpr float MAX_PHASE = 6.28318;
constexpr int BYTE_SIZE = 8;
class IOBinaryPrimitives {
public:
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

  template <class T, size_t bitCount>
  static auto writeNBits(T value, std::vector<bool> &output) -> void {
    for (size_t i = 0; i < bitCount; i++) {
      output.push_back((value >> (bitCount - i - 1)) & 1U);
    }
  }

  template <size_t length>
  static auto readFloatNBits(std::vector<bool> &bitstream, int &startIdx, float minValue,
                             float maxValue) -> float {
    std::string str;
    for (auto i = startIdx; i < startIdx + length; i++) {
      if (bitstream[i]) {
        str += "1";
      } else {
        str += "0";
      }
    }
    auto intValue = std::bitset<length>(str).to_ulong();
    auto maxIntValue = static_cast<uint64_t>(std::pow(2, length) - 1);
    auto normalizedValue = intValue / static_cast<float>(maxIntValue);
    normalizedValue = std::clamp<float>(normalizedValue, 0, 1);
    auto value = normalizedValue * (maxValue - minValue) + minValue;
    startIdx += static_cast<int>(length);
    return value;
  }

  template <class T, size_t bitCount>
  static auto writeFloatNBits(float value, std::vector<bool> &output, float minValue,
                              float maxValue) -> void {
    auto normalizedValue = (value - minValue) / (maxValue - minValue);
    normalizedValue = std::clamp<float>(normalizedValue, 0, 1);
    auto maxIntValue = static_cast<T>(std::pow(2, bitCount) - 1);
    auto intValue = static_cast<T>((double)(normalizedValue)*maxIntValue);
    writeNBits<T, bitCount>(intValue, output);
  }

  static auto writeStrBits(std::string bits, std::vector<bool> &bitstream) -> void {
    // std::reverse(bits.begin(), bits.end());
    for (char c : bits) {
      bitstream.push_back(c == '1');
    }
  }
  static auto readInt(std::vector<bool> bitstream, int &startIdx, int length) -> int {
    std::string tsBits;
    for (int i = startIdx; i < startIdx + length; i++) {
      if ((bitstream[i])) {
        tsBits += "1";
      } else {
        tsBits += "0";
      }
    }
    startIdx += length;
    auto inv = std::stoul(tsBits, nullptr, 2);
    return static_cast<int>(inv);
  }

  static auto readString(std::vector<bool> bitstream, int &startIdx, int length) -> std::string {
    std::string res;
    for (int i = startIdx; i < startIdx + (BYTE_SIZE * length); i += BYTE_SIZE) {
      std::string bitsStr;
      for (auto b : std::vector<bool>(bitstream.begin() + i, bitstream.begin() + i + BYTE_SIZE)) {
        if (b) {
          bitsStr += "1";
        } else {
          bitsStr += "0";
        }
      }
      std::bitset<BYTE_SIZE> cBits(bitsStr);
      res += static_cast<char>((cBits).to_ulong());
    }
    startIdx += length * BYTE_SIZE;
    return res;
  }

  static auto writeBitset(const std::vector<bool> &bitset, std::ostream &file) -> void {
    auto bitsetSize = bitset.size();
    for (unsigned int i = 0; i < bitset.size(); i += BYTE_SIZE) {
      char byte = 0;
      for (uint8_t j = 0; j < BYTE_SIZE; j++) {
        if ((i + j < bitsetSize) && bitset[i + j]) {
          byte = static_cast<char>(byte | (1U << (BYTE_SIZE - 1 - j)));
        }
      }
      file.write(&byte, 1);
    }
  }

  static auto readNBytes(std::istream &file, int nbBytes, std::vector<bool> &bitstream) -> bool {
    std::vector<char> bytes(nbBytes);
    file.read(bytes.data(), nbBytes);
    for (auto byte : bytes) {
      for (uint8_t i = 0; i < BYTE_SIZE; i++) {
        bitstream.push_back(((byte >> (BYTE_SIZE - i - 1)) & 1U) == 1);
      }
    }
    return true;
  }
};

} // namespace haptics::io
#endif // IOBINARYPRIMITIVES_H
