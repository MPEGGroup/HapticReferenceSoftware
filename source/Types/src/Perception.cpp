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

#include <Types/include/Perception.h>

namespace haptics::types {

[[nodiscard]] auto Perception::getId() const -> int { return id; }

auto Perception::setId(int newId) -> void { id = newId; }

[[nodiscard]] auto Perception::getAvatarId() const -> int { return avatarId; }

auto Perception::setAvatarId(int newAvatarId) -> void { avatarId = newAvatarId; }

[[nodiscard]] auto Perception::getDescription() const -> std::string { return description; }

auto Perception::setDescription(std::string &newDescription) -> void {
  description = newDescription;
}

[[nodiscard]] auto Perception::getPerceptionModality() const -> PerceptionModality {
  return perceptionModality;
}

auto Perception::setPerceptionModality(PerceptionModality newPerceptionModality) -> void {
  perceptionModality = newPerceptionModality;
}

[[nodiscard]] auto Perception::getUnitExponent() const -> std::optional<int8_t> {
  return this->unitExponent;
}

[[nodiscard]] auto Perception::getUnitExponentOrDefault() const -> int8_t {
  return this->getUnitExponent().value_or(Perception::DEFAULT_UNIT_EXPONENT);
}

auto Perception::setUnitExponent(std::optional<int8_t> newUnitExponent) -> void {
  this->unitExponent = newUnitExponent;
}

[[nodiscard]] auto Perception::getPerceptionUnitExponent() const -> std::optional<int8_t> {
  return this->perceptionUnitExponent;
}

[[nodiscard]] auto Perception::getPerceptionUnitExponentOrDefault() const -> int8_t {
  return this->getPerceptionUnitExponent().value_or(Perception::DEFAULT_PERCEPTION_UNIT_EXPONENT);
}

auto Perception::setPerceptionUnitExponent(std::optional<int8_t> newPerceptionUnitExponent)
    -> void {
  this->perceptionUnitExponent = newPerceptionUnitExponent;
}

auto Perception::getTracksSize() -> size_t { return tracks.size(); }

auto Perception::getTrackAt(int index) -> haptics::types::Track & { return tracks.at(index); }

auto Perception::replaceTrackAt(int index, haptics::types::Track &newTrack) -> bool {
  if (index < 0 || index >= (int)tracks.size()) {
    return false;
  }
  tracks[index] = newTrack;
  return true;
}

auto Perception::addTrack(haptics::types::Track &newTrack) -> void { tracks.push_back(newTrack); }

auto Perception::getReferenceDevicesSize() -> size_t { return referenceDevices.size(); }

auto Perception::getReferenceDeviceAt(int index) -> ReferenceDevice & {
  return referenceDevices.at(index);
}
auto Perception::addReferenceDevice(haptics::types::ReferenceDevice &newReferenceDevice) -> void {
  referenceDevices.push_back(newReferenceDevice);
}

auto Perception::addReferenceDevice(
    const std::vector<std::tuple<
        int, std::string, std::optional<uint32_t>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
        std::optional<float>, std::optional<haptics::types::ActuatorType>>> &referenceDeviceValues)
    -> void {
  const size_t idIndex = 0;
  const size_t nameIndex = 1;
  const size_t bodyPartIndex = 2;
  const size_t maximumFrequencyIndex = 3;
  const size_t minimumFrequencyIndex = 4;
  const size_t resonanceFrequencyIndex = 5;
  const size_t maximumAmplitudeIndex = 6;
  const size_t impedanceIndex = 7;
  const size_t maximumVoltageIndex = 8;
  const size_t maximumCurrentIndex = 9;
  const size_t maximumDisplacementIndex = 10;
  const size_t weightIndex = 11;
  const size_t sizeIndex = 12;
  const size_t customIndex = 13;
  const size_t typeIndex = 14;
  for (auto values : referenceDeviceValues) {

    haptics::types::ReferenceDevice myDevice(std::get<idIndex>(values),
                                             std::get<nameIndex>(values));

    if (std::get<bodyPartIndex>(values).has_value()) {
      myDevice.setBodyPartMask(std::get<bodyPartIndex>(values).value());
    }
    if (std::get<maximumFrequencyIndex>(values).has_value()) {
      myDevice.setMaximumFrequency(std::get<maximumFrequencyIndex>(values).value());
    }
    if (std::get<minimumFrequencyIndex>(values).has_value()) {
      myDevice.setMinimumFrequency(std::get<minimumFrequencyIndex>(values).value());
    }
    if (std::get<resonanceFrequencyIndex>(values).has_value()) {
      myDevice.setResonanceFrequency(std::get<resonanceFrequencyIndex>(values).value());
    }
    if (std::get<maximumAmplitudeIndex>(values).has_value()) {
      myDevice.setMaximumAmplitude(std::get<maximumAmplitudeIndex>(values).value());
    }
    if (std::get<impedanceIndex>(values).has_value()) {
      myDevice.setImpedance(std::get<impedanceIndex>(values).value());
    }
    if (std::get<maximumVoltageIndex>(values).has_value()) {
      myDevice.setMaximumVoltage(std::get<maximumVoltageIndex>(values).value());
    }
    if (std::get<maximumCurrentIndex>(values).has_value()) {
      myDevice.setMaximumCurrent(std::get<maximumCurrentIndex>(values).value());
    }
    if (std::get<maximumDisplacementIndex>(values).has_value()) {
      myDevice.setMaximumDisplacement(std::get<maximumDisplacementIndex>(values).value());
    }
    if (std::get<weightIndex>(values).has_value()) {
      myDevice.setWeight(std::get<weightIndex>(values).value());
    }
    if (std::get<sizeIndex>(values).has_value()) {
      myDevice.setSize(std::get<sizeIndex>(values).value());
    }
    if (std::get<customIndex>(values).has_value()) {
      myDevice.setCustom(std::get<customIndex>(values).value());
    }
    if (std::get<typeIndex>(values).has_value()) {
      myDevice.setType(std::get<typeIndex>(values).value());
    }

    addReferenceDevice(myDevice);
  }
}

auto Perception::convertToModality(const std::string &modalityString) -> PerceptionModality {
  if (stringToPerceptionModality.count(modalityString) != 0) {
    return stringToPerceptionModality.at(modalityString);
  }
  if (modalityString == "Pressure effect") {
    return PerceptionModality::Pressure;
  }
  if (modalityString == "Acceleration effect") {
    return PerceptionModality::Acceleration;
  }
  if (modalityString == "Velocity effect") {
    return PerceptionModality::Velocity;
  }
  if (modalityString == "Position effect") {
    return PerceptionModality::Position;
  }
  if (modalityString == "Temperature effect") {
    return PerceptionModality::Temperature;
  }
  if (modalityString == "Vibration effect" || modalityString == "Vibrotactile effect") {
    return PerceptionModality::Vibrotactile;
  }
  if (modalityString == "Water effect") {
    return PerceptionModality::Water;
  }
  if (modalityString == "Wind effect") {
    return PerceptionModality::Wind;
  }
  if (modalityString == "Velocity effect") {
    return PerceptionModality::Velocity;
  }
  if (modalityString == "Kinesthetic effect" || modalityString == "Force effect") {
    return PerceptionModality::Force;
  }
  if (modalityString == "Vibrotactile Texture effect" || modalityString == "Texture effect") {
    return PerceptionModality::VibrotactileTexture;
  }
  if (modalityString == "Stiffness effect") {
    return PerceptionModality::Stiffness;
  }

  return PerceptionModality::Other;
}

} // namespace haptics::types
