/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2022, ISO/IEC
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

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
#include <rapidjson/istreamwrapper.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using json = nlohmann::json;

namespace haptics::io {

auto IOJson::loadFile(const std::string &filePath, types::Haptics &haptic) -> bool {
  bool loadingSuccess = true;
  std::ifstream ifs(filePath);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document jsonTree;
  if (jsonTree.ParseStream(isw).HasParseError()) {
    std::cerr << "Invalid GMPG input file: JSON parsing error" << std::endl;
    return false;
  }
  if (!jsonTree.IsObject()) {
    std::cerr << "Invalid GMPG input file: not a JSON object" << std::endl;
    return false;
  }
  if (!(jsonTree.HasMember("version") && jsonTree.HasMember("date") &&
        jsonTree.HasMember("description") && jsonTree.HasMember("perceptions") &&
        jsonTree["perceptions"].IsArray() && jsonTree.HasMember("avatars") &&
        jsonTree["avatars"].IsArray())) {
    std::cerr << "Invalid GMPG input file: missing required field" << std::endl;
    return false;
  }
  auto version = jsonTree["version"].GetString();
  auto date = jsonTree["date"].GetString();
  auto description = jsonTree["description"].GetString();
  haptic.setVersion(std::string(version));
  haptic.setDate(std::string(date));
  haptic.setDescription(std::string(description));
  auto jsonAvatars = jsonTree["avatars"].GetArray();
  loadingSuccess = loadingSuccess && loadAvatars(jsonAvatars, haptic);
  auto jsonPerceptions = jsonTree["perceptions"].GetArray();
  loadingSuccess = loadingSuccess && loadPerceptions(jsonPerceptions, haptic);
  return loadingSuccess;
}

auto IOJson::loadPerceptions(const rapidjson::Value::Array &jsonPerceptions, types::Haptics &haptic)
    -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonPerceptions.begin(); it != jsonPerceptions.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid perception: not an object" << std::endl;
      continue;
    }
    auto jsonPerception = it->GetObject();

    if (!jsonPerception.HasMember("id") || !jsonPerception["id"].IsInt()) {
      std::cerr << "Missing or invalid perception id" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("avatar_id") || !jsonPerception["avatar_id"].IsInt()) {
      std::cerr << "Missing or invalid perception avatar id" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("description") || !jsonPerception["description"].IsString()) {
      std::cerr << "Missing or invalid perception description" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("perception_modality") ||
        !jsonPerception["perception_modality"].IsString()) {
      std::cerr << "Missing or invalid perception modality" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("tracks") || !jsonPerception["tracks"].IsArray()) {
      std::cerr << "Missing or invalid tracks" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("effect_library") ||
        !jsonPerception["effect_library"].IsArray()) {
      std::cerr << "Missing or invalid library" << std::endl;
      continue;
    }

    auto perceptionId = jsonPerception["id"].GetInt();
    auto perceptionAvatarId = jsonPerception["avatar_id"].GetInt();
    auto perceptionDescription = jsonPerception["description"].GetString();
    auto perceptionPerceptionModality =
        types::stringToPerceptionModality.at(jsonPerception["perception_modality"].GetString());

    haptics::types::Perception perception(perceptionId, perceptionAvatarId, perceptionDescription,
                                          perceptionPerceptionModality);

    if (jsonPerception.HasMember("unit_exponent") && jsonPerception["unit_exponent"].IsInt()) {
      perception.setUnitExponent(jsonPerception["unit_exponent"].GetInt());
    }
    if (jsonPerception.HasMember("perception_unit_exponent") &&
        jsonPerception["perception_unit_exponent"].IsInt()) {
      perception.setPerceptionUnitExponent(jsonPerception["perception_unit_exponent"].GetInt());
    }

    auto jsonLibrary = jsonPerception["effect_library"].GetArray();
    loadingSuccess = loadingSuccess && loadLibrary(jsonLibrary, perception);

    auto jsonTracks = jsonPerception["tracks"].GetArray();
    loadingSuccess = loadingSuccess && loadTracks(jsonTracks, perception);
    if (jsonPerception.HasMember("reference_devices") &&
        jsonPerception["reference_devices"].IsArray()) {
      auto jsonReferenceDevices = jsonPerception["reference_devices"].GetArray();
      loadingSuccess = loadingSuccess && loadReferenceDevices(jsonReferenceDevices, perception);
    }
    haptic.addPerception(perception);
  }
  return loadingSuccess;
}

