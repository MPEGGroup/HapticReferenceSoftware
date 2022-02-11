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

#include <filesystem>
#include <Types/include/Haptics.h>
#include <catch2/catch.hpp>
#include <Tools/include/OHMData.h>

using haptics::types::Haptics;

TEST_CASE("haptics::types::Haptics") {

  Haptics h("1", "02/04/2022", "test content");
  haptics::types::Avatar avatar(0, 0, haptics::types::AvatarType::Vibration);
  haptics::types::Perception perception(0,0,"Vibration effect", haptics::types::PerceptionModality::Vibration);

  SECTION("Checking avatars", "[addAvatar]") {
    h.addAvatar(avatar);
    CHECK(h.getAvatarsSize() == 1);
    auto addedAvatar = h.getAvatarAt(0);
    bool sameAvatar = (addedAvatar.getId() == avatar.getId()) &&
                      (addedAvatar.getLod() == avatar.getLod()) &&
                      (addedAvatar.getType() == avatar.getType());
    CHECK(sameAvatar);
  }

  SECTION("Checking perception", "[addPerception]") {
    h.addPerception(perception);
    CHECK(h.getPerceptionsSize() == 1);
    auto addedPerception = h.getPerceptionAt(0);
    bool samePerception = (addedPerception.getId() == perception.getId()) &&
        (addedPerception.getPerceptionModality() == perception.getPerceptionModality()) &&
        (addedPerception.getAvatarId() == perception.getAvatarId()) &&
        (addedPerception.getDescription() == perception.getDescription());
    CHECK(samePerception);
  }
}
  
TEST_CASE("haptics::types::Haptics loading ohm file", "[loadMetadataFromOHM]") {

  SECTION("Test loading from OHM") {
    std::string filepath =
        std::filesystem::current_path().string() + "/../../../../../../data/test.ohm";
    haptics::tools::OHMData ohmData(filepath);

    Haptics h;
    h.loadMetadataFromOHM(ohmData);
    bool validDescription = h.getDescription() == "pantheon grand starfall";
    bool validVersion = h.getVersion() == "1";
    bool validSize = h.getPerceptionsSize() == 1;
    bool validElmDescription = validSize &&
        (h.getPerceptionAt(0).getDescription() == "Vibration effect");
    bool validNbTracks =
        validSize && (h.getPerceptionAt(0).getTracksSize() == 1);
    bool validChannelDescription =
        validSize && validNbTracks &&
        (h.getPerceptionAt(0).getTrackAt(0).getDescription() == "Full body");
    bool validGain = validSize && validNbTracks && (h.getPerceptionAt(0).getTrackAt(0).getGain() == 1);
    bool validBodyPart = validSize && validNbTracks && (h.getPerceptionAt(0).getTrackAt(0).getBodyPartMask() == static_cast<uint32_t> (haptics::tools::OHMData::Body::ALL));
    bool validFile = validDescription && validVersion && validSize &&
                     validElmDescription && validNbTracks &&
                     validChannelDescription && validGain && validBodyPart;
    CHECK(validFile);
  }
}