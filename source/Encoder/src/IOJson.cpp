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

#include <Encoder/include/IOJson.h>
#include <iostream>

using json = nlohmann::json;

namespace haptics::encoder {

auto IOJson::loadFile(const std::string &filePath) -> haptics::types::Haptics {
  std::ifstream ifs(filePath);
  json jsonTree = json::parse(ifs);
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
  haptics::types::Haptics haptic(version, date, description);
  loadPerceptions(jsonPerceptions, haptic);
  return haptic;
}


auto IOJson::loadPerceptions(const nlohmann::json& jsonPerceptions, types::Haptics& haptic)
-> void {

  for (auto it = jsonPerceptions.begin(); it != jsonPerceptions.end(); ++it) {
    auto jsonPerception = it.value();
    // TODO: add specific error messages
    if (!jsonPerception.contains("id") || !jsonPerception["id"].is_number_integer()) {
      continue;
    }
    if (!jsonPerception.contains("description") || !jsonPerception["description"].is_string()) {
      continue;
    }
    if (!jsonPerception.contains("tracks") || !jsonPerception["tracks"].is_array()) {
      continue;
    }

    auto perceptionId = jsonPerception["id"].get<int>();
    auto perceptionDescription = jsonPerception["description"].get<std::string>();

    haptics::types::Perception perception(perceptionId, perceptionDescription);
    auto jsonTracks = jsonPerception["tracks"];
    loadTracks(jsonTracks, perception);
    haptic.addPerception(perception);
  }
}
auto IOJson::loadTracks(const nlohmann::json& jsonTracks, types::Perception& perception) -> void {
  for (auto it = jsonTracks.begin(); it != jsonTracks.end(); ++it) {
    auto jsonTrack = it.value();
    // TODO: add specific error messages
    if (!jsonTrack.contains("id") || !jsonTrack["id"].is_number_integer()) {
      continue;
    }
    if (!jsonTrack.contains("description") || !jsonTrack["description"].is_string()) {
      continue;
    }
    if (!jsonTrack.contains("gain") || !jsonTrack["gain"].is_number()) {
      continue;
    }
    if (!jsonTrack.contains("mixing_weight") || !jsonTrack["mixing_weight"].is_number()) {
      continue;
    }
    if (!jsonTrack.contains("body_part_mask") || !jsonTrack["body_part_mask"].is_number_integer()) {
      continue;
    }
    if (!jsonTrack.contains("bands") || !jsonTrack["vertices"].is_array()) {
      continue;
    }

    auto trackId = jsonTrack["id"].get<int>();
    auto trackDescription = jsonTrack["description"].get<std::string>();
    auto trackGain = jsonTrack["gain"].get<float>();
    auto trackMixingWeight = jsonTrack["mixing_weight"].get<float>();
    auto trackBodyPart = jsonTrack["body_part_mask"].get<uint32_t>();

    types::Track track(trackId, trackDescription, trackGain, trackMixingWeight, trackBodyPart);

    if (jsonTrack.contains("vertices") && jsonTrack["vertices"].is_array()) {
      auto jsonVertices = jsonTrack["vertices"];
      for (auto itv = jsonVertices.begin(); itv != jsonVertices.end(); ++itv) {
        if (itv.value().is_number_integer()) {
          auto vertex = itv.value().get<int>();
          track.addVertex(vertex);
        }
      }
    }
    auto jsonBands = jsonTrack["bands"];
    loadBands(jsonBands, track);
    perception.addTrack(track);
  }
}
auto IOJson::loadBands(const nlohmann::json& jsonBands, types::Track& track) -> void {
  for (auto it = jsonBands.begin(); it != jsonBands.end(); ++it) {
    auto jsonBand = it.value();
    // TODO: add specific error messages
    if (!jsonBand.contains("band_type") || !jsonBand["band_type"].is_string()) {
      continue;
    }
    if (!jsonBand.contains("encoding_modality") || !jsonBand["encoding_modality"].is_string()) {
      continue;
    }
    if (!jsonBand.contains("window_length") || !jsonBand["window_length"].is_string()) {
      continue;
    }
    if (!jsonBand.contains("lower_frequency_limit") ||
        !jsonBand["lower_frequency_limit"].is_string()) {
      continue;
    }
    if (!jsonBand.contains("upper_frequency_limit") ||
        !jsonBand["upper_frequency_limit"].is_string()) {
      continue;
    }
    if (!jsonBand.contains("effects") || !jsonBand["effects"].is_array()) {
      continue;
    }

    types::BandType bandType = stringToBandType.at(jsonBand["band_type"].get<std::string>());
    types::EncodingModality encodingModality = stringToModality.at(jsonBand["encoding_modality"]);
    int windowLength = jsonBand["window_length"].get<int>();
    int lowerLimit = jsonBand["lower_frequency_limit"].get<int>();
    int upperLimit = jsonBand["upper_frequency_limit"].get<int>();

    types::Band band(bandType, encodingModality, windowLength, lowerLimit, upperLimit);
    auto jsonEffects = jsonBand["effects"];
    loadEffects(jsonEffects, band);

    track.addBand(band);
  }
}


auto IOJson::loadEffects(const nlohmann::json& jsonEffects, types::Band& band) -> void {
  for (auto it = jsonEffects.begin(); it != jsonEffects.end(); ++it) {
    auto jsonEffect = it.value();
    // TODO: add specific error messages
    if (!jsonEffect.contains("position") || !jsonEffect["position"].is_number_integer()) {
      continue;
    }
    if (!jsonEffect.contains("phase") || !jsonEffect["phase"].is_number()) {
      continue;
    }
    if (!jsonEffect.contains("base_signal") || !jsonEffect["base_signal"].is_string()) {
      continue;
    }
    if (!jsonEffect.contains("keyframes") || !jsonEffect["keyframes"].is_array()) {
      continue;
    }

    auto position = jsonEffect["position"].get<int>();
    auto phase = jsonEffect["phase"].get<float>();
    auto baseSignal = stringToBaseSignal.at(jsonEffect["base_signal"]);

    types::Effect effect(position, phase, baseSignal);
    auto jsonKeyframes = jsonEffect["keyframes"];
    loadKeyframes(jsonKeyframes, effect);

    band.addEffect(effect);
  }
}


auto IOJson::loadAvatars(const nlohmann::json& jsonAvatars, types::Haptics& haptic) -> void {
    //TODO
}
auto IOJson::loadReferenceDevices(const nlohmann::json& jsonAvatars, types::Perception& perception)
-> void {
    //TODO
}

auto IOJson::loadKeyframes(const nlohmann::json& jsonKeyframes, types::Effect& effect) -> void {
  for (auto it = jsonKeyframes.begin(); it != jsonKeyframes.end(); ++it) {
    auto jsonKeyframe = it.value();
    // TODO: add specific error messages
    if (!jsonKeyframe.contains("relative_position") || !jsonKeyframe["relative_position"].is_number_integer()) {
      continue;
    }
    auto relativePosition = jsonKeyframe["relative_position"].get<int>();
    std::optional<float> amplitudeModulation;
    std::optional<float> frequencyModulation;
    if (jsonKeyframe.contains("amplitude_modulation") && jsonKeyframe["amplitude_modulation"].is_number()) {
      amplitudeModulation = jsonKeyframe["amplitude_modulation"].get<float>();
    }
    if (jsonKeyframe.contains("frequency_modulation") &&
        jsonKeyframe["frequency_modulation"].is_number_integer()) {
      frequencyModulation = jsonKeyframe["frequency_modulation"].get<int>();
    }
    types::Keyframe keyframe(relativePosition, amplitudeModulation, frequencyModulation);
    effect.addKeyframe(keyframe);
  }
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

      auto jsonVertices = json::array();
      auto numVertices = track.getVerticesSize();
      for (int k = 0; k < numVertices; k++) {
          jsonVertices.push_back(track.getVertexAt(k));
      }
      if (numVertices > 0) {
        jsonTrack["vertices"] = jsonVertices;
      }

      auto jsonBands = json::array();
      auto numBands = track.getBandsSize();
      for (int l = 0; l < numBands; l++) {
        auto band = track.getBandAt(l);
        auto jsonBand = json::object();
        jsonBand["band_type"] = bandTypeToString.at(band.getBandType());
        jsonBand["encoding_modality"] = modalityToString.at(band.getEncodingModality());
        jsonBand["window_length"] = band.getWindowLength();
        jsonBand["lower_frequency_limit"] = band.getLowerFrequencyLimit();
        jsonBand["upper_frequency_limit"] = band.getUpperFrequencyLimit();

        auto jsonEffects = json::array();
        auto numEffects = band.getEffectsSize();
        for (int m = 0; m < numEffects; m++) {
          auto effect = band.getEffectAt(m);
          auto jsonEffect = json::object();
          jsonEffect["position"] = effect.getPosition();
          jsonEffect["phase"] = effect.getPhase();
          jsonEffect["base_signal"] = baseSignalToString.at(effect.getBaseSignal());

          
          auto jsonKeyframes = json::array();
          auto numKeyframes = effect.getKeyframesSize();
          for (int n = 0; n < numKeyframes; n++) {
            const auto &keyframe = effect.getKeyframeAt(n);
            auto jsonKeyframe = json::object();
            jsonKeyframe["relative_position"] = keyframe.getRelativePosition();
            if (keyframe.getAmplitudeModulation().has_value()) {
              jsonKeyframe["amplitude_modulation"] = keyframe.getAmplitudeModulation().value();
            }
            if (keyframe.getFrequencyModulation().has_value()) {
              jsonKeyframe["frequencyModulation"] = keyframe.getFrequencyModulation().value();
            }
            jsonKeyframes.push_back(jsonKeyframe);
          }
          jsonEffect["keyframes"] = jsonKeyframes;
          jsonEffects.push_back(jsonEffect);
        }
        jsonBand["effects"] = jsonEffects;
        jsonBands.push_back(jsonBand);
      }
      jsonTrack["bands"] = jsonBands;
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
