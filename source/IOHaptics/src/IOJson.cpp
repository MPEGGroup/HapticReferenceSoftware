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
#include <IOHaptics/include/IOJsonPrimitives.h>
#include <algorithm>
#include <iostream>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::io {

auto IOJson::loadFile(const std::string &filePath, types::Haptics &haptic) -> bool {
  bool loadingSuccess = true;
  std::ifstream ifs(filePath);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document jsonTree;
  if (jsonTree.ParseStream(isw).HasParseError()) {
    std::cerr << "Invalid HJIF input file: JSON parsing error" << std::endl;
    return false;
  }
  if (!jsonTree.IsObject()) {
    std::cerr << "Invalid HJIF input file: not a JSON object" << std::endl;
    return false;
  }
  if (!(jsonTree.HasMember("version") && jsonTree.HasMember("profile") &&
        jsonTree.HasMember("level") && jsonTree.HasMember("date") &&
        jsonTree.HasMember("description") && jsonTree.HasMember("timescale") &&
        jsonTree.HasMember("perceptions") && jsonTree["perceptions"].IsArray() &&
        jsonTree.HasMember("avatars") && jsonTree["avatars"].IsArray() &&
        jsonTree.HasMember("syncs") && jsonTree["syncs"].IsArray())) {

    std::cerr << "Invalid HJIF input file: missing required field" << std::endl;
    return false;
  }

  auto version = std::string(jsonTree["version"].GetString());
  auto profile = std::string(jsonTree["profile"].GetString());
  auto level = static_cast<uint8_t>(jsonTree["level"].GetUint());
  auto date = std::string(jsonTree["date"].GetString());
  auto description = std::string(jsonTree["description"].GetString());
  haptic.setVersion(version);
  haptic.setProfile(profile);
  haptic.setLevel(level);
  haptic.setDate(date);
  haptic.setDescription(description);
  if (jsonTree.HasMember("timescale") && jsonTree["timescale"].IsInt()) {
    haptic.setTimescale(jsonTree["timescale"].GetInt());
  }
  loadingSuccess = loadingSuccess && loadAvatars(jsonTree["avatars"], haptic);
  loadingSuccess = loadingSuccess && loadPerceptions(jsonTree["perceptions"], haptic);
  loadingSuccess = loadingSuccess && loadSyncs(jsonTree["syncs"], haptic);
  return loadingSuccess;
}

auto IOJson::loadPerceptions(const rapidjson::Value &jsonPerceptions, types::Haptics &haptic)
    -> bool {
  bool loadingSuccess = true;
  for (const auto &jpv : jsonPerceptions.GetArray()) {
    if (!jpv.IsObject()) {
      std::cerr << "Invalid perception: not an object" << std::endl;
      continue;
    }
    auto jsonPerception = jpv.GetObject();

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
    if (!jsonPerception.HasMember("channels") || !jsonPerception["channels"].IsArray()) {
      std::cerr << "Missing or invalid channels" << std::endl;
      continue;
    }
    if (!jsonPerception.HasMember("effect_library") ||
        !jsonPerception["effect_library"].IsArray()) {
      std::cerr << "Missing or invalid library" << std::endl;
      continue;
    }

    auto perceptionId = jsonPerception["id"].GetInt();
    auto perceptionAvatarId = jsonPerception["avatar_id"].GetInt();
    const auto *perceptionDescription = jsonPerception["description"].GetString();
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
    loadingSuccess = loadingSuccess && loadLibrary(jsonPerception["effect_library"], perception);
    if (jsonPerception.HasMember("effect_semantic_scheme") &&
        jsonPerception["effect_semantic_scheme"].IsString()) {
      auto effectSemanticScheme = std::string(jsonPerception["effect_semantic_scheme"].GetString());
      perception.setEffectSemanticScheme(effectSemanticScheme);
    }

    loadingSuccess = loadingSuccess && loadChannels(jsonPerception["channels"], perception);
    if (jsonPerception.HasMember("reference_devices") &&
        jsonPerception["reference_devices"].IsArray()) {
      loadingSuccess =
          loadingSuccess && loadReferenceDevices(jsonPerception["reference_devices"], perception);
    }
    haptic.addPerception(perception);
  }
  return loadingSuccess;
}

auto IOJson::loadSyncs(const rapidjson::Value &jsonSyncs, types::Haptics &haptic) -> bool {
  bool loadingSuccess = true;
  for (const auto &jsv : jsonSyncs.GetArray()) {
    if (!jsv.IsObject()) {
      std::cerr << "Invalid sync: not an object" << std::endl;
      continue;
    }
    auto jsonSync = jsv.GetObject();

    if (!jsonSync.HasMember("timestamp") || !jsonSync["timestamp"].IsInt()) {
      std::cerr << "Missing or invalid sync timestamp" << std::endl;
      continue;
    }
    auto timestamp = jsonSync["timestamp"].GetInt();
    types::Sync sync(timestamp);

    if (jsonSync.HasMember("timescale")) {
      if (jsonSync["timescale"].IsUint()) {
        auto timescale = jsonSync["timescale"].GetUint();
        sync.setTimescale(timescale);
      } else {
        std::cerr << "Invalid sync timescale" << std::endl;
      }
    }
    haptic.addSync(sync);
  }
  return loadingSuccess;
}

