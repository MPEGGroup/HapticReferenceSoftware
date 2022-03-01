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

#include <Types/include/ReferenceDevice.h>
#include <Types/include/Track.h>
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
  Vibration = 6,
  Water = 7,
  Wind = 8,
  Kinesthetic = 9,
  Texture = 10,
  Stiffness = 11
};

static const std::map<std::string, PerceptionModality> stringToPerceptionModality = {
    {"Other", PerceptionModality::Other},
    {"Pressure", PerceptionModality::Pressure},
    {"Acceleration", PerceptionModality::Acceleration},
    {"Velocity", PerceptionModality::Velocity},
    {"Position", PerceptionModality::Position},
    {"Temperature", PerceptionModality::Temperature},
    {"Vibration", PerceptionModality::Vibration},
    {"Water", PerceptionModality::Water},
    {"Wind", PerceptionModality::Wind},
    {"Kinesthetic", PerceptionModality::Kinesthetic},
    {"Texture", PerceptionModality::Texture},
    {"Stiffness", PerceptionModality::Stiffness}};
static const std::map<PerceptionModality, std::string> perceptionModalityToString = {
    {PerceptionModality::Other, "Other"},
    {PerceptionModality::Pressure, "Pressure"},
    {PerceptionModality::Acceleration, "Acceleration"},
    {PerceptionModality::Velocity, "Velocity"},
    {PerceptionModality::Position, "Position"},
    {PerceptionModality::Temperature, "Temperature"},
    {PerceptionModality::Vibration, "Vibration"},
    {PerceptionModality::Water, "Water"},
    {PerceptionModality::Wind, "Wind"},
    {PerceptionModality::Kinesthetic, "Kinesthetic"},
    {PerceptionModality::Texture, "Texture"},
    {PerceptionModality::Stiffness, "Stiffness"}};

class Perception {
public:
  explicit Perception() = default;
  explicit Perception(int newId, int newAvatarId, std::string newDescription,
                      PerceptionModality newPerceptionModality)
      : id(newId)
      , avatarId(newAvatarId)
      , description(std::move(newDescription))
      , perceptionModality(newPerceptionModality)
      , tracks({}){};

  [[nodiscard]] auto getAvatarId() const -> int;
  auto setAvatarId(int newAvatarId) -> void;
  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getDescription() const -> std::string;
  auto setDescription(std::string &newDescription) -> void;
  [[nodiscard]] auto getPerceptionModality() const -> PerceptionModality;
  auto setPerceptionModality(PerceptionModality newPerceptionModality) -> void;
  auto getTracksSize() -> size_t;
  auto getTrackAt(int index) -> Track &;
  auto addTrack(haptics::types::Track &newTrack) -> void;
  auto getReferenceDevicesSize() -> size_t;
  auto getReferenceDeviceAt(int index) -> ReferenceDevice &;
  auto replaceTrackAt(int index, Track &newTrack) -> bool;
  auto addReferenceDevice(haptics::types::ReferenceDevice &newReferenceDevice) -> void;
  auto addReferenceDevice(
      const std::vector<std::tuple<
          int, std::string, std::optional<uint32_t>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
          std::optional<float>, std::optional<haptics::types::ActuatorType>>>
          &referenceDeviceValues) -> void;
  static auto convertToModality(const std::string &modalityString) -> PerceptionModality;

private:
  int id = -1;
  int avatarId = -1;
  std::string description;
  PerceptionModality perceptionModality = PerceptionModality::Other;
  std::vector<Track> tracks = {};
  std::vector<ReferenceDevice> referenceDevices;
};
} // namespace haptics::types
#endif // PERCEPTION_H
