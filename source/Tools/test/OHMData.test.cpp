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

#include <catch2/catch.hpp>

#include <Tools/include/OHMData.h>
#include <filesystem>
#include <iostream>

using haptics::tools::OHMData;

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("haptics::tools::OHMData", "[loadFile][writeFile]") {
  OHMData ohmData;
  const std::string header = "OHM ";
  const std::string description = "Test description";
  const int version = 1;
  const std::string elementDescription = "Element description";
  const std::string elementFilename = "SomeHapticFile.wav";
  const std::string channelDescription = "Channel description";
  const OHMData::Body bodyPartMask = OHMData::Body::UNSPECIFIED;
  const float channelGain = 1.0;

  // Setting the data
  ohmData.setHeader(header);
  ohmData.setDescription(description);
  ohmData.setVersion(version);
  OHMData::HapticElementMetadata elementMetadata;
  elementMetadata.elementFilename = elementFilename;
  elementMetadata.elementDescription = elementDescription;
  elementMetadata.numHapticChannels = 1;
  OHMData::HapticChannelMetadata channelMetadata;
  channelMetadata.channelDescription = channelDescription;
  channelMetadata.gain = channelGain;
  channelMetadata.bodyPartMask = bodyPartMask;
  elementMetadata.channelsMetadata.push_back(channelMetadata);
  elementMetadata.numHapticChannels = static_cast<short>(elementMetadata.channelsMetadata.size());
  ohmData.addHapticElementMetadata(elementMetadata);

  std::string filename = "test.ohm";
  SECTION("Test writing/loading", "[writeFile]") {
    ohmData.writeFile(filename);
    CHECK(std::filesystem::is_regular_file(filename));
  }

  SECTION("Test loading", "[loadFile]") {
    OHMData ohmLoadedData(filename);
    CHECK(ohmLoadedData.getHeader() == header);
    CHECK(ohmLoadedData.getDescription() == description);
    CHECK(ohmLoadedData.getVersion() == version);
    bool validSize = ohmLoadedData.getHapticElementMetadataSize() == 1;
    REQUIRE(validSize);
    auto hapticElement = ohmLoadedData.getHapticElementMetadataAt(0);
    CHECK(hapticElement.elementFilename == elementFilename);
    CHECK(hapticElement.elementDescription == elementDescription);
    bool validNbTracks = ohmLoadedData.getHapticElementMetadataAt(0).numHapticChannels == 1;
    REQUIRE(validNbTracks);
    auto hapticChannel = hapticElement.channelsMetadata[0];
    CHECK(hapticChannel.channelDescription == channelDescription);
    CHECK(hapticChannel.gain == channelGain);
    CHECK(hapticChannel.bodyPartMask == bodyPartMask);
    std::filesystem::remove(filename);
    CHECK(!std::filesystem::is_regular_file(filename));
  }
}