auto IOJson::loadLibrary(const rapidjson::Value &jsonLibrary, types::Perception &perception)
    -> bool {
  bool loadingSuccess = true;
  for (const auto &jev : jsonLibrary.GetArray()) {
    if (!jev.IsObject()) {
      std::cerr << "Invalid effect library: not an object" << std::endl;
      continue;
    }
    auto jsonEffect = jev.GetObject();

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
      // TODO: check if effect is not wavelet -> having no keyframes is allowed
      // check for HasMember("bitstream")?
      if (!jsonEffect.HasMember("keyframes") || !jsonEffect["keyframes"].IsArray()) {
        std::cerr << "Missing or invalid list of keyframes" << std::endl;
        continue;
      }
    }

    auto effectId = jsonEffect["id"].GetInt();
    auto position = jsonEffect["position"].GetInt();
    auto phase = jsonEffect.HasMember("phase") ? jsonEffect["phase"].GetFloat() : 0;
    auto baseSignal = jsonEffect.HasMember("base_signal")
                          ? types::stringToBaseSignal.at(jsonEffect["base_signal"].GetString())
                          : types::BaseSignal::Sine;

    types::Effect effect(position, phase, baseSignal, effectType);
    effect.setId(effectId);
    if (jsonEffect.HasMember("semantic") && jsonEffect["semantic"].IsString()) {
      auto semantic = std::string(jsonEffect["semantic"].GetString());
      effect.setSemantic(semantic);
    }
    if (jsonEffect.HasMember("keyframes")) {
      loadingSuccess = loadingSuccess && loadKeyframes(jsonEffect["keyframes"], effect);
    }

    perception.addBasisEffect(effect);
  }
  return loadingSuccess;
}

auto IOJson::loadChannels(const rapidjson::Value &jsonChannels, types::Perception &perception)
    -> bool {
  bool loadingSuccess = true;
  for (const auto &jtv : jsonChannels.GetArray()) {
    if (!jtv.IsObject()) {
      std::cerr << "Invalid channel: not an object" << std::endl;
      continue;
    }
    auto jsonChannel = jtv.GetObject();

    if (!IOJsonPrimitives::hasInt(jsonChannel, "id")) {
      std::cerr << "Missing or invalid channel id" << std::endl;
      continue;
    }
    if (!IOJsonPrimitives::hasString(jsonChannel, "description")) {
      std::cerr << "Missing or invalid channel description" << std::endl;
      continue;
    }
    if (!IOJsonPrimitives::hasNumber(jsonChannel, "gain")) {
      std::cerr << "Missing or invalid channel gain" << std::endl;
      continue;
    }
    if (!IOJsonPrimitives::hasNumber(jsonChannel, "mixing_weight")) {
      std::cerr << "Missing or invalid channel mixing weight" << std::endl;
      continue;
    }
    if (!IOJsonPrimitives::hasNumber(jsonChannel, "body_part_mask")) {
      std::cerr << "Missing or invalid channel body part mask" << std::endl;
      continue;
    }
    if (!IOJsonPrimitives::hasArray(jsonChannel, "bands")) {
      std::cerr << "Missing or invalid bands" << std::endl;
      continue;
    }

    auto channelId = jsonChannel["id"].GetInt();
    const auto *channelDescription = jsonChannel["description"].GetString();
    auto channelGain = jsonChannel["gain"].GetFloat();
    auto channelMixingWeight = jsonChannel["mixing_weight"].GetFloat();
    auto channelBodyPart = jsonChannel["body_part_mask"].GetUint();

    types::Channel channel(channelId, channelDescription, channelGain, channelMixingWeight,
                           channelBodyPart);

    types::Vector direction{};
    if (jsonChannel.HasMember("direction") && loadVector(jsonChannel["direction"], direction)) {
      channel.setDirection(direction);
    }

    types::Vector actuatorResolution{};
    if (jsonChannel.HasMember("actuator_resolution") &&
        loadVector(jsonChannel["actuator_resolution"], actuatorResolution)) {
      channel.setActuatorResolution(actuatorResolution);
    }

    std::vector<std::string> bodyPartTargetString;
    if (IOJsonPrimitives::getStringArray(jsonChannel, "body_part_target", bodyPartTargetString)) {
      std::vector<types::BodyPartTarget> bodyPartTarget;
      std::transform(bodyPartTargetString.begin(), bodyPartTargetString.end(),
                     std::back_inserter(bodyPartTarget),
                     [](const std::string &str) { return types::stringToBodyPartTarget.at(str); });
      channel.setBodyPartTarget(bodyPartTarget);
    }

    std::vector<types::Vector> actuatorTarget;
    if (IOJsonPrimitives::getVectorArray(jsonChannel, "actuator_target", actuatorTarget)) {
      channel.setActuatorTarget(actuatorTarget);
    }

    if (IOJsonPrimitives::hasUint(jsonChannel, "frequency_sampling")) {
      auto frequencySampling = jsonChannel["frequency_sampling"].GetUint();
      channel.setFrequencySampling(frequencySampling);
    }

    if (IOJsonPrimitives::hasUint(jsonChannel, "sample_count")) {
      auto frequencySampling = jsonChannel["sample_count"].GetUint();
      channel.setSampleCount(frequencySampling);
    }

    if (IOJsonPrimitives::hasInt(jsonChannel, "reference_device_id")) {
      auto device_id = jsonChannel["reference_device_id"].GetInt();
      channel.setReferenceDeviceId(device_id);
    }

    std::vector<int> vertices;
    if (IOJsonPrimitives::getIntArray(jsonChannel, "vertices", vertices)) {
      for (auto &vertex : vertices) {
        channel.addVertex(vertex);
      }
    }
    loadingSuccess = loadingSuccess && loadBands(jsonChannel["bands"], channel);
    perception.addChannel(channel);
  }
  return loadingSuccess;
}