auto IOJson::loadLibrary(const rapidjson::Value::Array &jsonLibrary, types::Perception &perception)
    -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonLibrary.begin(); it != jsonLibrary.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid effect library: not an object" << std::endl;
      continue;
    }
    auto jsonEffect = it->GetObject();

    if (!jsonEffect.HasMember("effect_type") || !jsonEffect["effect_type"].IsString()) {
      std::cerr << "Missing or invalid effect type" << std::endl;
      continue;
    }

    auto effectType = types::stringToEffectType.at(jsonEffect["effect_type"].GetString());
    if (effectType == types::EffectType::Reference) {
      if (!jsonEffect.HasMember("id") || !jsonEffect["id"].IsInt()) {
        std::cerr << "Missing or invalid effect id" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("position") || !jsonEffect["position"].IsInt()) {
        std::cerr << "Missing or invalid effect position" << std::endl;
        continue;
      }
    } else if (effectType == types::EffectType::Basis) {
      if (!jsonEffect.HasMember("position") || !jsonEffect["position"].IsInt()) {
        std::cerr << "Missing or invalid effect position" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("phase") || !jsonEffect["phase"].IsNumber()) {
        std::cerr << "Missing or invalid effect phase" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("base_signal") || !jsonEffect["base_signal"].IsString()) {
        std::cerr << "Missing or invalid effect base_signal" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("keyframes") || !jsonEffect["keyframes"].IsArray()) {
        std::cerr << "Missing or invalid list of keyframes" << std::endl;
        continue;
      }
    }

    auto id = jsonEffect["id"].GetInt();
    auto position = jsonEffect["position"].GetInt();
    auto phase = jsonEffect.HasMember("phase") ? jsonEffect["phase"].GetFloat() : 0;
    auto baseSignal = jsonEffect.HasMember("base_signal")
                          ? types::stringToBaseSignal.at(jsonEffect["base_signal"].GetString())
                          : types::BaseSignal::Sine;

    types::Effect effect(position, phase, baseSignal, effectType);
    effect.setId(id);
    if (jsonEffect.HasMember("keyframes")) {
      auto jsonKeyframes = jsonEffect["keyframes"].GetArray();
      loadingSuccess = loadingSuccess && loadKeyframes(jsonKeyframes, effect);
    }

    perception.addBasisEffect(effect);
  }
  return loadingSuccess;
}

