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

#include <IOHaptics/include/IOJson.h>
#include <iostream>
#include <limits>
#include <Tools/include/Tools.h>

using json = nlohmann::json;

namespace haptics::io {

auto IOJson::loadFile(const std::string &filePath, types::Haptics &haptic) -> bool {
  bool loadingSuccess = true;
  std::ifstream ifs(filePath);
  json jsonTree = json::parse(ifs);
  if (!(jsonTree.contains("version") && jsonTree.contains("date") &&
        jsonTree.contains("description") && jsonTree.contains("perceptions") &&
        jsonTree["perceptions"].is_array() && jsonTree.contains("avatars") &&
        jsonTree["avatars"].is_array())) {
    std::cerr << "Invalid GMPG input file: missing required field" << std::endl;
    return false;
  }
  auto version = jsonTree["version"].get<std::string>();
  auto date = jsonTree["date"].get<std::string>();
  auto description = jsonTree["description"].get<std::string>();
  haptic.setVersion(version);
  haptic.setDate(date);
  haptic.setDescription(description);
  auto jsonAvatars = jsonTree["avatars"];
  loadingSuccess = loadingSuccess && loadAvatars(jsonAvatars, haptic);
  auto jsonPerceptions = jsonTree["perceptions"];
  loadingSuccess = loadingSuccess && loadPerceptions(jsonPerceptions, haptic);
  return loadingSuccess;
}

auto IOJson::loadPerceptions(const nlohmann::json &jsonPerceptions, types::Haptics &haptic)
    -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonPerceptions.begin(); it != jsonPerceptions.end(); ++it) {
    auto jsonPerception = it.value();
    if (!jsonPerception.contains("id") || !jsonPerception["id"].is_number_integer()) {
      std::cerr << "Missing or invalid perception id" << std::endl;
      continue;
    }
    if (!jsonPerception.contains("avatar_id") || !jsonPerception["avatar_id"].is_number_integer()) {
      std::cerr << "Missing or invalid perception avatar id" << std::endl;
      continue;
    }
    if (!jsonPerception.contains("description") || !jsonPerception["description"].is_string()) {
      std::cerr << "Missing or invalid perception description" << std::endl;
      continue;
    }
    if (!jsonPerception.contains("perception_modality") ||
        !jsonPerception["perception_modality"].is_string()) {
      std::cerr << "Missing or invalid perception modality" << std::endl;
      continue;
    }
    if (!jsonPerception.contains("tracks") || !jsonPerception["tracks"].is_array()) {
      std::cerr << "Missing or invalid tracks" << std::endl;
      continue;
    }

    auto perceptionId = jsonPerception["id"].get<int>();
    auto perceptionAvatarId = jsonPerception["avatar_id"].get<int>();
    auto perceptionDescription = jsonPerception["description"].get<std::string>();
    auto perceptionPerceptionModality = types::stringToPerceptionModality.at(
        jsonPerception["perception_modality"].get<std::string>());

    haptics::types::Perception perception(perceptionId, perceptionAvatarId, perceptionDescription,
                                          perceptionPerceptionModality);
    auto jsonTracks = jsonPerception["tracks"];
    loadingSuccess = loadingSuccess && loadTracks(jsonTracks, perception);
    if (jsonPerception.contains("reference_devices") &&
        jsonPerception["reference_devices"].is_array()) {
      auto jsonReferenceDevices = jsonPerception["reference_devices"];
      loadingSuccess = loadingSuccess && loadReferenceDevices(jsonReferenceDevices, perception);
    }
    haptic.addPerception(perception);
  }
  return loadingSuccess;
}
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto IOJson::loadTracks(const nlohmann::json &jsonTracks, types::Perception &perception) -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonTracks.begin(); it != jsonTracks.end(); ++it) {
    auto jsonTrack = it.value();
    if (!jsonTrack.contains("id") || !jsonTrack["id"].is_number_integer()) {
      std::cerr << "Missing or invalid track id" << std::endl;
      continue;
    }
    if (!jsonTrack.contains("description") || !jsonTrack["description"].is_string()) {
      std::cerr << "Missing or invalid track description" << std::endl;
      continue;
    }
    if (!jsonTrack.contains("gain") || !jsonTrack["gain"].is_number()) {
      std::cerr << "Missing or invalid track gain" << std::endl;
      continue;
    }
    if (!jsonTrack.contains("mixing_weight") || !jsonTrack["mixing_weight"].is_number()) {
      std::cerr << "Missing or invalid track mixing weight" << std::endl;
      continue;
    }
    if (!jsonTrack.contains("body_part_mask") || !jsonTrack["body_part_mask"].is_number_integer()) {
      std::cerr << "Missing or invalid track body part mask" << std::endl;
      continue;
    }
    if (!jsonTrack.contains("bands") || !jsonTrack["bands"].is_array()) {
      std::cerr << "Missing or invalid bands" << std::endl;
      continue;
    }

    auto trackId = jsonTrack["id"].get<int>();
    auto trackDescription = jsonTrack["description"].get<std::string>();
    auto trackGain = jsonTrack["gain"].get<float>();
    auto trackMixingWeight = jsonTrack["mixing_weight"].get<float>();
    auto trackBodyPart = jsonTrack["body_part_mask"].get<uint32_t>();
    std::optional<types::Direction> direction;
    const int axisCount = 3;
    if (jsonTrack.contains("direction") && jsonTrack["direction"].is_array() &&
        jsonTrack["direction"].size() == axisCount) {
      bool axisAreCorrect = true;
      for (auto directionit = jsonTrack["direction"].begin();
           directionit != jsonTrack["direction"].end(); ++directionit) {
        axisAreCorrect &= directionit.value().is_number();
      }
      if (axisAreCorrect) {
        auto _x = jsonTrack["direction"][0].get<double>();
        auto _y = jsonTrack["direction"][1].get<double>();
        auto _z = jsonTrack["direction"][2].get<double>();
        double directionMagnitude = std::sqrt(_x * _x + _y * _y + _z * _z);

        auto x = static_cast<int8_t>(tools::genericNormalization(
            -directionMagnitude, directionMagnitude, std::numeric_limits<int8_t>::min(),
            std::numeric_limits<int8_t>::max(), _x));
        auto y = static_cast<int8_t>(tools::genericNormalization(
            -directionMagnitude, directionMagnitude, std::numeric_limits<int8_t>::min(),
            std::numeric_limits<int8_t>::max(), _y));
        auto z = static_cast<int8_t>(tools::genericNormalization(
            -directionMagnitude, directionMagnitude, std::numeric_limits<int8_t>::min(),
            std::numeric_limits<int8_t>::max(), _z));
        direction = types::Direction(x, y, z);
      } else {
        direction = std::nullopt;
      }
    } else {
      direction = std::nullopt;
    }
    std::optional<int8_t> unitLength;
    if (jsonTrack.contains("unit_length") && jsonTrack["unit_length"].is_number_integer()) {
      unitLength = jsonTrack["unit_length"].get<int8_t>();
    } else {
      unitLength = std::nullopt; 
    }

    types::Track track(trackId, trackDescription, trackGain, trackMixingWeight, trackBodyPart,
                       direction, unitLength);

    if (jsonTrack.contains("reference_device_id") && jsonTrack["reference_device_id"].is_number()) {
      auto device_id = jsonTrack["reference_device_id"].get<int>();
      track.setReferenceDeviceId(device_id);
    }
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
    loadingSuccess = loadingSuccess && loadBands(jsonBands, track);
    perception.addTrack(track);
  }
  return loadingSuccess;
}
auto IOJson::loadBands(const nlohmann::json &jsonBands, types::Track &track) -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonBands.begin(); it != jsonBands.end(); ++it) {
    auto jsonBand = it.value();
    if (!jsonBand.contains("band_type") || !jsonBand["band_type"].is_string()) {
      std::cerr << "Missing or invalid band type" << std::endl;
      continue;
    }
    if (!jsonBand.contains("curve_type") || !jsonBand["curve_type"].is_string()) {
      std::cerr << "Missing or invalid curve type" << std::endl;
      continue;
    }
    if (!jsonBand.contains("encoding_modality") || !jsonBand["encoding_modality"].is_string()) {
      std::cerr << "Missing or invalid encoding modality" << std::endl;
      continue;
    }
    if (!jsonBand.contains("window_length") || !jsonBand["window_length"].is_number_integer()) {
      std::cerr << "Missing or invalid window length" << std::endl;
      continue;
    }
    if (!jsonBand.contains("lower_frequency_limit") ||
        !jsonBand["lower_frequency_limit"].is_number_integer()) {
      std::cerr << "Missing or invalid lower frequency limit" << std::endl;
      continue;
    }
    if (!jsonBand.contains("upper_frequency_limit") ||
        !jsonBand["upper_frequency_limit"].is_number_integer()) {
      std::cerr << "Missing or invalid upper frequency limit" << std::endl;
      continue;
    }
    if (!jsonBand.contains("effects") || !jsonBand["effects"].is_array()) {
      std::cerr << "Missing or invalid list of effects" << std::endl;
      continue;
    }

    types::BandType bandType = types::stringToBandType.at(jsonBand["band_type"].get<std::string>());
    types::CurveType curveType =
        types::stringToCurveType.at(jsonBand["curve_type"].get<std::string>());
    types::EncodingModality encodingModality =
        types::stringToModality.at(jsonBand["encoding_modality"]);
    int windowLength = jsonBand["window_length"].get<int>();
    int lowerLimit = jsonBand["lower_frequency_limit"].get<int>();
    int upperLimit = jsonBand["upper_frequency_limit"].get<int>();

    types::Band band(bandType, curveType, encodingModality, windowLength, lowerLimit, upperLimit);
    auto jsonEffects = jsonBand["effects"];
    loadingSuccess = loadingSuccess && loadEffects(jsonEffects, band);

    track.addBand(band);
  }
  return loadingSuccess;
}

