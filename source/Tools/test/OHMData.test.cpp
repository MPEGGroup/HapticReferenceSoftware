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

TEST_CASE("haptics::tools::OHMData") {

  std::string filepath = std::filesystem::current_path().string() + "/../../../../../../data/test.ohm";
  OHMData ohmData(filepath);

  SECTION("Test loading", "[loadFile]") { 
      bool validHeader = ohmData.getHeader() == "OHM ";
      bool validDescription = ohmData.getDescription() == "pantheon grand starfall";
      bool validVersion = ohmData.getVersion() == 1;
      bool validSize = ohmData.getHapticElementMetadataSize() == 1;
      bool validFileName = validSize && (ohmData.getHapticElementMetadataAt(0).elementFilename == "ACTK-vib-pantheongrandstarfall-8kHz-16-nopad.wav");
      bool validElmDescription = validSize && (ohmData.getHapticElementMetadataAt(0).elementDescription == "Vibration effect");
      bool validNbTracks = validSize && (ohmData.getHapticElementMetadataAt(0).numHapticChannels == 1);
      bool validChannelDescription = validSize && validNbTracks && (ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].channelDescription == "Full body");
      bool validGain = validSize && validNbTracks && (ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].gain == 1);
      bool validBodyPart = validSize && validNbTracks && (ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].bodyPartMask == OHMData::Body::ALL);
      bool validFile = validHeader && validDescription && validVersion && validSize &&
                       validFileName && validElmDescription && validNbTracks && 
                       validChannelDescription && validGain && validBodyPart;
      CHECK(validFile);
  }

  
  std::string filepath2 = std::filesystem::current_path().string() + "/../../../../../../data/tests/test2.ohm";

  SECTION("Test writing", "[writeFile]") {
    ohmData.writeFile(filepath2);
    CHECK(std::filesystem::is_regular_file(filepath2));
    OHMData ohmData2(filepath2);
    bool validHeader = ohmData2.getHeader() == "OHM ";
    bool validDescription = ohmData2.getDescription() == ohmData.getDescription();
    bool validVersion = ohmData2.getVersion() == ohmData.getVersion();
    bool validSize = ohmData2.getHapticElementMetadataSize() == ohmData.getHapticElementMetadataSize();
    bool validFileName = validSize && (ohmData2.getHapticElementMetadataAt(0).elementFilename == ohmData.getHapticElementMetadataAt(0).elementFilename);
    bool validElmDescription = validSize && (ohmData2.getHapticElementMetadataAt(0).elementDescription == ohmData.getHapticElementMetadataAt(0).elementDescription);
    bool validNbTracks = validSize && (ohmData2.getHapticElementMetadataAt(0).numHapticChannels == ohmData.getHapticElementMetadataAt(0).numHapticChannels);
    bool validChannelDescription = validSize && validNbTracks && (ohmData2.getHapticElementMetadataAt(0).channelsMetadata[0].channelDescription == ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].channelDescription);
    bool validGain = validSize && validNbTracks &&(ohmData2.getHapticElementMetadataAt(0).channelsMetadata[0].gain == ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].gain );
    bool validBodyPart = validSize && validNbTracks && (ohmData2.getHapticElementMetadataAt(0).channelsMetadata[0].bodyPartMask == ohmData.getHapticElementMetadataAt(0).channelsMetadata[0].bodyPartMask);
    bool validFile = validHeader && validDescription && validVersion && validSize &&
                     validFileName && validElmDescription && validNbTracks &&
                     validChannelDescription && validGain && validBodyPart;
    CHECK(validFile);
  }
}
