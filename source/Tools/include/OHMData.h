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

#ifndef OHMDATA_H
#define OHMDATA_H

#include <string>
#include <vector>

namespace haptics::tools {

class OHMData {
  static constexpr int oneByteShift = 8;
  static constexpr int twoBytesShift = 16;
  static constexpr int threeBytesShift = 24;
  static constexpr int descriptionByteSize = 32;
  static constexpr int fileNameByteSize = 64;

public:
  enum class Body : uint32_t {
    UNSPECIFIED = 0x00000000,
    HEAD_FRONT = 0x00000001,
    HEAD_BACK = 0x00000002,
    HEAD_RIGHT = 0x00000004,
    HEAD_LEFT = 0x00000008,
    RIGHT_UPPER_CHEST = 0x00000010,
    LEFT_UPPER_CHEST = 0x00000020,
    ABDOMEN = 0x00000040,
    WAIST = 0x00000080,
    UPPER_BACK = 0x00000100,
    LOWER_BACK = 0x00000200,
    RIGHT_UPPERARM = 0x00000400,
    LEFT_UPPERARM = 0x00000800,
    RIGHT_FOREARM = 0x00001000,
    LEFT_FOREARM = 0x00002000,
    RIGHT_WRIST = 0x00004000,
    LEFT_WRIST = 0x00008000,
    RIGHT_HAND_PALM = 0x00010000,
    LEFT_HAND_PALM = 0x00020000,
    RIGHT_HAND_DORSUM = 0x00040000,
    LEFT_HAND_DORSUM = 0x00080000,
    RIGHT_HAND_FINGERS = 0x00100000,
    LEFT_HAND_FINGERS = 0x00200000,
    RIGHT_THIGH = 0x00400000,
    LEFT_THIGH = 0x00800000,
    RIGHT_CALF = 0x01000000,
    LEFT_CALF = 0x02000000,
    RIGHT_FOOT_PALM = 0x04000000,
    LEFT_FOOT_PALM = 0x08000000,
    RIGHT_FOOT_DORSUM = 0x10000000,
    LEFT_FOOT_DORSUM = 0x20000000,
    RIGHT_FOOT_FINGERS = 0x40000000,
    LEFT_FOOT_FINGERS = 0x80000000,
    ALL = 0xFFFFFFFF,
    HEAD = HEAD_FRONT | HEAD_BACK | HEAD_LEFT | HEAD_RIGHT,
    UPPER_CHEST = RIGHT_UPPER_CHEST | LEFT_UPPER_CHEST,
    RIGHT_HAND = RIGHT_HAND_PALM | RIGHT_HAND_DORSUM | RIGHT_HAND_FINGERS,
    LEFT_HAND = LEFT_HAND_PALM | LEFT_HAND_DORSUM | LEFT_HAND_FINGERS,
    RIGHT_ARM = RIGHT_UPPERARM | RIGHT_FOREARM | RIGHT_WRIST | RIGHT_HAND,
    LEFT_ARM = LEFT_UPPERARM | LEFT_FOREARM | LEFT_WRIST | LEFT_HAND,
    RIGHT_FOOT = RIGHT_FOOT_PALM | RIGHT_FOOT_DORSUM | RIGHT_FOOT_FINGERS,
    LEFT_FOOT = LEFT_FOOT_PALM | LEFT_FOOT_DORSUM | LEFT_FOOT_FINGERS,
    RIGHT_LEG = RIGHT_THIGH | RIGHT_CALF | RIGHT_FOOT,
    LEFT_LEG = LEFT_THIGH | LEFT_CALF | LEFT_FOOT
  };
  struct HapticChannelMetadata {
    std::string channelDescription;
    float gain = 1.0F;
    Body bodyPartMask = Body::UNSPECIFIED;
  };
  struct HapticElementMetadata {
    std::string elementFilename;
    std::string elementDescription;
    short numHapticChannels = 0;
    std::vector<HapticChannelMetadata> channelsMetadata;
  };

private:
  std::string header;
  short version = 1;
  short numElements = 0;
  std::string description;
  std::vector<HapticElementMetadata> elementsMetadata;

  static auto fillString(const std::string &text, unsigned int numCharacters) -> std::string;

public:
  explicit OHMData() = default;
  explicit OHMData(std::string &_header, short _version, short _numElements,
                   std::string &_description)
      : header(_header)
      , version(_version)
      , numElements(_numElements)
      , description(_description)
      , elementsMetadata({}){};
  explicit OHMData(const std::string &filePath);
  auto loadFile(const std::string &filePath) -> bool;
  auto writeFile(const std::string &filePath) -> bool;

  [[nodiscard]] auto getVersion() const -> short;
  auto setVersion(short newVersion) -> void;
  [[nodiscard]] auto getNumElements() const -> short;
  [[nodiscard]] auto getHeader() const -> std::string;
  auto setHeader(const std::string &newHeader) -> void;
  [[nodiscard]] auto getDescription() const -> std::string;
  auto setDescription(const std::string &newDescription) -> void;
  auto getHapticElementMetadataSize() -> size_t;
  auto getHapticElementMetadataAt(int index) -> HapticElementMetadata &;
  auto addHapticElementMetadata(HapticElementMetadata &newElementMetadata) -> void;
};
} // namespace haptics::tools
#endif // OHMDATA_H