auto IOJson::loadTracks(const rapidjson::Value::Array &jsonTracks, types::Perception &perception)
    -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonTracks.begin(); it != jsonTracks.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid track: not an object" << std::endl;
      continue;
    }
    auto jsonTrack = it->GetObject();

    if (!jsonTrack.HasMember("id") || !jsonTrack["id"].IsInt()) {
      std::cerr << "Missing or invalid track id" << std::endl;
      continue;
    }
    if (!jsonTrack.HasMember("description") || !jsonTrack["description"].IsString()) {
      std::cerr << "Missing or invalid track description" << std::endl;
      continue;
    }
    if (!jsonTrack.HasMember("gain") || !jsonTrack["gain"].IsNumber()) {
      std::cerr << "Missing or invalid track gain" << std::endl;
      continue;
    }
    if (!jsonTrack.HasMember("mixing_weight") || !jsonTrack["mixing_weight"].IsNumber()) {
      std::cerr << "Missing or invalid track mixing weight" << std::endl;
      continue;
    }
    if (!jsonTrack.HasMember("body_part_mask") || !jsonTrack["body_part_mask"].IsUint()) {
      std::cerr << "Missing or invalid track body part mask" << std::endl;
      continue;
    }
    if (!jsonTrack.HasMember("bands") || !jsonTrack["bands"].IsArray()) {
      std::cerr << "Missing or invalid bands" << std::endl;
      continue;
    }

    auto trackId = jsonTrack["id"].GetInt();
    auto trackDescription = jsonTrack["description"].GetString();
    auto trackGain = jsonTrack["gain"].GetFloat();
    auto trackMixingWeight = jsonTrack["mixing_weight"].GetFloat();
    auto trackBodyPart = jsonTrack["body_part_mask"].GetUint();

    types::Track track(trackId, trackDescription, trackGain, trackMixingWeight, trackBodyPart);

    if (jsonTrack.HasMember("direction") && jsonTrack["direction"].IsObject() &&
        jsonTrack["direction"].HasMember("X") && jsonTrack["direction"]["X"].IsInt() &&
        jsonTrack["direction"].HasMember("Y") && jsonTrack["direction"]["Y"].IsInt() &&
        jsonTrack["direction"].HasMember("Z") && jsonTrack["direction"]["Z"].IsInt()) {
      types::Direction direction(jsonTrack["direction"]["X"].GetInt(),
                                 jsonTrack["direction"]["Y"].GetInt(),
                                 jsonTrack["direction"]["Z"].GetInt());
      track.setDirection(direction);
    }

    if (jsonTrack.HasMember("frequency_sampling") && jsonTrack["frequency_sampling"].IsUint()) {
      auto frequencySampling = jsonTrack["frequency_sampling"].GetUint();
      track.setFrequencySampling(frequencySampling);
    }

    if (jsonTrack.HasMember("sample_count") && jsonTrack["sample_count"].IsUint()) {
      auto frequencySampling = jsonTrack["sample_count"].GetUint();
      track.setSampleCount(frequencySampling);
    }

    if (jsonTrack.HasMember("reference_device_id") && jsonTrack["reference_device_id"].IsInt()) {
      auto device_id = jsonTrack["reference_device_id"].GetInt();
      track.setReferenceDeviceId(device_id);
    }
    if (jsonTrack.HasMember("vertices") && jsonTrack["vertices"].IsArray()) {
      auto jsonVertices = jsonTrack["vertices"].GetArray();
      for (auto itv = jsonVertices.begin(); itv != jsonVertices.end(); ++itv) {
        if (itv->IsInt()) {
          auto vertex = itv->GetInt();
          track.addVertex(vertex);
        }
      }
    }
    auto jsonBands = jsonTrack["bands"].GetArray();
    loadingSuccess = loadingSuccess && loadBands(jsonBands, track);
    perception.addTrack(track);
  }
  return loadingSuccess;
}

auto IOJson::loadBands(const rapidjson::Value::Array &jsonBands, types::Track &track) -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonBands.begin(); it != jsonBands.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid band: not an object" << std::endl;
      continue;
    }
    auto jsonBand = it->GetObject();

    if (!jsonBand.HasMember("band_type") || !jsonBand["band_type"].IsString()) {
      std::cerr << "Missing or invalid band type" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("curve_type") || !jsonBand["curve_type"].IsString()) {
      std::cerr << "Missing or invalid curve type" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("window_length") || !jsonBand["window_length"].IsInt()) {
      std::cerr << "Missing or invalid window length" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("lower_frequency_limit") ||
        !jsonBand["lower_frequency_limit"].IsInt()) {
      std::cerr << "Missing or invalid lower frequency limit" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("upper_frequency_limit") ||
        !jsonBand["upper_frequency_limit"].IsInt()) {
      std::cerr << "Missing or invalid upper frequency limit" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("effects") || !jsonBand["effects"].IsArray()) {
      std::cerr << "Missing or invalid list of effects" << std::endl;
      continue;
    }

    types::BandType bandType = types::stringToBandType.at(jsonBand["band_type"].GetString());
    types::CurveType curveType = types::stringToCurveType.at(jsonBand["curve_type"].GetString());
    int windowLength = jsonBand["window_length"].GetInt();
    int lowerLimit = jsonBand["lower_frequency_limit"].GetInt();
    int upperLimit = jsonBand["upper_frequency_limit"].GetInt();

    types::Band band(bandType, curveType, windowLength, lowerLimit, upperLimit);
    auto jsonEffects = jsonBand["effects"].GetArray();
    loadingSuccess = loadingSuccess && loadEffects(jsonEffects, band);

    track.addBand(band);
  }
  return loadingSuccess;
}

