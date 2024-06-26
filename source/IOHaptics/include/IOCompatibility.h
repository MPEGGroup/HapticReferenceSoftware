#ifndef IOCOMPATIBILITY_H
#define IOCOMPATIBILITY_H

#include <Types/include/Haptics.h>
#include <map>
#include <string>
#include <vector>

namespace haptics::io {

using namespace haptics::types;

static constexpr int MAX_8_BITS = 255;
static constexpr int MAX_7_BITS = 127;
static constexpr int MAX_15_BITS = 65535;
static constexpr int MAX_23_BITS = 16777215;
static constexpr int MAX_31_BITS = 4294967295;
static constexpr float TEN_K = 10000;

enum class hjifErrorCode {
  Experience_Description_Size_OutOfRange,
  Experience_Timescale_OutOfRange,
  Experience_Avatars_Size_OutOfRange,
  Experience_Perceptions_Size_OutOfRange,
  Avatar_ID_OutOfRange,
  Avatar_Mesh_Size_OutOfRange,
  Perception_ID_OutOfRange,
  Perception_Description_Size_OutOfRange,
  Perception_EffectLibrary_Size_OutOfRange,
  Perception_SemanticScheme_Size_OutOfRange,
  Perception_ReferenceDevices_Size_OutOfRange,
  Perception_Channels_Size_OutOfRange,
  Perception_UnitExponent_OutOfRange,
  Perception_PerceptionUnitExponent_OutOfRange,
  Sync_Timestamp_OutOfRange,
  Sync_Timescale_OutOfRange,
  ReferenceDevice_ID_OutOfRange,
  ReferenceDevice_Name_Size_OutOfRange,
  ReferenceDevice_BodyPartMask_OutOfRange,
  ReferenceDevice_MaximumFrequency_OutOfRange,
  ReferenceDevice_MinimumFrequency_OutOfRange,
  ReferenceDevice_ResonanceFrequency_OutOfRange,
  ReferenceDevice_MaximumAmplitude_OutOfRange,
  ReferenceDevice_Impedance_OutOfRange,
  ReferenceDevice_MaximumVoltage_OutOfRange,
  ReferenceDevice_MaximumCurrent_OutOfRange,
  ReferenceDevice_MaximumDisplacement_OutOfRange,
  ReferenceDevice_Weight_OutOfRange,
  ReferenceDevice_Size_OutOfRange,
  ReferenceDevice_Custom_OutOfRange,
  Channel_ID_OutOfRange,
  Channel_Description_Size_OutOfRange,
  Channel_Gain_OutOfRange,
  Channel_MixingWeight_OutOfRange,
  Channel_BodyPartMask_OutOfRange,
  Channel_FrequencySampling_OutOfRange,
  Channel_SampleCount_OutOfRange,
  Channel_Vertices_Size_OutOfRange,
  Channel_Bands_Size_OutOfRange,
  Band_Effects_Size_OutOfRange,
  Effect_ID_OutOfRange,
  Effect_Position_OutOfRange,
  Effect_Composition_Size_OutOfRange,
  Effect_Keyframes_Size_OutOfRange,
  Keyframe_RelativePosition_OutOfRange
};

static const std::map<hjifErrorCode, std::string> hjifErrorCodeToString = {
    // Init_Experience_*
    {hjifErrorCode::Experience_Description_Size_OutOfRange,
     "Experience Error: description too long. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Description_Size_OutOfRange))},
    {hjifErrorCode::Experience_Timescale_OutOfRange,
     "Experience Error: timescale out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Timescale_OutOfRange))},
    {hjifErrorCode::Experience_Avatars_Size_OutOfRange,
     "Experience Error: avatars vector size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Avatars_Size_OutOfRange))},
    {hjifErrorCode::Experience_Perceptions_Size_OutOfRange,
     "Experience Error: perceptions vector size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Perceptions_Size_OutOfRange))},
    {hjifErrorCode::Avatar_ID_OutOfRange,
     "Avatar Error: ID out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Avatar_ID_OutOfRange))},
    {hjifErrorCode::Avatar_Mesh_Size_OutOfRange,
     "Avatar Error: mesh vector size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Avatar_Mesh_Size_OutOfRange))},
    {hjifErrorCode::Perception_ID_OutOfRange,
     "Perception Error: ID out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Perception_ID_OutOfRange))},
    {hjifErrorCode::Perception_Description_Size_OutOfRange,
     "Perception Error: description too long. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Perception_Description_Size_OutOfRange))},
    {hjifErrorCode::Perception_EffectLibrary_Size_OutOfRange,
     "Perception Error: effect library size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Perception_EffectLibrary_Size_OutOfRange))},
    {hjifErrorCode::Perception_SemanticScheme_Size_OutOfRange,
     "Perception Error: semantic scheme size out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::Perception_SemanticScheme_Size_OutOfRange))},
    {hjifErrorCode::Perception_ReferenceDevices_Size_OutOfRange,
     "Perception Error: reference devices vector size out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::Perception_ReferenceDevices_Size_OutOfRange))},
    {hjifErrorCode::Perception_Channels_Size_OutOfRange,
     "Perception Error: channels vector size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Perception_Channels_Size_OutOfRange))},
    {hjifErrorCode::Perception_UnitExponent_OutOfRange,
     "Perception Error: unit exponent out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Perception_UnitExponent_OutOfRange))},
    {hjifErrorCode::Perception_PerceptionUnitExponent_OutOfRange,
     "Perception Error: perception unit exponent out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::Perception_PerceptionUnitExponent_OutOfRange))},
    {hjifErrorCode::Sync_Timestamp_OutOfRange,
     "Sync Error: timestamp out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Sync_Timestamp_OutOfRange))},
    {hjifErrorCode::Sync_Timescale_OutOfRange,
     "Sync Error: timescale out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Sync_Timescale_OutOfRange))},
    {hjifErrorCode::Sync_Timescale_OutOfRange,
     "Sync Error: timescale out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Sync_Timescale_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_ID_OutOfRange,
     "ReferenceDevice Error: ID out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_ID_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_Name_Size_OutOfRange,
     "ReferenceDevice Error: name size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_Name_Size_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_BodyPartMask_OutOfRange,
     "ReferenceDevice Error: bodyPartMask out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_BodyPartMask_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MaximumFrequency_OutOfRange,
     "ReferenceDevice Error: maximum frequency out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MaximumFrequency_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MinimumFrequency_OutOfRange,
     "ReferenceDevice Error: minimum frequency out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MinimumFrequency_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_ResonanceFrequency_OutOfRange,
     "ReferenceDevice Error: resonance frequency out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_ResonanceFrequency_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MaximumAmplitude_OutOfRange,
     "ReferenceDevice Error: maximum amplitude out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MaximumAmplitude_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_Impedance_OutOfRange,
     "ReferenceDevice Error: impedance out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_Impedance_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MaximumVoltage_OutOfRange,
     "ReferenceDevice Error: maximum voltage out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MaximumVoltage_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MaximumCurrent_OutOfRange,
     "ReferenceDevice Error: maximum current out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MaximumCurrent_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_MaximumDisplacement_OutOfRange,
     "ReferenceDevice Error: maximum displacement out of range. Error code: " +
         std::to_string(
             static_cast<int>(hjifErrorCode::ReferenceDevice_MaximumDisplacement_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_Weight_OutOfRange,
     "ReferenceDevice Error: weight out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_Weight_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_Size_OutOfRange,
     "ReferenceDevice Error: size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_Size_OutOfRange))},
    {hjifErrorCode::ReferenceDevice_Custom_OutOfRange,
     "ReferenceDevice Error: custom out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::ReferenceDevice_Custom_OutOfRange))},
    {hjifErrorCode::Channel_ID_OutOfRange,
     "Channel Error: ID out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_ID_OutOfRange))},
    {hjifErrorCode::Channel_Description_Size_OutOfRange,
     "Channel Error: description size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_Description_Size_OutOfRange))},
    {hjifErrorCode::Channel_Gain_OutOfRange,
     "Channel Error: gain out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_Gain_OutOfRange))},
    {hjifErrorCode::Channel_MixingWeight_OutOfRange,
     "Channel Error: mixingWeight out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_MixingWeight_OutOfRange))},
    {hjifErrorCode::Channel_BodyPartMask_OutOfRange,
     "Channel Error: bodyPartMask out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_BodyPartMask_OutOfRange))},
    {hjifErrorCode::Channel_FrequencySampling_OutOfRange,
     "Channel Error: frequencySampling out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_FrequencySampling_OutOfRange))},
    {hjifErrorCode::Channel_SampleCount_OutOfRange,
     "Channel Error: sampleCount out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_SampleCount_OutOfRange))},
    {hjifErrorCode::Channel_Vertices_Size_OutOfRange,
     "Channel Error: vertices size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_Vertices_Size_OutOfRange))},
    {hjifErrorCode::Channel_Bands_Size_OutOfRange,
     "Channel Error: bands size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Channel_Bands_Size_OutOfRange))},
    {hjifErrorCode::Band_Effects_Size_OutOfRange,
     "Band Error: effects size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Band_Effects_Size_OutOfRange))},
    {hjifErrorCode::Effect_ID_OutOfRange,
     "Band Error: effect ID out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Effect_ID_OutOfRange))},
    {hjifErrorCode::Effect_Position_OutOfRange,
     "Band Error: effect position out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Effect_Position_OutOfRange))},
    {hjifErrorCode::Effect_Composition_Size_OutOfRange,
     "Band Error: effect composition size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Effect_Composition_Size_OutOfRange))},
    {hjifErrorCode::Effect_Keyframes_Size_OutOfRange,
     "Band Error: effect keyframes size out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Effect_Keyframes_Size_OutOfRange))},
    {hjifErrorCode::Keyframe_RelativePosition_OutOfRange,
     "Band Error: keyframe relative position out of range. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Keyframe_RelativePosition_OutOfRange))}};

class IOCompatibility {
public:
  static auto checkExperience(types::Haptics &haptic, std::vector<std::string> &logs) -> void {

    if (haptic.getDescription().size() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Experience_Description_Size_OutOfRange));
    }

    if (haptic.getTimescaleOrDefault() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Experience_Timescale_OutOfRange));
    }

    if (haptic.getAvatarsSize() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Experience_Avatars_Size_OutOfRange));
    }

    if (haptic.getPerceptionsSize() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Experience_Perceptions_Size_OutOfRange));
    }
  };

  static auto checkAvatar(types::Avatar &avatar, std::vector<std::string> &logs) -> void {
    if (avatar.getId() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Avatar_ID_OutOfRange));
    }

    auto mesh = avatar.getMesh();
    if (mesh.has_value()) {
      if (mesh.value().size() > MAX_8_BITS) {
        logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Avatar_ID_OutOfRange));
      }
    }
  };

  static auto checkPerception(types::Perception &perception, std::vector<std::string> &logs)
      -> void {
    if (perception.getId() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Perception_ID_OutOfRange));
    }

    if (perception.getDescription().size() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Perception_Description_Size_OutOfRange));
    }

    if (perception.getEffectLibrarySize() > MAX_15_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Perception_EffectLibrary_Size_OutOfRange));
    }

    if (perception.getEffectSemanticSchemeOrDefault().size() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Perception_SemanticScheme_Size_OutOfRange));
    }

    if (perception.getReferenceDevicesSize() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Perception_ReferenceDevices_Size_OutOfRange));
    }

    auto unitExponent = perception.getUnitExponentOrDefault();
    if (unitExponent > MAX_7_BITS || unitExponent < -MAX_7_BITS - 1) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Perception_UnitExponent_OutOfRange));
    }

    auto perceptionUnitExponent = perception.getPerceptionUnitExponentOrDefault();
    if (perceptionUnitExponent > MAX_7_BITS || perceptionUnitExponent < -MAX_7_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Perception_PerceptionUnitExponent_OutOfRange));
    }
  };

  static auto checkSync(types::Sync &sync, std::vector<std::string> &logs) -> void {
    if (sync.getTimestamp() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Sync_Timestamp_OutOfRange));
    }

    if (sync.getTimescaleOrDefault() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Sync_Timescale_OutOfRange));
    }
  }

  static auto checkHapticDevice(types::ReferenceDevice &device, std::vector<std::string> &logs)
      -> void {
    if (device.getId() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_ID_OutOfRange));
    }

    if (device.getName().size() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Name_Size_OutOfRange));
    }

    auto bodyPartMask = device.getBodyPartMask();
    if (bodyPartMask.has_value()) {
      if (bodyPartMask.value() > MAX_31_BITS) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Name_Size_OutOfRange));
      }
    }

