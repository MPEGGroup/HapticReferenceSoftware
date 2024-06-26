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

#ifndef PERCEPTION_H
#define PERCEPTION_H

#include <Types/include/Channel.h>
#include <Types/include/ReferenceDevice.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace haptics::types {

enum class PerceptionModality {
  Other = 0,
  Pressure = 1,
  Acceleration = 2,
  Velocity = 3,
  Position = 4,
  Temperature = 5,
  Vibrotactile = 6,
  Water = 7,
  Wind = 8,
  Force = 9,
  VibrotactileTexture = 10,
  Stiffness = 11,
  Friction = 12,
  Electrotactile = 13
};

static const std::map<std::string, PerceptionModality> stringToPerceptionModality = {
    {"Other", PerceptionModality::Other},
    {"Pressure", PerceptionModality::Pressure},
    {"Acceleration", PerceptionModality::Acceleration},
    {"Velocity", PerceptionModality::Velocity},
    {"Position", PerceptionModality::Position},
    {"Temperature", PerceptionModality::Temperature},
    {"Vibration", PerceptionModality::Vibrotactile},
    {"Vibrotactile", PerceptionModality::Vibrotactile},
    {"Water", PerceptionModality::Water},
    {"Wind", PerceptionModality::Wind},
    {"Kinesthetic", PerceptionModality::Force},
    {"Force", PerceptionModality::Force},
    {"Vibrotactile Texture", PerceptionModality::VibrotactileTexture},
    {"Texture", PerceptionModality::VibrotactileTexture},
    {"Stiffness", PerceptionModality::Stiffness},
    {"Friction", PerceptionModality::Friction},
    {"Electrotactile", PerceptionModality::Electrotactile}};
static const std::map<PerceptionModality, std::string> perceptionModalityToString = {
    {PerceptionModality::Other, "Other"},
    {PerceptionModality::Pressure, "Pressure"},
    {PerceptionModality::Acceleration, "Acceleration"},
    {PerceptionModality::Velocity, "Velocity"},
    {PerceptionModality::Position, "Position"},
    {PerceptionModality::Temperature, "Temperature"},
    {PerceptionModality::Vibrotactile, "Vibrotactile"},
    {PerceptionModality::Water, "Water"},
    {PerceptionModality::Wind, "Wind"},
    {PerceptionModality::Force, "Force"},
    {PerceptionModality::VibrotactileTexture, "Vibrotactile Texture"},
    {PerceptionModality::Stiffness, "Stiffness"},
    {PerceptionModality::Friction, "Friction"},
    {PerceptionModality::Electrotactile, "Electrotactile"}};

class Perception {
public:
  explicit Perception() = default;
  explicit Perception(int newId, int newAvatarId, std::string newDescription,
                      PerceptionModality newPerceptionModality)
      : id(newId)
      , avatarId(newAvatarId)
      , description(std::move(newDescription))
      , perceptionModality(newPerceptionModality)
      , channels({})
      , unitExponent(std::nullopt)
      , perceptionUnitExponent(std::nullopt){};

  [[nodiscard]] auto getAvatarId() const -> int;
  auto setAvatarId(int newAvatarId) -> void;
  [[nodiscard]] auto getEffectSemanticSchemeOrDefault() const -> std::string;
  [[nodiscard]] auto getEffectSemanticScheme() const -> std::optional<std::string>;
  auto setEffectSemanticScheme(std::string &newEffectSemantic) -> void;
  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getDescription() const -> std::string;
  auto setDescription(std::string &newDescription) -> void;
  [[nodiscard]] auto getPriority() const -> std::optional<int>;
  [[nodiscard]] auto getPriorityOrDefault() const -> int;
  auto setPriority(int newPriority) -> void;
  [[nodiscard]] auto getPerceptionModality() const -> PerceptionModality;
  auto setPerceptionModality(PerceptionModality newPerceptionModality) -> void;
  [[nodiscard]] auto getUnitExponent() const -> std::optional<int8_t>;
  [[nodiscard]] auto getUnitExponentOrDefault() const -> int8_t;
  auto setUnitExponent(std::optional<int8_t> newUnitExponent) -> void;
  [[nodiscard]] auto getPerceptionUnitExponent() const -> std::optional<int8_t>;
  [[nodiscard]] auto getPerceptionUnitExponentOrDefault() const -> int8_t;
  auto setPerceptionUnitExponent(std::optional<int8_t> newPerceptionUnitExponent) -> void;
  auto getChannelsSize() -> size_t;
  auto getChannelAt(int index) -> Channel &;
  auto addChannel(haptics::types::Channel &newChannel) -> void;
  auto getReferenceDevicesSize() -> size_t;
  auto getReferenceDeviceAt(int index) -> ReferenceDevice &;
  auto clearReferenceDevices() -> void { referenceDevices.clear(); }
  auto replaceChannelAt(int index, Channel &newChannel) -> bool;
  auto replaceChannelMetadataAt(int index, Channel &newChannel) -> bool;
  auto removeChannelAt(int index) -> bool;
  auto clearChannels() -> void { channels.clear(); };

  auto addReferenceDevice(haptics::types::ReferenceDevice &newReferenceDevice) -> void;
  auto addReferenceDevice(
      const std::vector<std::tuple<
          int, std::string, std::optional<uint32_t>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<haptics::types::ActuatorType>>>
          &referenceDeviceValues) -> void;
  static auto convertToModality(const std::string &modalityString) -> PerceptionModality;
  auto getEffectLibrarySize() -> size_t;
  auto getBasisEffectAt(int index) -> haptics::types::Effect &;
  auto addBasisEffect(haptics::types::Effect &newEffect) -> void;
  auto clearEffectLibrary() -> void { effectLibrary.clear(); };
  auto refactorEffects() -> void;
  auto searchForEquivalentEffects(Effect &effect, int startingChannel)
      -> std::vector<std::tuple<int, int, int>>;
  auto linearizeLibrary() -> void;
  auto getEffectById(int id) -> std::optional<Effect>;

private:
  static constexpr int8_t DEFAULT_UNIT_EXPONENT = -3;
  static constexpr int8_t DEFAULT_PERCEPTION_UNIT_EXPONENT = 0;
  inline static const std::string DEFAULT_SEMANTIC_SCHEME =
      "urn:mpeg:mpegi:haptics:effectsemantic:2023";

  int id = -1;
  int avatarId = -1;
  std::string description;
  std::optional<int> priority = std::nullopt;
  std::optional<std::string> effectSemanticScheme = std::nullopt;
  PerceptionModality perceptionModality = PerceptionModality::Other;
  std::vector<Channel> channels = {};
  std::vector<ReferenceDevice> referenceDevices;
  std::optional<int8_t> unitExponent = std::nullopt;
  std::optional<int8_t> perceptionUnitExponent = std::nullopt;
  std::vector<Effect> effectLibrary = std::vector<Effect>{};
};
} // namespace haptics::types
#endif // PERCEPTION_H
