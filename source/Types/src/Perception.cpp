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

} // namespace haptics::types