auto IOJson::loadEffects(const nlohmann::json &jsonEffects, types::Band &band) -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonEffects.begin(); it != jsonEffects.end(); ++it) {
    auto jsonEffect = it.value();
    if (!jsonEffect.contains("position") || !jsonEffect["position"].is_number_integer()) {
      std::cerr << "Missing or invalid effect position" << std::endl;
      continue;
    }
    if (!jsonEffect.contains("phase") || !jsonEffect["phase"].is_number()) {
      std::cerr << "Missing or invalid effect phase" << std::endl;
      continue;
    }
    if (!jsonEffect.contains("base_signal") || !jsonEffect["base_signal"].is_string()) {
      std::cerr << "Missing or invalid effect base_signal" << std::endl;
      continue;
    }
    if (!jsonEffect.contains("keyframes") || !jsonEffect["keyframes"].is_array()) {
      std::cerr << "Missing or invalid list of keyframes" << std::endl;
      continue;
    }

    auto position = jsonEffect["position"].get<int>();
    auto phase = jsonEffect["phase"].get<float>();
    auto baseSignal = types::stringToBaseSignal.at(jsonEffect["base_signal"]);

    types::Effect effect(position, phase, baseSignal);
    auto jsonKeyframes = jsonEffect["keyframes"];
    loadingSuccess = loadingSuccess && loadKeyframes(jsonKeyframes, effect);

    band.addEffect(effect);
  }
  return loadingSuccess;
}

