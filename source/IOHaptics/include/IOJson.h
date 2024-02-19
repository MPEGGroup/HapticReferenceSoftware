/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2022, ISO/IEC
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

#ifndef IOJSON_H
#define IOJSON_H

#include <Types/include/Haptics.h>
#include <bitset>
#include <cmath>
#include <map>
#include <string>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996 26451 26495 26812 33010)
#endif
#include <rapidjson/document.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::io {

constexpr int BYTE_SIZE_IO = 8;

constexpr short COMPARE_START = 32;
constexpr short ASCII_UPPER_1 = 65;
constexpr short ASCII_UPPER_2 = 90;
constexpr short ASCII_LOWER_1 = 97;
constexpr short ASCII_LOWER_2 = 122;
constexpr short ASCII_DIGIT_1 = 48;
constexpr short ASCII_DIGIT_2 = 57;
constexpr short ASCII_PLUS = 43;
constexpr short ASCII_SOLIDUS = 47;

constexpr short BASE64_UPPER_1 = 0;
constexpr short BASE64_UPPER_2 = 25;
constexpr short BASE64_LOWER_1 = 26;
constexpr short BASE64_LOWER_2 = 51;
constexpr short BASE64_DIGIT_1 = 52;
constexpr short BASE64_DIGIT_2 = 61;
constexpr short BASE64_PLUS = 62;
constexpr short BASE64_SOLIDUS = 63;

constexpr short DIFF_UPPER = 65;
constexpr short DIFF_LOWER = 71;
constexpr short DIFF_DIGIT = 4;



class IOJson {
public:
  static auto loadFile(const std::string &filePath, types::Haptics &haptic) -> bool;
  static auto loadPerceptions(const rapidjson::Value &jsonPerceptions, types::Haptics &haptic)
      -> bool;
  static auto loadAvatars(const rapidjson::Value &jsonAvatars, types::Haptics &haptic) -> bool;
  static auto loadChannels(const rapidjson::Value &jsonChannels, types::Perception &perception)
      -> bool;
  static auto loadLibrary(const rapidjson::Value &jsonLibrary, types::Perception &perception)
      -> bool;
  static auto loadReferenceDevices(const rapidjson::Value &jsonReferenceDevices,
                                   types::Perception &perception) -> bool;
  static auto loadBands(const rapidjson::Value &jsonBands, types::Channel &channel) -> bool;
  static auto loadEffects(const rapidjson::Value &jsonEffects, types::Band &band) -> bool;
  static auto loadKeyframes(const rapidjson::Value &jsonKeyframes, types::Effect &effect) -> bool;
  static auto loadBitstream(const rapidjson::Value &jsonBitstream, types::Effect &effect) -> bool;
  static auto loadVector(const rapidjson::Value &jsonVector, types::Vector &vector) -> bool;
  static auto loadSyncs(const rapidjson::Value &jsonSyncs, types::Haptics &haptic) -> bool;

  static auto extractPerceptions(types::Haptics &haptic, rapidjson::Value &jsonPerceptions,
                                 rapidjson::Document &jsonTree) -> void;
  static auto extractAvatars(types::Haptics &haptic, rapidjson::Value &jsonAvatars,
                             rapidjson::Document &jsonTree) -> void;
  static auto extractChannels(types::Perception &perception, rapidjson::Value &jsonChannels,
                              rapidjson::Document &jsonTree) -> void;
  static auto extractBands(types::Channel &channel, rapidjson::Value &jsonBands,
                           rapidjson::Document &jsonTree) -> void;
  static auto extractLibrary(types::Perception &perception, rapidjson::Value &jsonLibrary,
                             rapidjson::Document &jsonTree) -> void;
  static auto extractReferenceDevices(types::Perception &perception,
                                      rapidjson::Value &jsonReferenceDevices,
                                      rapidjson::Document &jsonTree) -> void;
  static auto extractVector(types::Vector &vector, rapidjson::Value &jsonVector,
                            rapidjson::Document &jsonTree) -> void;
  static auto extractSyncs(types::Haptics &haptic, rapidjson::Value &jsonSyncs,
                           rapidjson::Document &jsonTree) -> void;

  static auto writeFile(types::Haptics &haptic, const std::string &filePath) -> void;

  static auto bytes2bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void;
  static auto bits2bytes(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void;
  static auto base642bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void;
  static auto bits2base64(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void;
};
} // namespace haptics::io
#endif // IOJSON_H