auto IOJson::loadBands(const rapidjson::Value &jsonBands, types::Channel &channel) -> bool {
  bool loadingSuccess = true;
  for (const auto &jbv : jsonBands.GetArray()) {
    if (!jbv.IsObject()) {
      std::cerr << "Invalid band: not an object" << std::endl;
      continue;
    }
    auto jsonBand = jbv.GetObject();

    if (!jsonBand.HasMember("band_type") || !jsonBand["band_type"].IsString()) {
      std::cerr << "Missing or invalid band type" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("curve_type") || !jsonBand["curve_type"].IsString()) {
      std::cerr << "Missing or invalid curve type" << std::endl;
      continue;
    }
    if (!jsonBand.HasMember("block_length") || !jsonBand["block_length"].IsDouble()) {
      std::cerr << "Missing or invalid block length" << std::endl;
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

    types::BandType bandType = types::stringToBandType.at(jsonBand["band_type"].GetString());
    if (bandType != types::BandType::WaveletWave) {
      if (!jsonBand.HasMember("effects") || !jsonBand["effects"].IsArray()) {
        std::cerr << "Missing or invalid list of effects" << std::endl;
        continue;
      }
    }

    types::CurveType curveType = types::stringToCurveType.at(jsonBand["curve_type"].GetString());
    double blockLength = jsonBand["block_length"].GetDouble();
    int lowerLimit = jsonBand["lower_frequency_limit"].GetInt();
    int upperLimit = jsonBand["upper_frequency_limit"].GetInt();

    types::Band band(bandType, curveType, blockLength, lowerLimit, upperLimit);
    loadingSuccess = loadingSuccess && loadEffects(jsonBand["effects"], band);

    channel.addBand(band);
  }
  return loadingSuccess;
}

auto IOJson::loadEffects(const rapidjson::Value &jsonEffects, types::Band &band) -> bool {
  bool loadingSuccess = true;
  for (const auto &jev : jsonEffects.GetArray()) {
    if (!jev.IsObject()) {
      std::cerr << "Invalid effect: not an object" << std::endl;
      continue;
    }
    auto jsonEffect = jev.GetObject();

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
      if (band.getBandType() != types::BandType::WaveletWave) {
        if (!jsonEffect.HasMember("keyframes") || !jsonEffect["keyframes"].IsArray()) {
          std::cerr << "Missing or invalid list of keyframes" << std::endl;
          continue;
        }
      } else {
        if (!jsonEffect.HasMember("bitstream")) {
          std::cerr << "Missing bitstream" << std::endl;
          continue;
        }
      }
    }

    auto position = jsonEffect["position"].GetInt();
    auto phase = jsonEffect.HasMember("phase") ? jsonEffect["phase"].GetFloat() : 0;
    auto baseSignal = jsonEffect.HasMember("base_signal")
                          ? types::stringToBaseSignal.at(jsonEffect["base_signal"].GetString())
                          : types::BaseSignal::Sine;

    types::Effect effect(position, phase, baseSignal, effectType);
    if (jsonEffect.HasMember("id") && jsonEffect["id"].IsInt()) {
      effect.setId(jsonEffect["id"].GetInt());
    }
    if (jsonEffect.HasMember("semantic") && jsonEffect["semantic"].IsString()) {
      auto semantic = std::string(jsonEffect["semantic"].GetString());
      effect.setSemantic(semantic);
    }
    if (jsonEffect.HasMember("keyframes") && jsonEffect["keyframes"].IsArray()) {
      loadingSuccess = loadingSuccess && loadKeyframes(jsonEffect["keyframes"], effect);
    }
    if (jsonEffect.HasMember("bitstream")) {
      loadingSuccess = loadingSuccess && loadBitstream(jsonEffect["bitstream"], effect);
    }

    band.addEffect(effect);
  }
  return loadingSuccess;
}

auto IOJson::loadAvatars(const rapidjson::Value &jsonAvatars, types::Haptics &haptic) -> bool {
  for (const auto &jav : jsonAvatars.GetArray()) {
    if (!jav.IsObject()) {
      std::cerr << "Invalid avatar: not an object" << std::endl;
      continue;
    }
    auto jsonAvatar = jav.GetObject();

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
    auto avatarId = jsonAvatar["id"].GetInt();
    auto lod = jsonAvatar["lod"].GetInt();
    auto type = types::stringToAvatarType.at(jsonAvatar["type"].GetString());

    types::Avatar avatar(avatarId, lod, type);

    if (jsonAvatar.HasMember("mesh") && jsonAvatar["mesh"].IsString()) {
      avatar.setMesh(jsonAvatar["mesh"].GetString());
    }
    haptic.addAvatar(avatar);
  }
  return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto IOJson::loadReferenceDevices(const rapidjson::Value &jsonReferenceDevices,
                                  types::Perception &perception) -> bool {
  for (const auto &jrdv : jsonReferenceDevices.GetArray()) {
    if (!jrdv.IsObject()) {
      std::cerr << "Invalid reference device: not an object" << std::endl;
      continue;
    }
    auto jsonReferenceDevice = jrdv.GetObject();

    if (!jsonReferenceDevice.HasMember("id") || !jsonReferenceDevice["id"].IsInt()) {
      std::cerr << "Missing or invalid reference device id" << std::endl;
      continue;
    }
    if (!jsonReferenceDevice.HasMember("name") || !jsonReferenceDevice["name"].IsString()) {
      std::cerr << "Missing or invalid reference device name" << std::endl;
      continue;
    }
    auto referenceDeviceId = jsonReferenceDevice["id"].GetInt();
    const auto *name = jsonReferenceDevice["name"].GetString();

    types::ReferenceDevice referenceDevice(referenceDeviceId, name);
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
      referenceDevice.setType(
          types::stringToActuatorType.at(jsonReferenceDevice["type"].GetString()));
    }
    perception.addReferenceDevice(referenceDevice);
  }
  return true;
}