auto IOJson::loadAvatars(const nlohmann::json &jsonAvatars, types::Haptics &haptic) -> bool {
  for (auto it = jsonAvatars.begin(); it != jsonAvatars.end(); ++it) {
    auto jsonAvatar = it.value();

    if (!jsonAvatar.contains("id") || !jsonAvatar["id"].is_number_integer()) {
      std::cerr << "Missing or invalid avatar id" << std::endl;
      continue;
    }
    if (!jsonAvatar.contains("lod") || !jsonAvatar["lod"].is_number_integer()) {
      std::cerr << "Missing or invalid avatar lod" << std::endl;
      continue;
    }
    if (!jsonAvatar.contains("type") || !jsonAvatar["type"].is_string()) {
      std::cerr << "Missing or invalid avatar type" << std::endl;
      continue;
    }
    auto id = jsonAvatar["id"].get<int>();
    auto lod = jsonAvatar["lod"].get<int>();
    auto type = types::stringToAvatarType.at(jsonAvatar["type"]);

    types::Avatar avatar(id, lod, type);

    if (jsonAvatar.contains("mesh") && jsonAvatar["mesh"].is_string()) {
      avatar.setMesh(jsonAvatar["mesh"]);
    }
    haptic.addAvatar(avatar);
  }
  return true;
}
auto IOJson::loadReferenceDevices(const nlohmann::json &jsonReferenceDevices,
                                  types::Perception &perception) -> bool {
  for (auto it = jsonReferenceDevices.begin(); it != jsonReferenceDevices.end(); ++it) {
    auto jsonReferenceDevice = it.value();

    if (!jsonReferenceDevice.contains("id") || !jsonReferenceDevice["id"].is_number_integer()) {
      std::cerr << "Missing or invalid reference device id" << std::endl;
      continue;
    }
    if (!jsonReferenceDevice.contains("name") || !jsonReferenceDevice["name"].is_string()) {
      std::cerr << "Missing or invalid reference device name" << std::endl;
      continue;
    }
    auto id = jsonReferenceDevice["id"].get<int>();
    auto name = jsonReferenceDevice["name"].get<std::string>();

    types::ReferenceDevice referenceDevice(id, name);
    if (jsonReferenceDevice.contains("body_part_mask") &&
        jsonReferenceDevice["body_part_mask"].is_number_integer()) {
      referenceDevice.setBodyPartMask(jsonReferenceDevice["body_part_mask"].get<uint32_t>());
    }
    if (jsonReferenceDevice.contains("maximum_frequency") &&
        jsonReferenceDevice["maximum_frequency"].is_number()) {
      referenceDevice.setMaximumFrequency(jsonReferenceDevice["maximum_frequency"].get<float>());
    }
    if (jsonReferenceDevice.contains("minimum_frequency") &&
        jsonReferenceDevice["minimum_frequency"].is_number()) {
      referenceDevice.setMinimumFrequency(jsonReferenceDevice["minimum_frequency"].get<float>());
    }
    if (jsonReferenceDevice.contains("resonance_frequency") &&
        jsonReferenceDevice["resonance_frequency"].is_number()) {
      referenceDevice.setResonanceFrequency(
          jsonReferenceDevice["resonance_frequency"].get<float>());
    }
    if (jsonReferenceDevice.contains("maximum_amplitude") &&
        jsonReferenceDevice["maximum_amplitude"].is_number()) {
      referenceDevice.setMaximumAmplitude(jsonReferenceDevice["maximum_amplitude"].get<float>());
    }
    if (jsonReferenceDevice.contains("impedance") && jsonReferenceDevice["impedance"].is_number()) {
      referenceDevice.setImpedance(jsonReferenceDevice["impedance"].get<float>());
    }
    if (jsonReferenceDevice.contains("maximum_voltage") &&
        jsonReferenceDevice["maximum_voltage"].is_number()) {
      referenceDevice.setMaximumVoltage(jsonReferenceDevice["maximum_voltage"].get<float>());
    }
    if (jsonReferenceDevice.contains("maximum_current") &&
        jsonReferenceDevice["maximum_current"].is_number()) {
      referenceDevice.setMaximumCurrent(jsonReferenceDevice["maximum_current"].get<float>());
    }
    if (jsonReferenceDevice.contains("maximum_displacement") &&
        jsonReferenceDevice["maximum_displacement"].is_number()) {
      referenceDevice.setMaximumDisplacement(
          jsonReferenceDevice["maximum_displacement"].get<float>());
    }
    if (jsonReferenceDevice.contains("weight") && jsonReferenceDevice["weight"].is_number()) {
      referenceDevice.setWeight(jsonReferenceDevice["weight"].get<float>());
    }
    if (jsonReferenceDevice.contains("size") && jsonReferenceDevice["size"].is_number()) {
      referenceDevice.setSize(jsonReferenceDevice["size"].get<float>());
    }
    if (jsonReferenceDevice.contains("custom") && jsonReferenceDevice["custom"].is_number()) {
      referenceDevice.setCustom(jsonReferenceDevice["custom"].get<float>());
    }
    if (jsonReferenceDevice.contains("type") && jsonReferenceDevice["type"].is_string()) {
      auto type = jsonReferenceDevice["type"].get<std::string>();
      referenceDevice.setType(types::stringToActuatorType.at(type));
    }
    perception.addReferenceDevice(referenceDevice);
  }
  return true;
}

