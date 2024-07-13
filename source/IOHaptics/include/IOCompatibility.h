#ifndef IOCOMPATIBILITY_H
#define IOCOMPATIBILITY_H

#include <Types/include/Haptics.h>
#include <map>
#include <string>
#include <vector>

namespace haptics::io {

static constexpr int MAX_8_BITS_SIGNED = 127;
static constexpr int MIN_8_BITS_SIGNED = -128;
static constexpr unsigned int MAX_8_BITS_UNSIGNED = 255;
static constexpr unsigned int MAX_16_BITS_UNSIGNED = 65535;
static constexpr int MAX_25_BITS_SIGNED = 16777215;
static constexpr int MIN_25_BITS_SIGNED = -16777216;
static constexpr unsigned int MAX_32_BITS_UNSIGNED = 4294967295;
static constexpr float TEN_K = 10000;

enum class hjifWarningCode {
  Experience_Description_Size_OutOfRange,
  Experience_Timescale_OutOfRange,
  Experience_Avatars_Size_OutOfRange,
  Experience_Perceptions_Size_OutOfRange,
  Avatar_ID_OutOfRange,
  Avatar_LOD_OutOfRange,
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
  Channel_MixingCoefficient_OutOfRange,
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

static const std::map<hjifWarningCode, std::string> hjifWarningCodeToString = {
    // Init_Experience_*
    {hjifWarningCode::Experience_Description_Size_OutOfRange,
     "Experience Warning: description too long. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Experience_Description_Size_OutOfRange))},
    {hjifWarningCode::Experience_Timescale_OutOfRange,
     "Experience Warning: timescale out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Experience_Timescale_OutOfRange))},
    {hjifWarningCode::Experience_Avatars_Size_OutOfRange,
     "Experience Warning: avatars vector size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Experience_Avatars_Size_OutOfRange))},
    {hjifWarningCode::Experience_Perceptions_Size_OutOfRange,
     "Experience Warning: perceptions vector size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Experience_Perceptions_Size_OutOfRange))},
    {hjifWarningCode::Avatar_ID_OutOfRange,
     "Avatar Warning: ID out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Avatar_ID_OutOfRange))},
    {hjifWarningCode::Avatar_LOD_OutOfRange,
     "Avatar Warning: LOD out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Avatar_LOD_OutOfRange))},
    {hjifWarningCode::Avatar_Mesh_Size_OutOfRange,
     "Avatar Warning: mesh vector size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Avatar_Mesh_Size_OutOfRange))},
    {hjifWarningCode::Perception_ID_OutOfRange,
     "Perception Warning: ID out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Perception_ID_OutOfRange))},
    {hjifWarningCode::Perception_Description_Size_OutOfRange,
     "Perception Warning: description too long. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Perception_Description_Size_OutOfRange))},
    {hjifWarningCode::Perception_EffectLibrary_Size_OutOfRange,
     "Perception Warning: effect library size out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::Perception_EffectLibrary_Size_OutOfRange))},
    {hjifWarningCode::Perception_SemanticScheme_Size_OutOfRange,
     "Perception Warning: semantic scheme size out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::Perception_SemanticScheme_Size_OutOfRange))},
    {hjifWarningCode::Perception_ReferenceDevices_Size_OutOfRange,
     "Perception Warning: reference devices vector size out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::Perception_ReferenceDevices_Size_OutOfRange))},
    {hjifWarningCode::Perception_Channels_Size_OutOfRange,
     "Perception Warning: channels vector size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Perception_Channels_Size_OutOfRange))},
    {hjifWarningCode::Perception_UnitExponent_OutOfRange,
     "Perception Warning: unit exponent out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Perception_UnitExponent_OutOfRange))},
    {hjifWarningCode::Perception_PerceptionUnitExponent_OutOfRange,
     "Perception Warning: perception unit exponent out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::Perception_PerceptionUnitExponent_OutOfRange))},
    {hjifWarningCode::Sync_Timestamp_OutOfRange,
     "Sync Warning: timestamp out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Sync_Timestamp_OutOfRange))},
    {hjifWarningCode::Sync_Timescale_OutOfRange,
     "Sync Warning: timescale out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Sync_Timescale_OutOfRange))},
    {hjifWarningCode::Sync_Timescale_OutOfRange,
     "Sync Warning: timescale out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Sync_Timescale_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_ID_OutOfRange,
     "ReferenceDevice Warning: ID out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_ID_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_Name_Size_OutOfRange,
     "ReferenceDevice Warning: name size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_Name_Size_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_BodyPartMask_OutOfRange,
     "ReferenceDevice Warning: bodyPartMask out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_BodyPartMask_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MaximumFrequency_OutOfRange,
     "ReferenceDevice Warning: maximum frequency out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MaximumFrequency_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MinimumFrequency_OutOfRange,
     "ReferenceDevice Warning: minimum frequency out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MinimumFrequency_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_ResonanceFrequency_OutOfRange,
     "ReferenceDevice Warning: resonance frequency out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_ResonanceFrequency_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MaximumAmplitude_OutOfRange,
     "ReferenceDevice Warning: maximum amplitude out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MaximumAmplitude_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_Impedance_OutOfRange,
     "ReferenceDevice Warning: impedance out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_Impedance_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MaximumVoltage_OutOfRange,
     "ReferenceDevice Warning: maximum voltage out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MaximumVoltage_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MaximumCurrent_OutOfRange,
     "ReferenceDevice Warning: maximum current out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MaximumCurrent_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_MaximumDisplacement_OutOfRange,
     "ReferenceDevice Warning: maximum displacement out of range. Warning code: " +
         std::to_string(
             static_cast<int>(hjifWarningCode::ReferenceDevice_MaximumDisplacement_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_Weight_OutOfRange,
     "ReferenceDevice Warning: weight out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_Weight_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_Size_OutOfRange,
     "ReferenceDevice Warning: size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_Size_OutOfRange))},
    {hjifWarningCode::ReferenceDevice_Custom_OutOfRange,
     "ReferenceDevice Warning: custom out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::ReferenceDevice_Custom_OutOfRange))},
    {hjifWarningCode::Channel_ID_OutOfRange,
     "Channel Warning: ID out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_ID_OutOfRange))},
    {hjifWarningCode::Channel_Description_Size_OutOfRange,
     "Channel Warning: description size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_Description_Size_OutOfRange))},
    {hjifWarningCode::Channel_Gain_OutOfRange,
     "Channel Warning: gain out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_Gain_OutOfRange))},
    {hjifWarningCode::Channel_MixingCoefficient_OutOfRange,
     "Channel Warning: mixingCoefficient out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_MixingCoefficient_OutOfRange))},
    {hjifWarningCode::Channel_BodyPartMask_OutOfRange,
     "Channel Warning: bodyPartMask out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_BodyPartMask_OutOfRange))},
    {hjifWarningCode::Channel_FrequencySampling_OutOfRange,
     "Channel Warning: frequencySampling out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_FrequencySampling_OutOfRange))},
    {hjifWarningCode::Channel_SampleCount_OutOfRange,
     "Channel Warning: sampleCount out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_SampleCount_OutOfRange))},
    {hjifWarningCode::Channel_Vertices_Size_OutOfRange,
     "Channel Warning: vertices size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_Vertices_Size_OutOfRange))},
    {hjifWarningCode::Channel_Bands_Size_OutOfRange,
     "Channel Warning: bands size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Channel_Bands_Size_OutOfRange))},
    {hjifWarningCode::Band_Effects_Size_OutOfRange,
     "Band Warning: effects size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Band_Effects_Size_OutOfRange))},
    {hjifWarningCode::Effect_ID_OutOfRange,
     "Band Warning: effect ID out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Effect_ID_OutOfRange))},
    {hjifWarningCode::Effect_Position_OutOfRange,
     "Band Warning: effect position out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Effect_Position_OutOfRange))},
    {hjifWarningCode::Effect_Composition_Size_OutOfRange,
     "Band Warning: effect composition size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Effect_Composition_Size_OutOfRange))},
    {hjifWarningCode::Effect_Keyframes_Size_OutOfRange,
     "Band Warning: effect keyframes size out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Effect_Keyframes_Size_OutOfRange))},
    {hjifWarningCode::Keyframe_RelativePosition_OutOfRange,
     "Band Warning: keyframe relative position out of range. Warning code: " +
         std::to_string(static_cast<int>(hjifWarningCode::Keyframe_RelativePosition_OutOfRange))}};

class IOCompatibility {
public:
  static auto checkHaptics(types::Haptics &haptic) -> std::vector<std::string> {
    std::vector<std::string> logs;
    checkExperience(haptic, logs);
    for (int a = 0; a < static_cast<int>(haptic.getAvatarsSize()); a++) {
      auto avatar = haptic.getAvatarAt(a);
      checkAvatar(avatar, logs);
    }
    for (int s = 0; s < static_cast<int>(haptic.getSyncsSize()); s++) {
      auto sync = haptic.getSyncsAt(s);
      checkSync(sync, logs);
    }
    for (int p = 0; p < static_cast<int>(haptic.getPerceptionsSize()); p++) {
      auto perception = haptic.getPerceptionAt(p);
      checkPerception(perception, logs);
      for (int l = 0; l < static_cast<int>(perception.getEffectLibrarySize()); l++) {
        auto effect = perception.getBasisEffectAt(l);
        checkEffect(effect, logs);
        checkEffectKeyframes(effect, logs);
      }
      for (int d = 0; d < static_cast<int>(perception.getReferenceDevicesSize()); d++) {
        auto device = perception.getReferenceDeviceAt(d);
        checkHapticDevice(device, logs);
      }
      for (int c = 0; c < static_cast<int>(perception.getChannelsSize()); c++) {
        auto channel = perception.getChannelAt(c);
        checkChannel(channel, logs);
        for (int b = 0; b < static_cast<int>(channel.getBandsSize()); b++) {
          auto band = channel.getBandAt(b);
          checkBand(band, logs);
          for (int e = 0; e < static_cast<int>(band.getEffectsSize()); e++) {
            auto effect = band.getEffectAt(e);
            checkEffect(effect, logs);
            checkEffectKeyframes(effect, logs);
          }
        }
      }
    }
    return logs;
  }

  static auto checkEffectKeyframes(types::Effect &effect, std::vector<std::string> &logs) -> void {
    for (int k = 0; k < static_cast<int>(effect.getKeyframesSize()); k++) {
      auto keyframe = effect.getKeyframeAt(k);
      checkKeyframe(keyframe, logs);
    }
  }

  static auto checkExperience(types::Haptics &haptic, std::vector<std::string> &logs) -> void {

    if (haptic.getDescription().size() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Experience_Description_Size_OutOfRange));
    }

    if (haptic.getTimescaleOrDefault() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Experience_Timescale_OutOfRange));
    }

    if (haptic.getAvatarsSize() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Experience_Avatars_Size_OutOfRange));
    }

    if (haptic.getPerceptionsSize() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Experience_Perceptions_Size_OutOfRange));
    }
  };

  static auto checkAvatar(types::Avatar &avatar, std::vector<std::string> &logs) -> void {
    if (avatar.getId() > static_cast<int>(MAX_8_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Avatar_ID_OutOfRange));
    }

    if (avatar.getLod() > static_cast<int>(MAX_8_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Avatar_LOD_OutOfRange));
    }

    auto mesh = avatar.getMesh();
    if (mesh.has_value()) {
      if (mesh.value().size() > MAX_8_BITS_UNSIGNED) {
        logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Avatar_Mesh_Size_OutOfRange));
      }
    }
  };

  static auto checkPerception(types::Perception &perception, std::vector<std::string> &logs)
      -> void {
    if (perception.getId() > static_cast<int>(MAX_8_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Perception_ID_OutOfRange));
    }

    if (perception.getDescription().size() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Perception_Description_Size_OutOfRange));
    }

    if (perception.getEffectLibrarySize() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Perception_EffectLibrary_Size_OutOfRange));
    }

    if (perception.getEffectSemanticSchemeOrDefault().size() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Perception_SemanticScheme_Size_OutOfRange));
    }

    if (perception.getReferenceDevicesSize() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Perception_ReferenceDevices_Size_OutOfRange));
    }

    auto unitExponent = perception.getUnitExponentOrDefault();
    if (unitExponent > MAX_8_BITS_SIGNED || unitExponent < MIN_8_BITS_SIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Perception_UnitExponent_OutOfRange));
    }

    auto perceptionUnitExponent = perception.getPerceptionUnitExponentOrDefault();
    if (perceptionUnitExponent > MAX_8_BITS_SIGNED || perceptionUnitExponent < MIN_8_BITS_SIGNED) {
      logs.push_back(hjifWarningCodeToString.at(
          hjifWarningCode::Perception_PerceptionUnitExponent_OutOfRange));
    }
  };

  static auto checkSync(types::Sync &sync, std::vector<std::string> &logs) -> void {
    if (static_cast<unsigned int>(sync.getTimestamp()) > MAX_32_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Sync_Timestamp_OutOfRange));
    }

    if (sync.getTimescaleOrDefault() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Sync_Timescale_OutOfRange));
    }
  }

  static auto checkHapticDevice(types::ReferenceDevice &device, std::vector<std::string> &logs)
      -> void {
    if (device.getId() > static_cast<int>(MAX_8_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_ID_OutOfRange));
    }

    if (device.getName().size() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Name_Size_OutOfRange));
    }

    auto bodyPartMask = device.getBodyPartMask();
    if (bodyPartMask.has_value()) {
      if (bodyPartMask.value() > MAX_32_BITS_UNSIGNED) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Name_Size_OutOfRange));
      }
    }

    auto maximumFrequency = device.getMaximumFrequency();
    if (maximumFrequency.has_value()) {
      if (maximumFrequency.value() > TEN_K) {
        logs.push_back(hjifWarningCodeToString.at(
            hjifWarningCode::ReferenceDevice_MaximumFrequency_OutOfRange));
      }
    }

    auto minimumFrequency = device.getMinimumFrequency();
    if (minimumFrequency.has_value()) {
      if (minimumFrequency.value() > TEN_K) {
        logs.push_back(hjifWarningCodeToString.at(
            hjifWarningCode::ReferenceDevice_MinimumFrequency_OutOfRange));
      }
    }

    auto resonanceFrequency = device.getResonanceFrequency();
    if (resonanceFrequency.has_value()) {
      if (resonanceFrequency.value() > TEN_K) {
        logs.push_back(hjifWarningCodeToString.at(
            hjifWarningCode::ReferenceDevice_ResonanceFrequency_OutOfRange));
      }
    }

    auto maximumAmplitude = device.getMaximumAmplitude();
    if (maximumAmplitude.has_value()) {
      if (maximumAmplitude.value() > TEN_K) {
        logs.push_back(hjifWarningCodeToString.at(
            hjifWarningCode::ReferenceDevice_MaximumAmplitude_OutOfRange));
      }
    }

    auto impedance = device.getImpedance();
    if (impedance.has_value()) {
      if (impedance.value() > TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Impedance_OutOfRange));
      }
    }

    auto maximumVoltage = device.getMaximumVoltage();
    if (maximumVoltage.has_value()) {
      if (maximumVoltage.value() > TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_MaximumVoltage_OutOfRange));
      }
    }