auto IOJson::loadKeyframes(const rapidjson::Value &jsonKeyframes, types::Effect &effect) -> bool {
  for (const auto &jkfv : jsonKeyframes.GetArray()) {
    if (!jkfv.IsObject()) {
      std::cerr << "Invalid keyframe: not an object" << std::endl;
      continue;
    }
    auto jsonKeyframe = jkfv.GetObject();

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

auto IOJson::loadBitstream(const rapidjson::Value &jsonBitstream, types::Effect &effect) -> bool {
  const auto *const stream = jsonBitstream.GetString();
  auto length = jsonBitstream.GetStringLength();
  auto effectStream = std::vector<unsigned char>();
  std::string temp(stream, length);
  std::copy(temp.begin(), temp.end(), std::back_inserter(effectStream));
  auto effectStream_bits = std::vector<unsigned char>();
  base642bits(effectStream, effectStream_bits);
  auto effectStream_bytes = std::vector<unsigned char>();
  bits2bytes(effectStream_bits, effectStream_bytes);
  effect.setWaveletBitstream(effectStream_bytes);
  return true;
}

auto IOJson::loadVector(const rapidjson::Value &jsonVector, types::Vector &vector) -> bool {
  if (!(jsonVector.IsObject() && jsonVector.HasMember("X") && jsonVector["X"].IsInt() &&
        jsonVector.HasMember("Y") && jsonVector["Y"].IsInt() && jsonVector.HasMember("Z") &&
        jsonVector["Z"].IsInt())) {
    return false;
  }

  vector = types::Vector(static_cast<int8_t>(jsonVector["X"].GetInt()),
                         static_cast<int8_t>(jsonVector["Y"].GetInt()),
                         static_cast<int8_t>(jsonVector["Z"].GetInt()));
  return true;
}

auto IOJson::writeFile(haptics::types::Haptics &haptic, const std::string &filePath) -> void {
  auto jsonTree = rapidjson::Document(rapidjson::kObjectType);
  jsonTree.AddMember("version",
                     rapidjson::Value(haptic.getVersion().c_str(), jsonTree.GetAllocator()),
                     jsonTree.GetAllocator());
  jsonTree.AddMember("profile",
                     rapidjson::Value(haptic.getProfile().c_str(), jsonTree.GetAllocator()),
                     jsonTree.GetAllocator());
  jsonTree.AddMember("level", haptic.getLevel(), jsonTree.GetAllocator());
  jsonTree.AddMember("date", rapidjson::Value(haptic.getDate().c_str(), jsonTree.GetAllocator()),
                     jsonTree.GetAllocator());
  jsonTree.AddMember("description",
                     rapidjson::Value(haptic.getDescription().c_str(), jsonTree.GetAllocator()),
                     jsonTree.GetAllocator());
  jsonTree.AddMember("timescale", haptic.getTimescaleOrDefault(), jsonTree.GetAllocator());

  auto jsonAvatars = rapidjson::Value(rapidjson::kArrayType);
  extractAvatars(haptic, jsonAvatars, jsonTree);
  jsonTree.AddMember("avatars", jsonAvatars, jsonTree.GetAllocator());
  auto jsonPerceptions = rapidjson::Value(rapidjson::kArrayType);
  extractPerceptions(haptic, jsonPerceptions, jsonTree);
  jsonTree.AddMember("perceptions", jsonPerceptions, jsonTree.GetAllocator());
  auto jsonSyncs = rapidjson::Value(rapidjson::kArrayType);
  extractSyncs(haptic, jsonSyncs, jsonTree);
  jsonTree.AddMember("syncs", jsonSyncs, jsonTree.GetAllocator());
  std::ofstream file(filePath);
  rapidjson::OStreamWrapper osw(file);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  jsonTree.Accept(writer);
}

auto IOJson::extractPerceptions(types::Haptics &haptic, rapidjson::Value &jsonPerceptions,
                                rapidjson::Document &jsonTree) -> void {
  auto numPerceptions = haptic.getPerceptionsSize();
  for (decltype(numPerceptions) pix = 0; pix < numPerceptions; pix++) {
    auto perception = haptic.getPerceptionAt(static_cast<int>(pix));
    auto jsonPerception = rapidjson::Value(rapidjson::kObjectType);
    jsonPerception.AddMember("id", perception.getId(), jsonTree.GetAllocator());
    jsonPerception.AddMember("avatar_id", perception.getAvatarId(), jsonTree.GetAllocator());
    jsonPerception.AddMember(
        "description",
        rapidjson::Value(perception.getDescription().c_str(), jsonTree.GetAllocator()),
        jsonTree.GetAllocator());
    jsonPerception.AddMember(
        "perception_modality",
        rapidjson::Value(
            types::perceptionModalityToString.at(perception.getPerceptionModality()).c_str(),
            jsonTree.GetAllocator()),
        jsonTree.GetAllocator());

    if (perception.getUnitExponent().has_value()) {
      jsonPerception.AddMember("unit_exponent", perception.getUnitExponent().value(),
                               jsonTree.GetAllocator());
    }
    if (perception.getPerceptionUnitExponent().has_value()) {
      jsonPerception.AddMember("perception_unit_exponent",
                               perception.getPerceptionUnitExponent().value(),
                               jsonTree.GetAllocator());
    }

    auto jsonReferenceDevices = rapidjson::Value(rapidjson::kArrayType);
    extractReferenceDevices(perception, jsonReferenceDevices, jsonTree);
    jsonPerception.AddMember("reference_devices", jsonReferenceDevices, jsonTree.GetAllocator());

    auto jsonLibrary = rapidjson::Value(rapidjson::kArrayType);
    extractLibrary(perception, jsonLibrary, jsonTree);
    jsonPerception.AddMember("effect_library", jsonLibrary, jsonTree.GetAllocator());
    if (perception.getEffectSemanticScheme().has_value()) {
      jsonPerception.AddMember(
          "effect_semantic_scheme",
          rapidjson::Value(perception.getEffectSemanticScheme().value().c_str(),
                           jsonTree.GetAllocator()),
          jsonTree.GetAllocator());
    }
    auto jsonChannels = rapidjson::Value(rapidjson::kArrayType);
    extractChannels(perception, jsonChannels, jsonTree);
    jsonPerception.AddMember("channels", jsonChannels, jsonTree.GetAllocator());

    jsonPerceptions.PushBack(jsonPerception, jsonTree.GetAllocator());
  }
}

auto IOJson::extractSyncs(types::Haptics &haptic, rapidjson::Value &jsonSyncs,
                          rapidjson::Document &jsonTree) -> void {
  auto numSyncs = haptic.getSyncsSize();
  for (decltype(numSyncs) i = 0; i < numSyncs; i++) {
    auto sync = haptic.getSyncsAt(static_cast<int>(i));
    auto jsonSync = rapidjson::Value(rapidjson::kObjectType);
    jsonSync.AddMember("timestamp", sync.getTimestamp(), jsonTree.GetAllocator());

    if (sync.getTimescale().has_value()) {
      jsonSync.AddMember("timescale", sync.getTimescale().value(), jsonTree.GetAllocator());
    }
    jsonSyncs.PushBack(jsonSync, jsonTree.GetAllocator());
  }
}

auto IOJson::extractLibrary(types::Perception &perception, rapidjson::Value &jsonLibrary,
                            rapidjson::Document &jsonTree) -> void {
  auto numEffects = perception.getEffectLibrarySize();
  for (decltype(numEffects) eix = 0; eix < numEffects; eix++) {
    auto effect = perception.getBasisEffectAt(static_cast<int>(eix));
    auto jsonEffect = rapidjson::Value(rapidjson::kObjectType);
    jsonEffect.AddMember(
        "effect_type",
        rapidjson::Value(types::effectTypeToString.at(effect.getEffectType()).c_str(),
                         jsonTree.GetAllocator()),
        jsonTree.GetAllocator());
    jsonEffect.AddMember("id", effect.getId(), jsonTree.GetAllocator());
    jsonEffect.AddMember("position", effect.getPosition(), jsonTree.GetAllocator());
    jsonEffect.AddMember("phase", effect.getPhase(), jsonTree.GetAllocator());
    jsonEffect.AddMember(
        "base_signal",
        rapidjson::Value(types::baseSignalToString.at(effect.getBaseSignal()).c_str(),
                         jsonTree.GetAllocator()),
        jsonTree.GetAllocator());
    if (effect.getSemantic().has_value()) {
      jsonEffect.AddMember(
          "semantic",
          rapidjson::Value(effect.getSemantic().value().c_str(), jsonTree.GetAllocator()),
          jsonTree.GetAllocator());
    }
    // TODO: switch to reading bitstream for wavelet
    auto jsonKeyframes = rapidjson::Value(rapidjson::kArrayType);
    auto numKeyframes = effect.getKeyframesSize();
    for (decltype(numKeyframes) kfix = 0; kfix < numKeyframes; kfix++) {
      const auto &keyframe = effect.getKeyframeAt(static_cast<int>(kfix));
      auto jsonKeyframe = rapidjson::Value(rapidjson::kObjectType);
      if (keyframe.getRelativePosition().has_value()) {
        jsonKeyframe.AddMember("relative_position", keyframe.getRelativePosition().value(),
                               jsonTree.GetAllocator());
      }
      if (keyframe.getAmplitudeModulation().has_value()) {
        jsonKeyframe.AddMember("amplitude_modulation", keyframe.getAmplitudeModulation().value(),
                               jsonTree.GetAllocator());
      }
      if (keyframe.getFrequencyModulation().has_value()) {
        jsonKeyframe.AddMember("frequency_modulation", keyframe.getFrequencyModulation().value(),
                               jsonTree.GetAllocator());
      }
      jsonKeyframes.PushBack(jsonKeyframe, jsonTree.GetAllocator());
    }
    jsonEffect.AddMember("keyframes", jsonKeyframes, jsonTree.GetAllocator());
    jsonLibrary.PushBack(jsonEffect, jsonTree.GetAllocator());
  }
}

auto IOJson::extractAvatars(types::Haptics &haptic, rapidjson::Value &jsonAvatars,
                            rapidjson::Document &jsonTree) -> void {
  auto numAvatars = haptic.getAvatarsSize();
  for (decltype(numAvatars) aix = 0; aix < numAvatars; aix++) {
    auto avatar = haptic.getAvatarAt(static_cast<int>(aix));
    auto jsonAvatar = rapidjson::Value(rapidjson::kObjectType);
    jsonAvatar.AddMember("id", avatar.getId(), jsonTree.GetAllocator());
    jsonAvatar.AddMember("lod", avatar.getLod(), jsonTree.GetAllocator());
    jsonAvatar.AddMember("type",
                         rapidjson::Value(types::avatarTypeToString.at(avatar.getType()).c_str(),
                                          jsonTree.GetAllocator()),
                         jsonTree.GetAllocator());
    if (avatar.getMesh().has_value() && avatar.getType() == haptics::types::AvatarType::Custom) {
      jsonAvatar.AddMember(
          "mesh", rapidjson::Value(avatar.getMesh().value().c_str(), jsonTree.GetAllocator()),
          jsonTree.GetAllocator());
    }
    jsonAvatars.PushBack(jsonAvatar, jsonTree.GetAllocator());
  }
}

auto IOJson::extractChannels(types::Perception &perception, rapidjson::Value &jsonChannels,
                             rapidjson::Document &jsonTree) -> void {
  auto numChannels = perception.getChannelsSize();
  for (decltype(numChannels) tix = 0; tix < numChannels; tix++) {
    haptics::types::Channel channel = perception.getChannelAt(static_cast<int>(tix));
    auto jsonChannel = rapidjson::Value(rapidjson::kObjectType);
    jsonChannel.AddMember("id", channel.getId(), jsonTree.GetAllocator());
    jsonChannel.AddMember(
        "description", rapidjson::Value(channel.getDescription().c_str(), jsonTree.GetAllocator()),
        jsonTree.GetAllocator());
    jsonChannel.AddMember("gain", channel.getGain(), jsonTree.GetAllocator());
    jsonChannel.AddMember("mixing_weight", channel.getMixingWeight(), jsonTree.GetAllocator());
    jsonChannel.AddMember("body_part_mask", channel.getBodyPartMask(), jsonTree.GetAllocator());
    if (channel.getReferenceDeviceId().has_value()) {
      jsonChannel.AddMember("reference_device_id", channel.getReferenceDeviceId().value(),
                            jsonTree.GetAllocator());
    }
    if (channel.getFrequencySampling().has_value()) {
      jsonChannel.AddMember("frequency_sampling", channel.getFrequencySampling().value(),
                            jsonTree.GetAllocator());
    }
    if (channel.getSampleCount().has_value()) {
      jsonChannel.AddMember("sample_count", channel.getSampleCount().value(),
                            jsonTree.GetAllocator());
    }
    if (channel.getDirection().has_value()) {
      types::Vector direction = channel.getDirection().value();
      auto jsonDirection = rapidjson::Value(rapidjson::kObjectType);
      extractVector(direction, jsonDirection, jsonTree);
      jsonChannel.AddMember("direction", jsonDirection, jsonTree.GetAllocator());
    }
    if (channel.getActuatorResolution().has_value()) {
      types::Vector actuatorResolution = channel.getActuatorResolution().value();
      auto jsonActuatorResolution = rapidjson::Value(rapidjson::kObjectType);
      extractVector(actuatorResolution, jsonActuatorResolution, jsonTree);
      jsonChannel.AddMember("actuator_resolution", jsonActuatorResolution, jsonTree.GetAllocator());
    }
    if (channel.getBodyPartTarget().has_value()) {
      auto jsonBodyPartTarget = rapidjson::Value(rapidjson::kArrayType);
      std::vector<types::BodyPartTarget> bodyPartTargetList = channel.getBodyPartTarget().value();
      for (types::BodyPartTarget &bodyPartTarget : bodyPartTargetList) {
        auto bpt_value = rapidjson::Value(types::bodyPartTargetToString.at(bodyPartTarget).c_str(),
                                          jsonTree.GetAllocator());
        jsonBodyPartTarget.PushBack(bpt_value, jsonTree.GetAllocator());
      }
      jsonChannel.AddMember("body_part_target", jsonBodyPartTarget, jsonTree.GetAllocator());
    }
    if (channel.getActuatorTarget().has_value()) {
      auto jsonBodyPartTarget = rapidjson::Value(rapidjson::kArrayType);
      std::vector<types::Vector> actuatorTargetList = channel.getActuatorTarget().value();
      for (types::Vector &actuatorTarget : actuatorTargetList) {
        auto jsonTarget = rapidjson::Value(rapidjson::kObjectType);
        extractVector(actuatorTarget, jsonTarget, jsonTree);
        jsonBodyPartTarget.PushBack(jsonTarget, jsonTree.GetAllocator());
      }
      jsonChannel.AddMember("actuator_target", jsonBodyPartTarget, jsonTree.GetAllocator());
    }

    auto numVertices = channel.getVerticesSize();
    if (numVertices > 0) {
      auto jsonVertices = rapidjson::Value(rapidjson::kArrayType);
      for (decltype(numVertices) vix = 0; vix < numVertices; vix++) {
        jsonVertices.PushBack(channel.getVertexAt(static_cast<int>(vix)), jsonTree.GetAllocator());
      }
      jsonChannel.AddMember("vertices", jsonVertices, jsonTree.GetAllocator());
    }

    auto jsonBands = rapidjson::Value(rapidjson::kArrayType);
    IOJson::extractBands(channel, jsonBands, jsonTree);
    jsonChannel.AddMember("bands", jsonBands, jsonTree.GetAllocator());
    jsonChannels.PushBack(jsonChannel, jsonTree.GetAllocator());
  }
}

auto IOJson::extractBands(types::Channel &channel, rapidjson::Value &jsonBands,
                          rapidjson::Document &jsonTree) -> void {
  auto numBands = channel.getBandsSize();
  for (decltype(numBands) bix = 0; bix < numBands; bix++) {
    auto band = channel.getBandAt(static_cast<int>(bix));
    auto jsonBand = rapidjson::Value(rapidjson::kObjectType);
    jsonBand.AddMember("band_type",
                       rapidjson::Value(types::bandTypeToString.at(band.getBandType()).c_str(),
                                        jsonTree.GetAllocator()),
                       jsonTree.GetAllocator());
    jsonBand.AddMember("curve_type",
                       rapidjson::Value(types::curveTypeToString.at(band.getCurveType()).c_str(),
                                        jsonTree.GetAllocator()),
                       jsonTree.GetAllocator());
    jsonBand.AddMember("block_length", band.getBlockLength(), jsonTree.GetAllocator());
    jsonBand.AddMember("lower_frequency_limit", band.getLowerFrequencyLimit(),
                       jsonTree.GetAllocator());
    jsonBand.AddMember("upper_frequency_limit", band.getUpperFrequencyLimit(),
                       jsonTree.GetAllocator());

    auto jsonEffects = rapidjson::Value(rapidjson::kArrayType);
    auto numEffects = band.getEffectsSize();
    for (decltype(numEffects) eix = 0; eix < numEffects; eix++) {
      auto effect = band.getEffectAt(static_cast<int>(eix));
      auto jsonEffect = rapidjson::Value(rapidjson::kObjectType);

      jsonEffect.AddMember(
          "effect_type",
          rapidjson::Value(types::effectTypeToString.at(effect.getEffectType()).c_str(),
                           jsonTree.GetAllocator()),
          jsonTree.GetAllocator());
      jsonEffect.AddMember("position", effect.getPosition(), jsonTree.GetAllocator());
      if (effect.getSemantic().has_value()) {
        jsonEffect.AddMember(
            "semantic",
            rapidjson::Value(effect.getSemantic().value().c_str(), jsonTree.GetAllocator()),
            jsonTree.GetAllocator());
      }
      if (effect.getEffectType() == types::EffectType::Reference) {
        jsonEffect.AddMember("id", effect.getId(), jsonTree.GetAllocator());
      } else if (effect.getEffectType() == types::EffectType::Basis) {
        jsonEffect.AddMember("phase", effect.getPhase(), jsonTree.GetAllocator());
        jsonEffect.AddMember(
            "base_signal",
            rapidjson::Value(types::baseSignalToString.at(effect.getBaseSignal()).c_str(),
                             jsonTree.GetAllocator()),
            jsonTree.GetAllocator());
        if (band.getBandType() == types::BandType::WaveletWave) {
          auto stream = effect.getWaveletBitstream();
          auto stream_bits = std::vector<unsigned char>();
          bytes2bits(stream, stream_bits);
          auto stream_base64 = std::vector<unsigned char>();
          bits2base64(stream_bits, stream_base64);
          std::string streamString(stream_base64.begin(), stream_base64.end());
          jsonEffect.AddMember("bitstream",
                               rapidjson::Value(streamString.c_str(), jsonTree.GetAllocator()),
                               jsonTree.GetAllocator());
        } else {
          auto jsonKeyframes = rapidjson::Value(rapidjson::kArrayType);
          auto numKeyframes = effect.getKeyframesSize();
          for (decltype(numKeyframes) kfix = 0; kfix < numKeyframes; kfix++) {
            const auto &keyframe = effect.getKeyframeAt(static_cast<int>(kfix));
            auto jsonKeyframe = rapidjson::Value(rapidjson::kObjectType);
            if (keyframe.getRelativePosition().has_value()) {
              jsonKeyframe.AddMember("relative_position", keyframe.getRelativePosition().value(),
                                     jsonTree.GetAllocator());
            }
            if (keyframe.getAmplitudeModulation().has_value()) {
              jsonKeyframe.AddMember("amplitude_modulation",
                                     keyframe.getAmplitudeModulation().value(),
                                     jsonTree.GetAllocator());
            }
            if (keyframe.getFrequencyModulation().has_value()) {
              jsonKeyframe.AddMember("frequency_modulation",
                                     keyframe.getFrequencyModulation().value(),
                                     jsonTree.GetAllocator());
            }
            jsonKeyframes.PushBack(jsonKeyframe, jsonTree.GetAllocator());
          }
          jsonEffect.AddMember("keyframes", jsonKeyframes, jsonTree.GetAllocator());
        }
      }
      jsonEffects.PushBack(jsonEffect, jsonTree.GetAllocator());
    }
    jsonBand.AddMember("effects", jsonEffects, jsonTree.GetAllocator());
    jsonBands.PushBack(jsonBand, jsonTree.GetAllocator());
  }
}

auto IOJson::extractReferenceDevices(types::Perception &perception,
                                     rapidjson::Value &jsonReferenceDevices,
                                     rapidjson::Document &jsonTree) -> void {
  auto numReferenceDevices = perception.getReferenceDevicesSize();
  for (decltype(numReferenceDevices) rdix = 0; rdix < numReferenceDevices; rdix++) {
    auto referenceDevice = perception.getReferenceDeviceAt(static_cast<int>(rdix));
    auto jsonReferenceDevice = rapidjson::Value(rapidjson::kObjectType);
    jsonReferenceDevice.AddMember("id", referenceDevice.getId(), jsonTree.GetAllocator());
    jsonReferenceDevice.AddMember(
        "name", rapidjson::Value(referenceDevice.getName().c_str(), jsonTree.GetAllocator()),
        jsonTree.GetAllocator());

    if (referenceDevice.getBodyPartMask().has_value()) {
      jsonReferenceDevice.AddMember("body_part_mask", referenceDevice.getBodyPartMask().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMaximumFrequency().has_value()) {
      jsonReferenceDevice.AddMember("maximum_frequency",
                                    referenceDevice.getMaximumFrequency().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMinimumFrequency().has_value()) {
      jsonReferenceDevice.AddMember("minimum_frequency",
                                    referenceDevice.getMinimumFrequency().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getResonanceFrequency().has_value()) {
      jsonReferenceDevice.AddMember("resonance_frequency",
                                    referenceDevice.getResonanceFrequency().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMaximumAmplitude().has_value()) {
      jsonReferenceDevice.AddMember("maximum_amplitude",
                                    referenceDevice.getMaximumAmplitude().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getImpedance().has_value()) {
      jsonReferenceDevice.AddMember("impedance", referenceDevice.getImpedance().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMaximumVoltage().has_value()) {
      jsonReferenceDevice.AddMember("maximum_voltage", referenceDevice.getMaximumVoltage().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMaximumCurrent().has_value()) {
      jsonReferenceDevice.AddMember("maximum_current", referenceDevice.getMaximumCurrent().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getMaximumDisplacement().has_value()) {
      jsonReferenceDevice.AddMember("maximum_displacement",
                                    referenceDevice.getMaximumDisplacement().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getWeight().has_value()) {
      jsonReferenceDevice.AddMember("weight", referenceDevice.getWeight().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getSize().has_value()) {
      jsonReferenceDevice.AddMember("size", referenceDevice.getSize().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getCustom().has_value()) {
      jsonReferenceDevice.AddMember("custom", referenceDevice.getCustom().value(),
                                    jsonTree.GetAllocator());
    }
    if (referenceDevice.getType().has_value()) {
      jsonReferenceDevice.AddMember(
          "type",
          rapidjson::Value(
              types::actuatorTypeToString.at(referenceDevice.getType().value()).c_str(),
              jsonTree.GetAllocator()),
          jsonTree.GetAllocator());
    }

    jsonReferenceDevices.PushBack(jsonReferenceDevice, jsonTree.GetAllocator());
  }
}

auto IOJson::extractVector(types::Vector &vector, rapidjson::Value &jsonVector,
                           rapidjson::Document &jsonTree) -> void {
  jsonVector = rapidjson::Value(rapidjson::kObjectType);
  jsonVector.AddMember("X", vector.X, jsonTree.GetAllocator());
  jsonVector.AddMember("Y", vector.Y, jsonTree.GetAllocator());
  jsonVector.AddMember("Z", vector.Z, jsonTree.GetAllocator());
}

auto IOJson::bytes2bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void {
  out.resize(in.size() * BYTE_SIZE_IO);
  int index = 0;
  for (auto &v : in) {
    std::bitset<BYTE_SIZE_IO> temp((unsigned long)v);
    for (int j = 0; j < BYTE_SIZE_IO; j++) {
      if (temp[j]) {
        out.at(index) = 1;
      }
      index++;
    }
  }
}

auto IOJson::bits2bytes(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void {
  out.resize(ceil((double)in.size() / BYTE_SIZE_IO));
  size_t index = 0;
  for (auto &v : out) {
    std::bitset<BYTE_SIZE_IO> temp;
    for (int j = 0; j < BYTE_SIZE_IO; j++) {
      if (index >= in.size()) {
        break;
      }
      if (in.at(index) == 1) {
        temp[j] = true;
      }
      index++;
    }
    v = (unsigned char)temp.to_ulong();
  }
}

auto IOJson::base642bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void {
  out.reserve(in.size() * 6);
  int index = 0;
  for (auto &v : in) {
    auto temp = (short)v;
    if (ASCII_UPPER_1 <= temp && temp <= ASCII_UPPER_2) {
      temp -= DIFF_UPPER;
    } else if (ASCII_LOWER_1 <= temp && temp <= ASCII_LOWER_2) {
      temp -= DIFF_LOWER;
    } else if (ASCII_DIGIT_1 <= temp && temp <= ASCII_DIGIT_2) {
      temp += DIFF_DIGIT;
    } else if (temp == ASCII_PLUS) {
      temp = BASE64_PLUS;
    } else {
      temp = BASE64_SOLIDUS;
    }

    short compare = COMPARE_START;
    for (int j = 0; j < BASE64_SIZE; j++) {
      if (temp >= compare) {
        out.push_back((unsigned char)1);
        temp = temp - compare;
      } else {
        out.push_back((unsigned char)0);
      }
      index++;
      compare = (short)compare >> 1;
    }
  }
}

auto IOJson::bits2base64(std::vector<unsigned char> &in, std::vector<unsigned char> &out) -> void {
  out.reserve(in.size() / BASE64_SIZE);
  short compare = COMPARE_START;
  short temp = 0;
  for (auto &v : in) {
    if ((short)v == 1) {
      temp += compare;
    }
    compare = (short)compare >> 1;
    if (compare == 0) {
      compare = COMPARE_START;
      if (BASE64_UPPER_1 <= temp && temp <= BASE64_UPPER_2) {
        temp += DIFF_UPPER;
      } else if (BASE64_LOWER_1 <= temp && temp <= BASE64_LOWER_2) {
        temp += DIFF_LOWER;
      } else if (BASE64_DIGIT_1 <= temp && temp <= BASE64_DIGIT_2) {
        temp -= DIFF_DIGIT;
      } else if (temp == BASE64_PLUS) {
        temp = ASCII_PLUS;
      } else {
        temp = ASCII_SOLIDUS;
      }
      out.push_back(temp);
      temp = 0;
    }
  }
}

} // namespace haptics::io