    auto maximumFrequency = device.getMaximumFrequency();
    if (maximumFrequency.has_value()) {
      if (maximumFrequency.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_MaximumFrequency_OutOfRange));
      }
    }

    auto minimumFrequency = device.getMinimumFrequency();
    if (minimumFrequency.has_value()) {
      if (minimumFrequency.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_MinimumFrequency_OutOfRange));
      }
    }

    auto resonanceFrequency = device.getResonanceFrequency();
    if (resonanceFrequency.has_value()) {
      if (resonanceFrequency.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_ResonanceFrequency_OutOfRange));
      }
    }

    auto maximumAmplitude = device.getMaximumAmplitude();
    if (maximumAmplitude.has_value()) {
      if (maximumAmplitude.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_MaximumAmplitude_OutOfRange));
      }
    }

    auto impedance = device.getImpedance();
    if (impedance.has_value()) {
      if (impedance.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Impedance_OutOfRange));
      }
    }

    auto maximumVoltage = device.getMaximumVoltage();
    if (maximumVoltage.has_value()) {
      if (maximumVoltage.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_MaximumVoltage_OutOfRange));
      }
    }

    auto maximumCurrent = device.getMaximumCurrent();
    if (maximumCurrent.has_value()) {
      if (maximumCurrent.value() > TEN_K) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_MaximumCurrent_OutOfRange));
      }
    }

    auto maximumDisplacement = device.getMaximumDisplacement();
    if (maximumDisplacement.has_value()) {
      if (maximumDisplacement.value() > TEN_K) {
        logs.push_back(hjifErrorCodeToString.at(
            hjifErrorCode::ReferenceDevice_MaximumDisplacement_OutOfRange));
      }
    }

    auto weight = device.getWeight();
    if (weight.has_value()) {
      if (weight.value() > TEN_K) {
        logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Weight_OutOfRange));
      }
    }

    auto size = device.getSize();
    if (size.has_value()) {
      if (size.value() > TEN_K) {
        logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Size_OutOfRange));
      }
    }

    auto custom = device.getCustom();
    if (custom.has_value()) {
      if (custom.value() > TEN_K || custom.value() < -TEN_K) {
        logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::ReferenceDevice_Custom_OutOfRange));
      }
    }
  }

  static auto checkChannel(types::Channel &channel, std::vector<std::string> &logs) -> void {
    if (channel.getId() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_ID_OutOfRange));
    }

    if (channel.getDescription().size() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_Description_Size_OutOfRange));
    }

    auto gain = channel.getGain();
    if (gain > TEN_K || gain < TEN_K) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_Gain_OutOfRange));
    }

    if (channel.getMixingWeight() > TEN_K) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_MixingWeight_OutOfRange));
    }

    if (channel.getBodyPartMask() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_BodyPartMask_OutOfRange));
    }

    if (channel.getFrequencySampling() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_FrequencySampling_OutOfRange));
    }

    if (channel.getSampleCount() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_SampleCount_OutOfRange));
    }

    if (channel.getVerticesSize() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_Vertices_Size_OutOfRange));
    }

    if (channel.getBandsSize() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Channel_Bands_Size_OutOfRange));
    }
  }

  static auto checkBand(types::Band &band, std::vector<std::string> &logs) -> void {
    if (band.getEffectsSize() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Band_Effects_Size_OutOfRange));
    }
  }

  static auto checkEffect(types::Effect &effect, std::vector<std::string> &logs) -> void {
    if (effect.getId() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Effect_ID_OutOfRange));
    }

    auto position = effect.getPosition();
    if (position > MAX_23_BITS || position < -MAX_23_BITS-1) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Effect_Position_OutOfRange));
    }

    if (effect.getKeyframesSize() > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Effect_Keyframes_Size_OutOfRange));
    }

    if (effect.getTimelineSize()  > MAX_15_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Effect_Composition_Size_OutOfRange));
    }
  }

  static auto checkKeyframe(types::Keyframe &keyframe, std::vector<std::string> &logs) -> void {
    auto position = keyframe.getRelativePosition();
    if (position.has_value()) {
      if (position.value() > MAX_15_BITS) {
        logs.push_back(
            hjifErrorCodeToString.at(hjifErrorCode::Keyframe_RelativePosition_OutOfRange));
      }
    }
  }
};
} // namespace haptics::io
#endif