auto IOJson::loadKeyframes(const nlohmann::json &jsonKeyframes, types::Effect &effect) -> bool {
  for (auto it = jsonKeyframes.begin(); it != jsonKeyframes.end(); ++it) {
    auto jsonKeyframe = it.value();
    std::optional<int> relativePosition;
    std::optional<float> amplitudeModulation;
    std::optional<float> frequencyModulation;
    if (jsonKeyframe.contains("relative_position") &&
        jsonKeyframe["relative_position"].is_number_integer()) {
      relativePosition = jsonKeyframe["relative_position"].get<int>();
    }
    if (jsonKeyframe.contains("amplitude_modulation") &&
        jsonKeyframe["amplitude_modulation"].is_number()) {
      amplitudeModulation = jsonKeyframe["amplitude_modulation"].get<float>();
    }
    if (jsonKeyframe.contains("frequency_modulation") &&
        jsonKeyframe["frequency_modulation"].is_number_integer()) {
      frequencyModulation = jsonKeyframe["frequency_modulation"].get<int>();
    }
    types::Keyframe keyframe(relativePosition, amplitudeModulation, frequencyModulation);
    effect.addKeyframe(keyframe);
  }
  return true;
}

auto IOJson::writeFile(haptics::types::Haptics &haptic, const std::string &filePath) -> void {
  nlohmann::json jsonTree;
  jsonTree["version"] = haptic.getVersion();
  jsonTree["description"] = haptic.getDescription();
  jsonTree["date"] = haptic.getDate();
  auto jsonAvatars = json::array();
  extractAvatars(haptic, jsonAvatars);
  jsonTree["avatars"] = jsonAvatars;
  auto jsonPerceptions = json::array();
  extractPerceptions(haptic, jsonPerceptions);
  jsonTree["perceptions"] = jsonPerceptions;

  std::ofstream file(filePath);
  file << jsonTree;
}
auto IOJson::extractPerceptions(types::Haptics &haptic, nlohmann::json &jsonPerceptions) -> void {
  auto numPerceptions = haptic.getPerceptionsSize();
  for (uint32_t i = 0; i < numPerceptions; i++) {
    auto perception = haptic.getPerceptionAt((int)i);
    auto jsonPerception = json::object();
    jsonPerception["id"] = perception.getId();
    jsonPerception["avatar_id"] = perception.getAvatarId();
    jsonPerception["description"] = perception.getDescription();
    jsonPerception["perception_modality"] =
        types::perceptionModalityToString.at(perception.getPerceptionModality());

    auto jsonReferenceDevices = json::array();
    extractReferenceDevices(perception, jsonReferenceDevices);
    jsonPerception["reference_devices"] = jsonReferenceDevices;

    auto jsonTracks = json::array();
    extractTracks(perception, jsonTracks);
    jsonPerception["tracks"] = jsonTracks;

    jsonPerceptions.push_back(jsonPerception);
  }
}

