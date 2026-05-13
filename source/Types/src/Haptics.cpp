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

#include <Types/include/Haptics.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

namespace haptics::types {

[[nodiscard]] auto Haptics::getVersion() const -> std::string { return version; }

auto Haptics::setVersion(std::string &newVersion) -> void { version = newVersion; }

[[nodiscard]] auto Haptics::getProfile() const -> std::string { return profile; }

auto Haptics::setProfile(std::string &newProfile) -> void { profile = newProfile; }

[[nodiscard]] auto Haptics::getLevel() const -> uint8_t { return level; }

auto Haptics::setLevel(uint8_t newLevel) -> void { level = newLevel; }

[[nodiscard]] auto Haptics::getDate() const -> std::string { return date; }
auto Haptics::setDate(std::string &newDate) -> void { date = newDate; }

auto Haptics::setCurrentDate() -> void {
  auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  date = ss.str();
}

auto Haptics::checkDate() -> bool {
  const std::regex txt_regex("[+-]?[0-9]{4}(-[01][0-9](-[0-3][0-9](T[0-2][0-9]:[0-5][0-9]:?([0-5]["
                             "0-9](.[0-9]+)?)?[+-][0-2][0-9]:[0-5][0-9]Z?)?)?)?");
  return regex_match(date, txt_regex);
}

[[nodiscard]] auto Haptics::getDescription() const -> std::string { return description; }

auto Haptics::setDescription(std::string &newDescription) -> void { description = newDescription; }

[[nodiscard]] auto Haptics::getPerceptionsSize() -> size_t { return perceptions.size(); }

[[nodiscard]] auto Haptics::getPerceptionAt(int index) -> Perception & {
  return perceptions.at(index);
}

auto Haptics::replacePerceptionAt(int index, Perception &newPerception) -> bool {
  if (index < 0 || index >= (int)perceptions.size()) {
    return false;
  }

  perceptions[index] = newPerception;
  return true;
}

auto Haptics::replacePerceptionMetadataAt(int index, Perception &newPerception) -> bool {
  if (index < 0 || index >= (int)perceptions.size()) {
    return false;
  }
  perceptions[index].setId(newPerception.getId());
  perceptions[index].setAvatarId(newPerception.getId());
  auto desc = newPerception.getDescription();
  perceptions[index].setDescription(desc);
  perceptions[index].setPerceptionModality(newPerception.getPerceptionModality());
  perceptions[index].setUnitExponent(newPerception.getUnitExponentOrDefault());
  perceptions[index].setPerceptionUnitExponent(newPerception.getPerceptionUnitExponentOrDefault());
  auto semantic = newPerception.getEffectSemanticSchemeOrDefault();
  perceptions[index].setEffectSemanticScheme(semantic);
  perceptions[index].clearReferenceDevices();
  for (auto i = 0; i < static_cast<int>(newPerception.getReferenceDevicesSize()); i++) {
    perceptions[index].addReferenceDevice(newPerception.getReferenceDeviceAt(i));
  }
  perceptions[index].clearEffectLibrary();
  for (auto i = 0; i < static_cast<int>(newPerception.getEffectLibrarySize()); i++) {
    perceptions[index].addBasisEffect(newPerception.getBasisEffectAt(i));
  }
  return true;
}

auto Haptics::removePerceptionAt(int index) -> bool {
  if (index < 0 || index >= (int)perceptions.size()) {
    return false;
  }

  perceptions.erase(perceptions.begin() + index);
  return true;
}

auto Haptics::addPerception(Perception &newPerception) -> void {
  perceptions.push_back(newPerception);
}

[[nodiscard]] auto Haptics::getAvatarsSize() -> size_t { return avatars.size(); }

[[nodiscard]] auto Haptics::getAvatarAt(int index) -> Avatar & { return avatars.at(index); }

auto Haptics::addAvatar(Avatar &newAvatar) -> void { avatars.push_back(newAvatar); }

[[nodiscard]] auto Haptics::getTimescaleOrDefault() const -> uint64_t {
  return this->getTimescale().value_or(Haptics::DEFAULT_TIMESCALE);
}

[[nodiscard]] auto Haptics::getTimescale() const -> std::optional<uint64_t> {
  return this->timescale;
}

auto Haptics::setTimescale(std::optional<uint64_t> newTimescale) -> void {
  this->timescale = newTimescale;
}

[[nodiscard]] auto Haptics::getSyncsSize() -> size_t { return syncs.size(); }

[[nodiscard]] auto Haptics::getSyncsAt(int index) -> Sync & { return syncs.at(index); }

auto Haptics::addSync(Sync &newSync) -> void { syncs.push_back(newSync); }

auto Haptics::loadMetadataFromOHM(haptics::tools::OHMData data) -> void {
  version = "2025";
  setCurrentDate();
  description = data.getDescription();
  auto numElements = static_cast<int>(data.getHapticElementMetadataSize());
  for (int i = 0; i < numElements; i++) {
    auto element = data.getHapticElementMetadataAt(i);
    std::string elemDescription = element.elementDescription;
    PerceptionModality perceptionModality = Perception::convertToModality(elemDescription);
    Perception perception(i, 0, elemDescription, perceptionModality);
    short numChannels = element.numHapticChannels;
    for (int j = 0; j < numChannels; j++) {
      auto OHMChannel = element.channelsMetadata[j];
      Channel channel(j, OHMChannel.channelDescription, OHMChannel.gain, 1,
                      static_cast<uint32_t>(OHMChannel.bodyPartMask));
      perception.addChannel(channel);
    }
    perceptions.push_back(perception);
  }
}

auto Haptics::extractMetadataToOHM(std::string &filename) -> haptics::tools::OHMData {
  std::string header = std::string("OHM ");
  auto v = static_cast<short>(version.empty() ? 0 : std::stoi(version));
  std::string desc = description;
  haptics::tools::OHMData res(header, v, desc);
  tools::OHMData::HapticElementMetadata element;
  tools::OHMData::HapticChannelMetadata channel;
  for (types::Perception p : perceptions) {
    element = tools::OHMData::HapticElementMetadata();
    element.elementDescription = p.getDescription();
    element.numHapticChannels = static_cast<short>(p.getChannelsSize());
    element.elementFilename = filename;
    element.channelsMetadata = {};
    for (int i = 0; i < element.numHapticChannels; i++) {
      channel = tools::OHMData::HapticChannelMetadata();
      types::Channel t = p.getChannelAt(i);
      channel.bodyPartMask = (tools::OHMData::Body)t.getBodyPartMaskOrDefault();
      channel.channelDescription = t.getDescription();
      channel.gain = t.getGain();

      element.channelsMetadata.push_back(channel);
    }

    res.addHapticElementMetadata(element);
  }
  return res;
}
auto Haptics::linearize() -> void {
  for (types::Perception &p : perceptions) {
    p.linearizeLibrary();
  }
}

auto Haptics::refactor() -> void {
  for (types::Perception &p : perceptions) {
    p.refactorEffects();
  }
}

auto Haptics::equalsImpl(const Haptics &haptic, bool compareSyncs) const -> bool {
  if (version != haptic.getVersion()) {
    std::cerr << "Version fields are different" << std::endl;
    return false;
  }
  if (profile != haptic.getProfile()) {
    std::cerr << "Profile fields are different" << std::endl;
    return false;
  }
  if (level != haptic.getLevel()) {
    std::cerr << "Level fields are different" << std::endl;
    return false;
  }
  if (date != haptic.getDate()) {
    std::cerr << "Date fields are different" << std::endl;
    return false;
  }
  if (description != haptic.getDescription()) {
    std::cerr << "Description fields are different" << std::endl;
    return false;
  }
  if (timescale != haptic.getTimescale()) {
    std::cerr << "Timescale fields are different" << std::endl;
    return false;
  }
  if (perceptions.size() != haptic.perceptions.size()) {
    std::cerr << "The number of perceptions is different" << std::endl;
    return false;
  }
  if (avatars.size() != haptic.avatars.size()) {
    std::cerr << "The number of avatars is different" << std::endl;
    return false;
  }
  if (compareSyncs && syncs.size() != haptic.syncs.size()) {
    std::cerr << "The number of Syncs is different" << std::endl;
    return false;
  }
  bool isEqual = true;
  if (isEqual) {
    for (int i = 0; i < static_cast<int>(perceptions.size()); i++) {
      const auto perception1 = perceptions.at(i);
      const auto perception2 = haptic.perceptions.at(i);
      isEqual = isEqual && perception1.equals(perception2);
    }
  }
  if (isEqual) {
    for (int i = 0; i < static_cast<int>(avatars.size()); i++) {
      const auto avatar1 = avatars.at(i);
      const auto avatar2 = haptic.avatars.at(i);
      isEqual = isEqual && avatar1.equals(avatar2);
    }
  }
  if (isEqual && compareSyncs) {
    for (int i = 0; i < static_cast<int>(syncs.size()); i++) {
      const auto sync1 = syncs.at(i);
      const auto sync2 = haptic.syncs.at(i);
      isEqual = isEqual && sync1.equals(sync2);
    }
  }
  return isEqual;
}

auto Haptics::equals(const Haptics &haptic) const -> bool { return equalsImpl(haptic, true); }

auto Haptics::equalsWithoutSyncs(const Haptics &haptic) const -> bool {
  return equalsImpl(haptic, false);
}

} // namespace haptics::types