    auto maximumCurrent = device.getMaximumCurrent();
    if (maximumCurrent.has_value()) {
      if (maximumCurrent.value() > TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_MaximumCurrent_OutOfRange));
      }
    }

    auto maximumDisplacement = device.getMaximumDisplacement();
    if (maximumDisplacement.has_value()) {
      if (maximumDisplacement.value() > TEN_K) {
        logs.push_back(hjifWarningCodeToString.at(
            hjifWarningCode::ReferenceDevice_MaximumDisplacement_OutOfRange));
      }
    }

    auto weight = device.getWeight();
    if (weight.has_value()) {
      if (weight.value() > TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Weight_OutOfRange));
      }
    }

    auto size = device.getSize();
    if (size.has_value()) {
      if (size.value() > TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Size_OutOfRange));
      }
    }

    auto custom = device.getCustom();
    if (custom.has_value()) {
      if (custom.value() > TEN_K || custom.value() < -TEN_K) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::ReferenceDevice_Custom_OutOfRange));
      }
    }
  }

  static auto checkChannel(types::Channel &channel, std::vector<std::string> &logs) -> void {
    if (channel.getId() > static_cast<int>(MAX_16_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_ID_OutOfRange));
    }

    if (channel.getDescription().size() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Channel_Description_Size_OutOfRange));
    }

    auto gain = channel.getGain();
    if (gain > TEN_K || gain < -TEN_K) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_Gain_OutOfRange));
    }

    if (channel.getMixingWeight() > TEN_K) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_MixingCoefficient_OutOfRange));
    }

    if (channel.getBodyPartMask() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_BodyPartMask_OutOfRange));
    }

    if (channel.getFrequencySampling() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Channel_FrequencySampling_OutOfRange));
    }

    if (channel.getSampleCount() > MAX_32_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_SampleCount_OutOfRange));
    }

    if (channel.getVerticesSize() > MAX_16_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_Vertices_Size_OutOfRange));
    }

    if (channel.getBandsSize() > MAX_8_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Channel_Bands_Size_OutOfRange));
    }
  }

  static auto checkBand(types::Band &band, std::vector<std::string> &logs) -> void {
    if (band.getEffectsSize() > MAX_16_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Band_Effects_Size_OutOfRange));
    }
  }

  static auto checkEffect(types::Effect &effect, std::vector<std::string> &logs) -> void {
    if (effect.getId() > 0 && effect.getId() > static_cast<int>(MAX_16_BITS_UNSIGNED)) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Effect_ID_OutOfRange));
    }

    auto position = effect.getPosition();
    if (position > MAX_25_BITS_SIGNED || position < MIN_25_BITS_SIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Effect_Position_OutOfRange));
    }

    if (effect.getKeyframesSize() > MAX_16_BITS_UNSIGNED) {
      logs.push_back(hjifWarningCodeToString.at(hjifWarningCode::Effect_Keyframes_Size_OutOfRange));
    }

    if (effect.getTimelineSize() > MAX_16_BITS_UNSIGNED) {
      logs.push_back(
          hjifWarningCodeToString.at(hjifWarningCode::Effect_Composition_Size_OutOfRange));
    }
  }

  static auto checkKeyframe(types::Keyframe &keyframe, std::vector<std::string> &logs) -> void {
    auto position = keyframe.getRelativePosition();
    if (position.has_value()) {
      if (position.value() > static_cast<int>(MAX_16_BITS_UNSIGNED)) {
        logs.push_back(
            hjifWarningCodeToString.at(hjifWarningCode::Keyframe_RelativePosition_OutOfRange));
      }
    }
  }
};
} // namespace haptics::io
#endif