auto IOJson::extractAvatars(types::Haptics &haptic, nlohmann::json &jsonAvatars) -> void {
  auto numAvatars = haptic.getAvatarsSize();
  for (uint32_t i = 0; i < numAvatars; i++) {
    auto avatar = haptic.getAvatarAt((int)i);
    auto jsonAvatar = json::object();
    jsonAvatar["id"] = avatar.getId();
    jsonAvatar["lod"] = avatar.getLod();
    jsonAvatar["type"] = types::avatarTypeToString.at(avatar.getType());
    if (avatar.getMesh().has_value() && avatar.getType() == haptics::types::AvatarType::Custom) {
      jsonAvatar["mesh"] = avatar.getMesh().value();
    }
    jsonAvatars.push_back(jsonAvatar);
  }
}
auto IOJson::extractTracks(types::Perception &perception, nlohmann::json &jsonTracks) -> void {
  auto numTracks = perception.getTracksSize();
  for (uint32_t j = 0; j < numTracks; j++) {
    haptics::types::Track track = perception.getTrackAt((int)j);
    auto jsonTrack = json::object();
    jsonTrack["id"] = track.getId();
    jsonTrack["description"] = track.getDescription();
    jsonTrack["gain"] = track.getGain();
    jsonTrack["mixing_weight"] = track.getMixingWeight();
    jsonTrack["body_part_mask"] = track.getBodyPartMask();
    if (track.getReferenceDeviceId().has_value()) {
      jsonTrack["reference_device_id"] = track.getReferenceDeviceId().value();
    }
    if (track.getDirection().has_value()) {
      types::Direction direction = track.getDirection().value();
      jsonTrack["direction"] = json::array({direction.X, direction.Y, direction.Z});
    }
    if (track.getUnitLength().has_value()) {
      jsonTrack["unit_length"] = track.getUnitLength().value();
    }

    auto jsonVertices = json::array();
    auto numVertices = track.getVerticesSize();
    for (uint32_t k = 0; k < numVertices; k++) {
      jsonVertices.push_back(track.getVertexAt((int)k));
    }
    if (numVertices > 0) {
      jsonTrack["vertices"] = jsonVertices;
    }

    auto jsonBands = json::array();
    auto numBands = track.getBandsSize();
    for (uint32_t l = 0; l < numBands; l++) {
      auto band = track.getBandAt((int)l);
      auto jsonBand = json::object();
      jsonBand["band_type"] = types::bandTypeToString.at(band.getBandType());
      jsonBand["curve_type"] = types::curveTypeToString.at(band.getCurveType());
      jsonBand["encoding_modality"] = types::modalityToString.at(band.getEncodingModality());
      jsonBand["window_length"] = band.getWindowLength();
      jsonBand["lower_frequency_limit"] = band.getLowerFrequencyLimit();
      jsonBand["upper_frequency_limit"] = band.getUpperFrequencyLimit();

      auto jsonEffects = json::array();
      auto numEffects = band.getEffectsSize();
      for (uint32_t m = 0; m < numEffects; m++) {
        auto effect = band.getEffectAt((int)m);
        auto jsonEffect = json::object();
        jsonEffect["position"] = effect.getPosition();
        jsonEffect["phase"] = effect.getPhase();
        jsonEffect["base_signal"] = types::baseSignalToString.at(effect.getBaseSignal());

        auto jsonKeyframes = json::array();
        auto numKeyframes = effect.getKeyframesSize();
        for (uint32_t n = 0; n < numKeyframes; n++) {
          const auto &keyframe = effect.getKeyframeAt((int)n);
          auto jsonKeyframe = json::object();
          if (keyframe.getRelativePosition().has_value()) {
            jsonKeyframe["relative_position"] = keyframe.getRelativePosition().value();
          }
          if (keyframe.getAmplitudeModulation().has_value()) {
            jsonKeyframe["amplitude_modulation"] = keyframe.getAmplitudeModulation().value();
          }
          if (keyframe.getFrequencyModulation().has_value()) {
            jsonKeyframe["frequency_modulation"] = keyframe.getFrequencyModulation().value();
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
    jsonTracks.push_back(jsonTrack);
  }
}
auto IOJson::extractReferenceDevices(types::Perception &perception,
                                     nlohmann::json &jsonReferenceDevices) -> void {
  auto numReferenceDevices = perception.getReferenceDevicesSize();
  for (uint32_t i = 0; i < numReferenceDevices; i++) {
    auto referenceDevice = perception.getReferenceDeviceAt((int)i);
    auto jsonReferenceDevice = json::object();
    jsonReferenceDevice["id"] = referenceDevice.getId();
    jsonReferenceDevice["name"] = referenceDevice.getName();

    if (referenceDevice.getBodyPartMask().has_value()) {
      jsonReferenceDevice["body_part_mask"] = referenceDevice.getBodyPartMask().value();
    }
    if (referenceDevice.getMaximumFrequency().has_value()) {
      jsonReferenceDevice["maximum_frequency"] = referenceDevice.getMaximumFrequency().value();
    }
    if (referenceDevice.getMinimumFrequency().has_value()) {
      jsonReferenceDevice["minimum_frequency"] = referenceDevice.getMinimumFrequency().value();
    }
    if (referenceDevice.getResonanceFrequency().has_value()) {
      jsonReferenceDevice["resonance_frequency"] = referenceDevice.getResonanceFrequency().value();
    }
    if (referenceDevice.getMaximumAmplitude().has_value()) {
      jsonReferenceDevice["maximum_amplitude"] = referenceDevice.getMaximumAmplitude().value();
    }
    if (referenceDevice.getImpedance().has_value()) {
      jsonReferenceDevice["impedance"] = referenceDevice.getImpedance().value();
    }
    if (referenceDevice.getMaximumVoltage().has_value()) {
      jsonReferenceDevice["maximum_voltage"] = referenceDevice.getMaximumVoltage().value();
    }
    if (referenceDevice.getMaximumCurrent().has_value()) {
      jsonReferenceDevice["maximum_current"] = referenceDevice.getMaximumCurrent().value();
    }
    if (referenceDevice.getMaximumDisplacement().has_value()) {
      jsonReferenceDevice["maximum_displacement"] =
          referenceDevice.getMaximumDisplacement().value();
    }
    if (referenceDevice.getWeight().has_value()) {
      jsonReferenceDevice["weight"] = referenceDevice.getWeight().value();
    }
    if (referenceDevice.getSize().has_value()) {
      jsonReferenceDevice["size"] = referenceDevice.getSize().value();
    }
    if (referenceDevice.getCustom().has_value()) {
      jsonReferenceDevice["custom"] = referenceDevice.getCustom().value();
    }
    if (referenceDevice.getType().has_value()) {
      jsonReferenceDevice["type"] =
          types::actuatorTypeToString.at(referenceDevice.getType().value());
    }

    jsonReferenceDevices.push_back(jsonReferenceDevice);
  }
}
} // namespace haptics::io