auto IOJson::loadEffects(const rapidjson::Value::Array &jsonEffects, types::Band &band) -> bool {
  bool loadingSuccess = true;
  for (auto it = jsonEffects.begin(); it != jsonEffects.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid effect: not an object" << std::endl;
      continue;
    }
    auto jsonEffect = it->GetObject();

    if (!jsonEffect.HasMember("effect_type") || !jsonEffect["effect_type"].IsString()) {
      std::cerr << "Missing or invalid effect type" << std::endl;
      continue;
    }

    auto effectType = types::stringToEffectType.at(jsonEffect["effect_type"].GetString());
    if (effectType == types::EffectType::Reference) {
      if (!jsonEffect.HasMember("id") || !jsonEffect["id"].IsInt()) {
        std::cerr << "Missing or invalid effect id" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("position") || !jsonEffect["position"].IsInt()) {
        std::cerr << "Missing or invalid effect position" << std::endl;
        continue;
      }
    } else if (effectType == types::EffectType::Basis) {
      if (!jsonEffect.HasMember("position") || !jsonEffect["position"].IsInt()) {
        std::cerr << "Missing or invalid effect position" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("phase") || !jsonEffect["phase"].IsNumber()) {
        std::cerr << "Missing or invalid effect phase" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("base_signal") || !jsonEffect["base_signal"].IsString()) {
        std::cerr << "Missing or invalid effect base_signal" << std::endl;
        continue;
      }
      if (!jsonEffect.HasMember("keyframes") || !jsonEffect["keyframes"].IsArray()) {
        std::cerr << "Missing or invalid list of keyframes" << std::endl;
        continue;
      }
    }

    auto position = jsonEffect["position"].GetInt();
    auto phase = jsonEffect.HasMember("phase") ? jsonEffect["phase"].GetFloat() : 0;
    auto baseSignal = jsonEffect.HasMember("base_signal")
                          ? types::stringToBaseSignal.at(jsonEffect["base_signal"].GetString())
                          : types::BaseSignal::Sine;

    types::Effect effect(position, phase, baseSignal, effectType);
    if (jsonEffect.HasMember("id") && jsonEffect["id"].IsInt()) {
      auto id = jsonEffect["id"].GetInt();
      effect.setId(id);
    }
    if (jsonEffect.HasMember("keyframes") && jsonEffect["keyframes"].IsArray()) {
      auto jsonKeyframes = jsonEffect["keyframes"].GetArray();
      loadingSuccess = loadingSuccess && loadKeyframes(jsonKeyframes, effect);
    }

    band.addEffect(effect);
  }
  return loadingSuccess;
}

auto IOJson::loadAvatars(const rapidjson::Value::Array &jsonAvatars, types::Haptics &haptic)
    -> bool {
  for (auto it = jsonAvatars.begin(); it != jsonAvatars.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid avatar: not an object" << std::endl;
      continue;
    }
    auto jsonAvatar = it->GetObject();

    if (!jsonAvatar.HasMember("id") || !jsonAvatar["id"].IsInt()) {
      std::cerr << "Missing or invalid avatar id" << std::endl;
      continue;
    }
    if (!jsonAvatar.HasMember("lod") || !jsonAvatar["lod"].IsInt()) {
      std::cerr << "Missing or invalid avatar lod" << std::endl;
      continue;
    }
    if (!jsonAvatar.HasMember("type") || !jsonAvatar["type"].IsString()) {
      std::cerr << "Missing or invalid avatar type" << std::endl;
      continue;
    }
    auto id = jsonAvatar["id"].GetInt();
    auto lod = jsonAvatar["lod"].GetInt();
    auto type = types::stringToAvatarType.at(jsonAvatar["type"].GetString());

    types::Avatar avatar(id, lod, type);

    if (jsonAvatar.HasMember("mesh") && jsonAvatar["mesh"].IsString()) {
      avatar.setMesh(jsonAvatar["mesh"].GetString());
    }
    haptic.addAvatar(avatar);
  }
  return true;
}

