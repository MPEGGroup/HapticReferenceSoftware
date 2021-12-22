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

#include "../include/IOJson.h"
#include <iostream>

using json = nlohmann::json;

namespace haptics::encoder {

auto IOJson::loadFile(const std::string &filePath) -> haptics::types::Haptics {
  std::ifstream ifs(filePath);
  json jsonTree = nlohmann::json::parse(ifs);
  if (!(jsonTree.contains("version") && jsonTree.contains("date") &&
        jsonTree.contains("description") && jsonTree.contains("perceptions"))) {
      //TODO invalid input file
  }
  auto version = jsonTree["version"].get<std::string>();
  auto date = jsonTree["date"].get<std::string>();
  auto description = jsonTree["description"].get<std::string>();

  if (!(jsonTree.contains("perceptions") && jsonTree["perceptions"].is_array())) {
    //TODO invalid json file
  }
  auto jsonPerceptions = jsonTree["perceptions"];
  std::vector<haptics::types::Perception> perceptions;
  for (auto itp = jsonPerceptions.begin(); itp != jsonPerceptions.end(); ++itp) {
    auto jsonPerception = itp.value();

    // id is a required field, if missing or incorrect, the perception is skipped
    if (!jsonPerception.contains("id") || !jsonPerception["id"].is_number_integer()) {
      continue;
    }
    auto perceptionId = jsonPerception["id"].get<int>();

    // description is a required field, if missing or incorrect, the perception is skipped
    if (!jsonPerception.contains("description") || !jsonPerception["description"].is_string()) {
      continue;
    }
    auto perceptionDescription = jsonPerception["description"].get<std::string>();

    // tracks is a required field, if missing or incorrect, the perception is skipped
    if (!jsonPerception.contains("tracks") || !jsonPerception["tracks"].is_array()) {
      continue;
    }
    auto jsonTracks = jsonPerception["tracks"];
    std::vector<haptics::types::Track> tracks;
    for (auto itt = jsonTracks.begin(); itt != jsonTracks.end(); ++itt) {
        auto jsonTrack = itt.value();
        // id is a required field, if missing or incorrect, the track is skipped
        if (!jsonTrack.contains("id") || !jsonTrack["id"].is_number_integer()) {
            continue;
        }
        auto trackId = jsonTrack["id"].get<int>();

        // description is a required field, if missing or incorrect, the track is skipped
        if (!jsonTrack.contains("description") || !jsonTrack["description"].is_string()) {
          continue;
        }
        auto trackDescription = jsonTrack["description"].get<std::string>();

        // gain is a required field, if missing or incorrect, the track is skipped
        if (!jsonTrack.contains("gain") || !jsonTrack["gain"].is_number()) {
          continue;
        }
        auto trackGain = jsonTrack["gain"].get<float>();

        // mixing_weight is a required field, if missing or incorrect, the track is skipped
        if (!jsonTrack.contains("mixing_weight") || !jsonTrack["mixing_weight"].is_number()) {
          continue;
        }
        auto trackMixingWeight = jsonTrack["mixing_weight"].get<float>();

        // body_part_mask is a required field, if missing or incorrect, the track is skipped
        if (!jsonTrack.contains("body_part_mask") || !jsonTrack["body_part_mask"].is_number_integer()) {
          continue;
        }
        auto trackBodyPart = jsonTrack["body_part_mask"].get<uint32_t>();

        haptics::types::Track track(trackId, trackDescription, trackGain, trackMixingWeight, trackBodyPart);
        tracks.push_back(track);
      
    }
    haptics::types::Perception perception(perceptionId, perceptionDescription, tracks);
    perceptions.push_back(perception);

  }

  haptics::types::Haptics haptic(version, date, description, perceptions);

  return haptic;
}

auto IOJson::writeFile(haptics::types::Haptics &haptic, const std::string &filePath) -> void {
  nlohmann::json jsonTree;
  jsonTree["version"] = haptic.getVersion();
  jsonTree["description"] = haptic.getDescription();
  jsonTree["date"] = haptic.getDate();

  auto jsonPerceptions = json::array();  
  auto numPerceptions = haptic.getPerceptionsSize();
  for (int i = 0; i < numPerceptions; i++) {
    haptics::types::Perception perception = haptic.getPerceptionAt(i);
    auto jsonPerception = json::object();
    jsonPerception["id"] = perception.getId();
    jsonPerception["description"] = perception.getDescription();
    
    auto jsonTracks = json::array();
    auto numTracks = perception.getTracksSize();
    for (int j = 0; j < numTracks; j++) {
      haptics::types::Track track = perception.getTrackAt(j);
      auto jsonTrack = json::object();
      jsonTrack["id"] = track.getId();
      jsonTrack["description"] = track.getDescription();
      jsonTrack["gain"] = track.getGain();
      jsonTrack["mixing_weight"] = track.getMixingWeight();
      jsonTrack["body_part_mask"] = track.getBodyPartMask();
      //TODO vertices
      //TODO bands
      jsonTracks.push_back(jsonTrack);
    }
    jsonPerception["tracks"] = jsonTracks;
    jsonPerceptions.push_back(jsonPerception);
  }
  jsonTree["perceptions"] = jsonPerceptions;

  std::ofstream file(filePath);
  file << jsonTree;


}

} // namespace haptics::encoder
