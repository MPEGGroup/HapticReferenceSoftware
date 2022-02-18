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
#include <catch2/catch.hpp>

TEST_CASE("haptics::types::Perception checking getters") {
  using haptics::types::Perception;
  using haptics::types::PerceptionModality;
  Perception perception(0, 0, "Some perception test content", PerceptionModality::Temperature);

  SECTION("Checking getId", "[getId]") { CHECK(perception.getId() == 0); }

  SECTION("Checking getDescription", "[getDescription]") {
    auto checkDescription = perception.getDescription();
    CHECK(checkDescription == "Some perception test content");
  }

  SECTION("Checking getAvatarId", "[getAvatarId]") { CHECK(perception.getAvatarId() == 0); }

  SECTION("Checking getPerceptionModality", "[getPerceptionModality]") {
    auto checkPerceptionModality = perception.getPerceptionModality();
    CHECK(checkPerceptionModality == PerceptionModality::Temperature);
  }
}

TEST_CASE("haptics::types::Perception checking setters") {
  using haptics::types::Perception;
  using haptics::types::PerceptionModality;
  Perception perception(0, 0, "Some perception test content", PerceptionModality::Temperature);

  SECTION("Checking setId", "[setId]") {
    perception.setId(2);
    CHECK(perception.getId() == 2);
  }

  SECTION("Checking setDescription", "[setDescription]") {
    std::string newDescription = "Some perception test content 2";
    perception.setDescription(newDescription);
    auto checkDescription = perception.getDescription();
    CHECK(checkDescription == newDescription);
  }

  SECTION("Checking setAvatarId", "[setAvatarId]") {
    perception.setAvatarId(1);
    CHECK(perception.getAvatarId() == 1);
  }

  SECTION("Checking setPerceptionModality", "[setPerceptionModality]") {
    perception.setPerceptionModality(PerceptionModality::Vibration);
    auto checkPerceptionModality = perception.getPerceptionModality();
    CHECK(checkPerceptionModality == PerceptionModality::Vibration);
  }
}

TEST_CASE("haptics::types::Perception testing tracks") {
  using haptics::types::Perception;
  using haptics::types::PerceptionModality;
  Perception perception(0, 0, "Some perception test content", PerceptionModality::Temperature);
  haptics::types::Track track(0, "Test track", 1, 1, 0);
  SECTION("Checking addTrack", "[addTrack]") {
    perception.addTrack(track);
    CHECK(perception.getTracksSize() == 1);
    haptics::types::Track addedTrack = perception.getTrackAt(0);
    CHECK(track.getId() == addedTrack.getId());
    CHECK(track.getDescription() == addedTrack.getDescription());
    CHECK(track.getGain() == Approx(addedTrack.getGain()));
    CHECK(track.getMixingWeight() ==  Approx(addedTrack.getMixingWeight()));
    CHECK(track.getBodyPartMask() == addedTrack.getBodyPartMask());
  }
}

TEST_CASE("haptics::types::Perception testing referenceDevice") {
  using haptics::types::Perception;
  using haptics::types::PerceptionModality;
  Perception perception(0, 0, "Some perception test content", PerceptionModality::Temperature);
  haptics::types::ReferenceDevice device(0, "Test Device");
  SECTION("Checking addReferenceDevice", "[addReferenceDevice]") {
    perception.addReferenceDevice(device);
    CHECK(perception.getReferenceDevicesSize() == 1);
    haptics::types::ReferenceDevice addedDevice = perception.getReferenceDeviceAt(0);
    bool sameDevice =
        (device.getId() == addedDevice.getId()) && (device.getName() == addedDevice.getName());
    CHECK(sameDevice);
  }
}