auto IOJson::loadReferenceDevices(const rapidjson::Value::Array &jsonReferenceDevices,
                                  types::Perception &perception) -> bool {
  for (auto it = jsonReferenceDevices.begin(); it != jsonReferenceDevices.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid reference device: not an object" << std::endl;
      continue;
    }
    auto jsonReferenceDevice = it->GetObject();

    if (!jsonReferenceDevice.HasMember("id") || !jsonReferenceDevice["id"].IsInt()) {
      std::cerr << "Missing or invalid reference device id" << std::endl;
      continue;
    }
    if (!jsonReferenceDevice.HasMember("name") || !jsonReferenceDevice["name"].IsString()) {
      std::cerr << "Missing or invalid reference device name" << std::endl;
      continue;
    }
    auto id = jsonReferenceDevice["id"].GetInt();
    auto name = jsonReferenceDevice["name"].GetString();

    types::ReferenceDevice referenceDevice(id, name);
    if (jsonReferenceDevice.HasMember("body_part_mask") &&
        jsonReferenceDevice["body_part_mask"].IsUint()) {
      referenceDevice.setBodyPartMask(jsonReferenceDevice["body_part_mask"].GetUint());
    }
    if (jsonReferenceDevice.HasMember("maximum_frequency") &&
        jsonReferenceDevice["maximum_frequency"].IsNumber()) {
      referenceDevice.setMaximumFrequency(jsonReferenceDevice["maximum_frequency"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("minimum_frequency") &&
        jsonReferenceDevice["minimum_frequency"].IsNumber()) {
      referenceDevice.setMinimumFrequency(jsonReferenceDevice["minimum_frequency"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("resonance_frequency") &&
        jsonReferenceDevice["resonance_frequency"].IsNumber()) {
      referenceDevice.setResonanceFrequency(jsonReferenceDevice["resonance_frequency"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("maximum_amplitude") &&
        jsonReferenceDevice["maximum_amplitude"].IsNumber()) {
      referenceDevice.setMaximumAmplitude(jsonReferenceDevice["maximum_amplitude"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("impedance") && jsonReferenceDevice["impedance"].IsNumber()) {
      referenceDevice.setImpedance(jsonReferenceDevice["impedance"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("maximum_voltage") &&
        jsonReferenceDevice["maximum_voltage"].IsNumber()) {
      referenceDevice.setMaximumVoltage(jsonReferenceDevice["maximum_voltage"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("maximum_current") &&
        jsonReferenceDevice["maximum_current"].IsNumber()) {
      referenceDevice.setMaximumCurrent(jsonReferenceDevice["maximum_current"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("maximum_displacement") &&
        jsonReferenceDevice["maximum_displacement"].IsNumber()) {
      referenceDevice.setMaximumDisplacement(
          jsonReferenceDevice["maximum_displacement"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("weight") && jsonReferenceDevice["weight"].IsNumber()) {
      referenceDevice.setWeight(jsonReferenceDevice["weight"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("size") && jsonReferenceDevice["size"].IsNumber()) {
      referenceDevice.setSize(jsonReferenceDevice["size"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("custom") && jsonReferenceDevice["custom"].IsNumber()) {
      referenceDevice.setCustom(jsonReferenceDevice["custom"].GetFloat());
    }
    if (jsonReferenceDevice.HasMember("type") && jsonReferenceDevice["type"].IsString()) {
      auto type = jsonReferenceDevice["type"].GetString();
      referenceDevice.setType(types::stringToActuatorType.at(type));
    }
    perception.addReferenceDevice(referenceDevice);
  }
  return true;
}

auto IOJson::loadKeyframes(const rapidjson::Value::Array &jsonKeyframes, types::Effect &effect)
    -> bool {
  for (auto it = jsonKeyframes.begin(); it != jsonKeyframes.end(); ++it) {
    if (!it->IsObject()) {
      std::cerr << "Invalid keyframe: not an object" << std::endl;
      continue;
    }
    auto jsonKeyframe = it->GetObject();

    std::optional<int> relativePosition;
    std::optional<float> amplitudeModulation;
    std::optional<int> frequencyModulation;
    if (jsonKeyframe.HasMember("relative_position") && jsonKeyframe["relative_position"].IsInt()) {
      relativePosition = jsonKeyframe["relative_position"].GetInt();
    }
    if (jsonKeyframe.HasMember("amplitude_modulation") &&
        jsonKeyframe["amplitude_modulation"].IsNumber()) {
      amplitudeModulation = jsonKeyframe["amplitude_modulation"].GetFloat();
    }
    if (jsonKeyframe.HasMember("frequency_modulation") &&
        jsonKeyframe["frequency_modulation"].IsInt()) {
      frequencyModulation = jsonKeyframe["frequency_modulation"].GetInt();
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

    if (perception.getUnitExponent().has_value()) {
      jsonPerception["unit_exponent"] = perception.getUnitExponent().value();
    }
    if (perception.getPerceptionUnitExponent().has_value()) {
      jsonPerception["perception_unit_exponent"] = perception.getPerceptionUnitExponent().value();
    }

    auto jsonReferenceDevices = json::array();
    extractReferenceDevices(perception, jsonReferenceDevices);
    jsonPerception["reference_devices"] = jsonReferenceDevices;

    auto jsonLibrary = json::array();
    extractLibrary(perception, jsonLibrary);
    jsonPerception["effect_library"] = jsonLibrary;
    auto jsonTracks = json::array();
    extractTracks(perception, jsonTracks);
    jsonPerception["tracks"] = jsonTracks;

    jsonPerceptions.push_back(jsonPerception);
  }
}

auto IOJson::extractLibrary(types::Perception &perception, nlohmann::json &jsonLibrary) -> void {
  auto numEffects = perception.getEffectLibrarySize();
  for (uint32_t m = 0; m < numEffects; m++) {
    auto effect = perception.getBasisEffectAt((int)m);
    auto jsonEffect = json::object();
    jsonEffect["effect_type"] = types::effectTypeToString.at(effect.getEffectType());
    jsonEffect["id"] = effect.getId();
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
    jsonLibrary.push_back(jsonEffect);
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
    if (track.getFrequencySampling().has_value()) {
      jsonTrack["frequency_sampling"] = track.getFrequencySampling().value();
    }
    if (track.getSampleCount().has_value()) {
      jsonTrack["sample_count"] = track.getSampleCount().value();
    }
    if (track.getDirection().has_value()) {
      jsonTrack["direction"] = json::object();
      jsonTrack["direction"]["X"] = track.getDirection().value().X;
      jsonTrack["direction"]["Y"] = track.getDirection().value().Y;
      jsonTrack["direction"]["Z"] = track.getDirection().value().Z;
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
      jsonBand["window_length"] = band.getWindowLength();
      jsonBand["lower_frequency_limit"] = band.getLowerFrequencyLimit();
      jsonBand["upper_frequency_limit"] = band.getUpperFrequencyLimit();

      auto jsonEffects = json::array();
      auto numEffects = band.getEffectsSize();
      for (uint32_t m = 0; m < numEffects; m++) {
        auto effect = band.getEffectAt((int)m);
        auto jsonEffect = json::object();

        jsonEffect["effect_type"] = types::effectTypeToString.at(effect.getEffectType());
        jsonEffect["position"] = effect.getPosition();
        if (effect.getEffectType() == types::EffectType::Reference) {
          jsonEffect["id"] = effect.getId();
        } else if (effect.getEffectType() == types::EffectType::Basis) {
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
        }
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
