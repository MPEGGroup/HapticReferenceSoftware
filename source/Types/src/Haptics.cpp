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
#include <ctime>

namespace haptics::types {

[[nodiscard]] auto Haptics::getVersion() const -> std::string { return version; }

auto Haptics::setVersion(std::string &newVersion) -> void { version = newVersion; }

[[nodiscard]] auto Haptics::getDate() const -> std::string { return date; }
auto Haptics::setDate(std::string &newDate) -> void { date = newDate; }

[[nodiscard]] auto Haptics::getDescription() const -> std::string { return description; }

auto Haptics::setDescription(std::string &newDescription) -> void { description = newDescription; }

auto Haptics::getPerceptionsSize() -> size_t { return perceptions.size(); }

auto Haptics::getPerceptionAt(int index) -> Perception & { return perceptions.at(index); }

auto Haptics::replacePerceptionAt(int index, Perception &newPerception) -> bool {
  if (index < 0 || index >= (int)perceptions.size()) {
    return false;
  }

  perceptions[index] = newPerception;
  return true;
}

auto Haptics::addPerception(Perception &newPerception) -> void {
  perceptions.push_back(newPerception);
}

auto Haptics::getAvatarsSize() -> size_t { return avatars.size(); }

auto Haptics::getAvatarAt(int index) -> Avatar & { return avatars.at(index); }

auto Haptics::addAvatar(Avatar &newAvatar) -> void { avatars.push_back(newAvatar); }

auto Haptics::loadMetadataFromOHM(haptics::tools::OHMData data) -> void {
  version = std::to_string(data.getVersion());
  time_t now = time(nullptr);
  date = ctime(&now);
  description = data.getDescription();
  auto numElements = static_cast<int>(data.getHapticElementMetadataSize());
  for (int i = 0; i < numElements; i++) {
    auto element = data.getHapticElementMetadataAt(i);
    std::string elemDescription = element.elementDescription;
    PerceptionModality perceptionModality = Perception::convertToModality(elemDescription);
    Perception perception(i, 0, elemDescription, perceptionModality);
    short numChannels = element.numHapticChannels;
    for (int j = 0; j < numChannels; j++) {
      auto channel = element.channelsMetadata[j];
      Track track(j, channel.channelDescription, channel.gain, 1,
                  static_cast<uint32_t>(channel.bodyPartMask));
      perception.addTrack(track);
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
    element.numHapticChannels = static_cast<short>(p.getTracksSize());
    element.elementFilename = filename;
    element.channelsMetadata = {};
    for (int i = 0; i < element.numHapticChannels; i++) {
      channel = tools::OHMData::HapticChannelMetadata();
      types::Track t = p.getTrackAt(i);
      channel.bodyPartMask = (tools::OHMData::Body)t.getBodyPartMask();
      channel.channelDescription = t.getDescription();
      channel.gain = t.getGain();

      element.channelsMetadata.push_back(channel);
    }

    res.addHapticElementMetadata(element);
  }
  return res;
}

} // namespace haptics::types
