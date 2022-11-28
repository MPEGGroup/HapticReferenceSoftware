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

#include <IOHaptics/include/IOBinary.h>
#include <IOHaptics/include/IOBinaryBands.h>
#include <IOHaptics/include/IOBinaryFields.h>
#include <array>
#include <fstream>
#include <iostream>

namespace haptics::io {

auto IOBinary::loadMemory(IOMemoryBuffer &in, types::Haptics &out) -> bool {
  std::istream istr(&in);

  std::vector<bool> unusedBits;
  bool res = readFileHeader(out, istr, unusedBits);
  if (res) {
    readFileBody(out, istr, unusedBits);
  }
  return res;
}

auto IOBinary::loadFile(const std::string &filePath, types::Haptics &out) -> bool {
  std::ifstream file(filePath, std::ios::binary | std::ifstream::in);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    file.close();
    return false;
  }

  file.seekg(0, std::ios::end);
  unsigned int length = static_cast<unsigned int>(file.tellg());
  file.seekg(0, std::ios::beg);

  if (length == 0) { // avoid undefined behavior
    file.close();
    return false;
  }
  std::vector<bool> unusedBits;
  bool res = IOBinary::readFileHeader(out, file, unusedBits);
  if (res) {
    IOBinary::readFileBody(out, file, unusedBits);
  }

  file.close();
  return res;
}

auto IOBinary::writeFile(types::Haptics &haptic, const std::string &filePath) -> bool {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    return false;
  }
  std::vector<bool> output;
  bool res = IOBinary::writeFileHeader(haptic, output);
  if (res) {
    res = IOBinary::writeFileBody(haptic, output);
  }
  IOBinaryPrimitives::fillBitset(output);
  IOBinaryPrimitives::writeBitset(output, file);

  file.close();
  return res;
}

auto IOBinary::readFileHeader(types::Haptics &haptic, std::istream &file,
                              std::vector<bool> &unusedBits) -> bool {

  std::string version = IOBinaryPrimitives::readString(file, unusedBits);
  std::string date = IOBinaryPrimitives::readString(file, unusedBits);
  std::string description = IOBinaryPrimitives::readString(file, unusedBits);
  haptic.setVersion(version);
  haptic.setDate(date);
  haptic.setDescription(description);

  // Get avatars
  if (!IOBinary::readAvatars(haptic, file, unusedBits)) {
    return false;
  }

  // Get perceptions
  return IOBinary::readPerceptionsHeader(haptic, file, unusedBits);
}

auto IOBinary::writeFileHeader(types::Haptics &haptic, std::vector<bool> &output) -> bool {
  const std::string version = haptic.getVersion();
  const std::string date = haptic.getDate();
  const std::string description = haptic.getDescription();

  IOBinaryPrimitives::writeString(version, output);
  IOBinaryPrimitives::writeString(date, output);
  IOBinaryPrimitives::writeString(description, output);

  if (!IOBinary::writeAvatars(haptic, output)) {
    return false;
  }

  return IOBinary::writePerceptionsHeader(haptic, output);
}

auto IOBinary::readAvatars(types::Haptics &haptic, std::istream &file,
                           std::vector<bool> &unusedBits) -> bool {
  auto avatarCount =
      IOBinaryPrimitives::readNBits<unsigned short, MDEXP_AVATAR_COUNT>(file, unusedBits);

  types::Avatar myAvatar;
  for (unsigned short i = 0; i < avatarCount; i++) {
    auto avatarId = IOBinaryPrimitives::readNBits<short, AVATAR_ID>(file, unusedBits);
    auto avatarLod = IOBinaryPrimitives::readNBits<uint8_t, AVATAR_LOD>(file, unusedBits);
    auto avatarType = IOBinaryPrimitives::readNBits<uint8_t, AVATAR_TYPE>(file, unusedBits);

    std::string avatarURI;
    myAvatar = types::Avatar(avatarId, avatarLod, static_cast<types::AvatarType>(avatarType));
    if (myAvatar.getType() == types::AvatarType::Custom) {
      avatarURI = IOBinaryPrimitives::readString(file, unusedBits);
      myAvatar.setMesh(avatarURI);
    }

    haptic.addAvatar(myAvatar);
  }
  return true;
}

auto IOBinary::writeAvatars(types::Haptics &haptic, std::vector<bool> &output) -> bool {
  auto avatarCount = static_cast<unsigned short>(haptic.getAvatarsSize());
  IOBinaryPrimitives::writeNBits<unsigned short, MDEXP_AVATAR_COUNT>(avatarCount, output);
  types::Avatar myAvatar;
  for (unsigned short i = 0; i < avatarCount; i++) {
    myAvatar = haptic.getAvatarAt(i);

    auto avatarId = static_cast<short>(myAvatar.getId());
    IOBinaryPrimitives::writeNBits<short, AVATAR_ID>(avatarId, output);

    int avatarLod = myAvatar.getLod();
    IOBinaryPrimitives::writeNBits<uint8_t, AVATAR_LOD>(avatarLod, output);

    auto avatarType = static_cast<unsigned short>(myAvatar.getType());
    IOBinaryPrimitives::writeNBits<uint8_t, AVATAR_TYPE>(avatarType, output);

    if (myAvatar.getType() == types::AvatarType::Custom) {
      const std::string avatarURI = myAvatar.getMesh().value_or("");
      IOBinaryPrimitives::writeString(avatarURI, output);
    }
  }
  return true;
}

auto IOBinary::readPerceptionsHeader(types::Haptics &haptic, std::istream &file,
                                     std::vector<bool> &unusedBits) -> bool {
  auto perceptionCount =
      IOBinaryPrimitives::readNBits<unsigned short, MDEXP_PERC_COUNT>(file, unusedBits);

  types::Perception myPerception;
  for (unsigned short i = 0; i < perceptionCount; i++) {
    auto perceptionId = IOBinaryPrimitives::readNBits<short, MDPERCE_ID>(file, unusedBits);
    auto perceptionModality =
        IOBinaryPrimitives::readNBits<unsigned short, MDPERCE_MODALITY>(file, unusedBits);
    std::string perceptionDescription = IOBinaryPrimitives::readString(file, unusedBits);
    auto avatarId = IOBinaryPrimitives::readNBits<int, AVATAR_ID>(file, unusedBits);
    auto unitExponent = IOBinaryPrimitives::readNBits<int8_t, MDPERCE_UNIT_EXP>(file, unusedBits);
    auto perceptionUnitExponent =
        IOBinaryPrimitives::readNBits<int8_t, MDPERCE_PERCE_UNIT_EXP>(file, unusedBits);

    myPerception = types::Perception(perceptionId, avatarId, perceptionDescription,
                                     static_cast<types::PerceptionModality>(perceptionModality));
    myPerception.setUnitExponent(unitExponent);
    myPerception.setPerceptionUnitExponent(perceptionUnitExponent);
    if (!IOBinary::readLibrary(myPerception, file, unusedBits)) {
      return false;
    }
    if (!IOBinary::readReferenceDevices(myPerception, file, unusedBits)) {
      return false;
    }
    if (!IOBinary::readTracksHeader(myPerception, file, unusedBits)) {
      return false;
    }
    haptic.addPerception(myPerception);
  }

  return true;
}

auto IOBinary::writePerceptionsHeader(types::Haptics &haptic, std::vector<bool> &output) -> bool {
  auto perceptionCount = static_cast<unsigned short>(haptic.getPerceptionsSize());
  IOBinaryPrimitives::writeNBits<unsigned short, MDEXP_PERC_COUNT>(perceptionCount, output);
  types::Perception myPerception;
  for (unsigned short i = 0; i < perceptionCount; i++) {
    myPerception = haptic.getPerceptionAt(i);

    auto perceptionId = static_cast<short>(myPerception.getId());
    IOBinaryPrimitives::writeNBits<short, MDPERCE_ID>(perceptionId, output);

    auto perceptionModality = static_cast<unsigned short>(myPerception.getPerceptionModality());
    IOBinaryPrimitives::writeNBits<unsigned short, MDPERCE_MODALITY>(perceptionModality, output);

    std::string perceptionDescription = myPerception.getDescription();
    IOBinaryPrimitives::writeString(perceptionDescription, output);

    int avatarId = myPerception.getAvatarId();
    IOBinaryPrimitives::writeNBits<int, AVATAR_ID>(avatarId, output);

    int8_t unitExponent = myPerception.getUnitExponentOrDefault();
    IOBinaryPrimitives::writeNBits<int8_t, MDPERCE_UNIT_EXP>(unitExponent, output);

    int8_t perceptionUnitExponent = myPerception.getPerceptionUnitExponentOrDefault();
    IOBinaryPrimitives::writeNBits<int8_t, MDPERCE_PERCE_UNIT_EXP>(perceptionUnitExponent, output);

    if (!IOBinary::writeLibrary(myPerception, output)) {
      return false;
    }

    if (!IOBinary::writeReferenceDevices(myPerception, output)) {
      return false;
    }

    if (!IOBinary::writeTracksHeader(myPerception, output)) {
      return false;
    }
  }
  return true;
}

auto IOBinary::readLibraryEffect(std::istream &file, std::vector<bool> &unusedBits)
    -> types::Effect {
  auto id = IOBinaryPrimitives::readNBits<int, EFFECT_ID>(file, unusedBits);
  auto position = IOBinaryPrimitives::readNBits<int, EFFECT_POSITION>(file, unusedBits);
  auto phase =
      IOBinaryPrimitives::readFloatNBits<uint16_t, EFFECT_PHASE>(file, 0, MAX_PHASE, unusedBits);
  auto baseSignal = IOBinaryPrimitives::readNBits<uint8_t, EFFECT_BASE_SIGNAL>(file, unusedBits);
  auto effectType = IOBinaryPrimitives::readNBits<uint8_t, EFFECT_TYPE>(file, unusedBits);
  auto keyframeCount =
      IOBinaryPrimitives::readNBits<uint32_t, EFFECT_KEYFRAME_COUNT>(file, unusedBits);
  types::Effect effect(static_cast<int>(position), phase,
                       static_cast<types::BaseSignal>(baseSignal),
                       static_cast<types::EffectType>(effectType));
  effect.setId(id);
  for (unsigned int i = 0; i < keyframeCount; i++) {
    auto mask = IOBinaryPrimitives::readNBits<uint8_t, KEYFRAME_MASK>(file, unusedBits);
    std::optional<int> position = std::nullopt;
    std::optional<float> amplitude = std::nullopt;
    std::optional<int> frequency = std::nullopt;
    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      position = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_POSITION>(file, unusedBits);
    }

    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      amplitude = IOBinaryPrimitives::readFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
          file, -MAX_AMPLITUDE, MAX_AMPLITUDE, unusedBits);
    }

    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      frequency = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_FREQUENCY>(file, unusedBits);
    }
    types::Keyframe myKeyframe(position, amplitude, frequency);
    effect.addKeyframe(myKeyframe);
  }
  auto timelineEffectCount =
      IOBinaryPrimitives::readNBits<unsigned short, EFFECT_TIMELINE_COUNT>(file, unusedBits);
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    auto timelineEffect = readLibraryEffect(file, unusedBits);
    effect.addTimelineEffect(timelineEffect);
  }
  return effect;
}

auto IOBinary::writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &output) -> bool {
  int id = libraryEffect.getId();
  IOBinaryPrimitives::writeNBits<int, EFFECT_ID>(id, output);
  int position = libraryEffect.getPosition();
  IOBinaryPrimitives::writeNBits<int, EFFECT_POSITION>(position, output);
  float phase = libraryEffect.getPhase();
  IOBinaryPrimitives::writeFloatNBits<uint16_t, EFFECT_PHASE>(phase, output, 0, MAX_PHASE);
  auto baseSignal = static_cast<uint8_t>(libraryEffect.getBaseSignal());
  IOBinaryPrimitives::writeNBits<uint8_t, EFFECT_BASE_SIGNAL>(baseSignal, output);
  auto effectType = static_cast<uint8_t>(libraryEffect.getEffectType());
  IOBinaryPrimitives::writeNBits<uint8_t, EFFECT_TYPE>(effectType, output);

  auto keyframeCount = static_cast<uint16_t>(libraryEffect.getKeyframesSize());
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(keyframeCount, output);
  types::Keyframe keyframe;
  for (unsigned short i = 0; i < keyframeCount; i++) {
    keyframe = libraryEffect.getKeyframeAt(i);
    auto mask = (uint8_t)KeyframeMask::NOTHING;
    if (keyframe.getRelativePosition().has_value()) {
      mask |= (uint8_t)KeyframeMask::RELATIVE_POSITION;
    }
    if (keyframe.getAmplitudeModulation().has_value()) {
      mask |= (uint8_t)KeyframeMask::AMPLITUDE_MODULATION;
    }
    if (keyframe.getFrequencyModulation().has_value()) {
      mask |= (uint8_t)KeyframeMask::FREQUENCY_MODULATION;
    }
    IOBinaryPrimitives::writeNBits<uint8_t, KEYFRAME_MASK>(mask, output);
    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      auto position = keyframe.getRelativePosition().value();
      IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_POSITION>(position, output);
    }

    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      auto amplitude = keyframe.getAmplitudeModulation().value();
      IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
          amplitude, output, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    }

    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      auto frequency = keyframe.getFrequencyModulation().value();
      IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_FREQUENCY>(frequency, output);
    }
  }

  auto timelineEffectCount = static_cast<uint16_t>(libraryEffect.getTimelineSize());
  IOBinaryPrimitives::writeNBits<unsigned short, EFFECT_TIMELINE_COUNT>(timelineEffectCount,
                                                                        output);
  // for each library effect
  types::Effect timelineEffect;
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    timelineEffect = libraryEffect.getTimelineEffectAt(i);
    writeLibraryEffect(libraryEffect, output);
  }
  return true;
}

auto IOBinary::readLibrary(types::Perception &perception, std::istream &file,
                           std::vector<bool> &unusedBits) -> bool {
  auto effectCount =
      IOBinaryPrimitives::readNBits<unsigned short, MDPERCE_LIBRARY_COUNT>(file, unusedBits);
  bool success = true;

  for (unsigned short i = 0; i < effectCount; i++) {
    auto effect = readLibraryEffect(file, unusedBits);
    perception.addBasisEffect(effect);
  }
  return success;
}

auto IOBinary::writeLibrary(types::Perception &perception, std::vector<bool> &output) -> bool {
  auto effectCount = static_cast<unsigned short>(perception.getEffectLibrarySize());
  IOBinaryPrimitives::writeNBits<unsigned short, MDPERCE_LIBRARY_COUNT>(effectCount, output);
  // for each library effect
  types::Effect libraryEffect;
  bool success = true;
  for (unsigned short i = 0; i < effectCount; i++) {
    libraryEffect = perception.getBasisEffectAt(i);
    success &= writeLibraryEffect(libraryEffect, output);
  }
  return success;
}

auto IOBinary::readReferenceDevices(types::Perception &perception, std::istream &file,
                                    std::vector<bool> &unusedBits) -> bool {
  auto referenceDeviceCount =
      IOBinaryPrimitives::readNBits<unsigned short, MDPERCE_REFDEVICE_COUNT>(file, unusedBits);

  for (unsigned short i = 0; i < referenceDeviceCount; i++) {
    auto referenceDeviceId = IOBinaryPrimitives::readNBits<short, REFDEV_ID>(file, unusedBits);
    std::string referenceDeviceName = IOBinaryPrimitives::readString(file, unusedBits);
    auto bodyPartMask =
        IOBinaryPrimitives::readNBits<uint32_t, REFDEV_BODY_PART_MASK>(file, unusedBits);

    types::ReferenceDevice myReferenceDevice(referenceDeviceId, referenceDeviceName);
    myReferenceDevice.setBodyPartMask(bodyPartMask);

    auto deviceInformationMask =
        IOBinaryPrimitives::readNBits<uint16_t, REFDEV_OPT_FIELDS>(file, unusedBits);

    float value = 0;
    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MAX_FREQ>(file, 0, MAX_FREQUENCY,
                                                                            unusedBits);
      myReferenceDevice.setMaximumFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MINIMUM_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MIN_FREQ>(file, 0, MAX_FREQUENCY,
                                                                            unusedBits);
      myReferenceDevice.setMinimumFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::RESONANCE_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_RES_FREQ>(file, 0, MAX_FREQUENCY,
                                                                            unusedBits);
      myReferenceDevice.setResonanceFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_AMPLITUDE) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MAX_AMP>(file, 0, MAX_FLOAT,
                                                                           unusedBits);
      myReferenceDevice.setMaximumAmplitude(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::IMPEDANCE) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_IMPEDANCE>(file, 0, MAX_FLOAT,
                                                                             unusedBits);
      myReferenceDevice.setImpedance(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_VOLTAGE) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MAX_VOLT>(file, 0, MAX_FLOAT,
                                                                            unusedBits);
      myReferenceDevice.setMaximumVoltage(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_CURRENT) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MAX_CURR>(file, 0, MAX_FLOAT,
                                                                            unusedBits);
      myReferenceDevice.setMaximumCurrent(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_DISPLACEMENT) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_MAX_DISP>(file, 0, MAX_FLOAT,
                                                                            unusedBits);
      myReferenceDevice.setMaximumDisplacement(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::WEIGHT) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_WEIGHT>(file, 0, MAX_FLOAT,
                                                                          unusedBits);
      myReferenceDevice.setWeight(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::SIZE) != 0) {
      value =
          IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_SIZE>(file, 0, MAX_FLOAT, unusedBits);
      myReferenceDevice.setSize(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::CUSTOM) != 0) {
      value = IOBinaryPrimitives::readFloatNBits<uint32_t, REFDEV_SIZE>(file, -MAX_FLOAT, MAX_FLOAT,
                                                                        unusedBits);
      myReferenceDevice.setCustom(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::TYPE) != 0) {
      auto type = IOBinaryPrimitives::readNBits<uint8_t, REFDEV_TYPE>(file, unusedBits);
      myReferenceDevice.setType(static_cast<types::ActuatorType>(type));
    }

    perception.addReferenceDevice(myReferenceDevice);
  }

  return true;
}

auto IOBinary::writeReferenceDevices(types::Perception &perception, std::vector<bool> &output)
    -> bool {
  auto referenceDeviceCount = static_cast<unsigned short>(perception.getReferenceDevicesSize());
  IOBinaryPrimitives::writeNBits<unsigned short, MDPERCE_REFDEVICE_COUNT>(referenceDeviceCount,
                                                                          output);

  // for each reference device
  types::ReferenceDevice myReferenceDevice;
  for (unsigned short i = 0; i < referenceDeviceCount; i++) {
    myReferenceDevice = perception.getReferenceDeviceAt(i);

    auto referenceDeviceId = static_cast<short>(myReferenceDevice.getId());
    IOBinaryPrimitives::writeNBits<short, REFDEV_ID>(referenceDeviceId, output);

    IOBinaryPrimitives::writeString(myReferenceDevice.getName(), output);

    uint32_t bodyPartMask = 0;
    if (myReferenceDevice.getBodyPartMask().has_value()) {
      bodyPartMask = myReferenceDevice.getBodyPartMask().value();
    }
    IOBinaryPrimitives::writeNBits<uint32_t, REFDEV_BODY_PART_MASK>(bodyPartMask, output);

    uint16_t deviceInformationMask =
        IOBinary::generateReferenceDeviceInformationMask(myReferenceDevice);
    IOBinaryPrimitives::writeNBits<uint16_t, REFDEV_OPT_FIELDS>(deviceInformationMask, output);

    float value = 0;
    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_FREQUENCY) != 0) {
      value = myReferenceDevice.getMaximumFrequency().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_FREQ>(value, output, 0,
                                                                     MAX_FREQUENCY);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MINIMUM_FREQUENCY) != 0) {
      value = myReferenceDevice.getMinimumFrequency().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MIN_FREQ>(value, output, 0,
                                                                     MAX_FREQUENCY);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::RESONANCE_FREQUENCY) != 0) {
      value = myReferenceDevice.getResonanceFrequency().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_RES_FREQ>(value, output, 0,
                                                                     MAX_FREQUENCY);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_AMPLITUDE) != 0) {
      value = myReferenceDevice.getMaximumAmplitude().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_AMP>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::IMPEDANCE) != 0) {
      value = myReferenceDevice.getImpedance().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_IMPEDANCE>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_VOLTAGE) != 0) {
      value = myReferenceDevice.getMaximumVoltage().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_VOLT>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_CURRENT) != 0) {
      value = myReferenceDevice.getMaximumCurrent().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_CURR>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_DISPLACEMENT) != 0) {
      value = myReferenceDevice.getMaximumDisplacement().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_DISP>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::WEIGHT) != 0) {
      value = myReferenceDevice.getWeight().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_WEIGHT>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::SIZE) != 0) {
      value = myReferenceDevice.getSize().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_SIZE>(value, output, 0, MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::CUSTOM) != 0) {
      value = myReferenceDevice.getCustom().value();
      IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_CUSTOM>(value, output, -MAX_FLOAT,
                                                                   MAX_FLOAT);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::TYPE) != 0) {
      uint8_t type = static_cast<uint8_t>(myReferenceDevice.getType().value());
      IOBinaryPrimitives::writeNBits<uint8_t, REFDEV_TYPE>(type, output);
    }
  }

  return true;
}

auto IOBinary::readTracksHeader(types::Perception &perception, std::istream &file,
                                std::vector<bool> &unusedBits) -> bool {
  auto trackCount =
      IOBinaryPrimitives::readNBits<unsigned short, MDPERCE_TRACK_COUNT>(file, unusedBits);
  // for each track
  for (unsigned short i = 0; i < trackCount; i++) {
    types::Track t;
    t.setId(IOBinaryPrimitives::readNBits<short, MDTRACK_ID>(file, unusedBits));
    std::string trackDescription = IOBinaryPrimitives::readString(file, unusedBits);
    t.setDescription(trackDescription);
    t.setReferenceDeviceId(
        IOBinaryPrimitives::readNBits<short, MDTRACK_DEVICE_ID>(file, unusedBits));
    t.setGain(IOBinaryPrimitives::readFloatNBits<uint32_t, MDTRACK_GAIN>(file, -MAX_FLOAT,
                                                                         MAX_FLOAT, unusedBits));
    t.setMixingWeight(IOBinaryPrimitives::readFloatNBits<uint32_t, MDTRACK_MIXING_WEIGHT>(
        file, 0, MAX_FLOAT, unusedBits));
    auto optionalMetadataMask =
        IOBinaryPrimitives::readNBits<uint8_t, MDTRACK_OPT_FIELDS>(file, unusedBits);
    if ((optionalMetadataMask & 0b0000'0001) != 0) {
      t.setBodyPartMask(
          IOBinaryPrimitives::readNBits<uint32_t, MDTRACK_BODY_PART_MASK>(file, unusedBits));
    } else if ((optionalMetadataMask & 0b0000'0010) != 0) {
      t.setActuatorResolution(IOBinaryPrimitives::readVector(file, unusedBits));

      auto bodyPartTargetCount =
          IOBinaryPrimitives::readNBits<uint8_t, MDTRACK_BODY_PART_TARGET_COUNT>(file, unusedBits);
      std::vector<types::BodyPartTarget> bodyPartTarget(bodyPartTargetCount,
                                                        types::BodyPartTarget::Unknown);
      for (auto &target : bodyPartTarget) {
        target = static_cast<types::BodyPartTarget>(
            IOBinaryPrimitives::readNBits<uint8_t, MDTRACK_BODY_PART_TARGET>(file, unusedBits));
      }
      t.setBodyPartTarget(bodyPartTarget);

      auto actuatorTargetCount =
          IOBinaryPrimitives::readNBits<uint8_t, MDTRACK_ACTUATOR_TARGET_COUNT>(file, unusedBits);
      std::vector<types::Vector> actuatorTarget(actuatorTargetCount);
      for (auto &target : actuatorTarget) {
        target = IOBinaryPrimitives::readVector(file, unusedBits);
      }
      t.setActuatorTarget(actuatorTarget);
    }

    auto frequencySampling =
        IOBinaryPrimitives::readNBits<uint32_t, MDTRACK_FREQ_SAMPLING>(file, unusedBits);
    if (frequencySampling != 0) {
      t.setFrequencySampling(frequencySampling);
      t.setSampleCount(
          IOBinaryPrimitives::readNBits<uint32_t, MDTRACK_SAMPLE_COUNT>(file, unusedBits));
    }
    if ((optionalMetadataMask & 0b0000'0100) != 0) {
      t.setDirection(IOBinaryPrimitives::readVector(file, unusedBits));
    }
    auto verticesCount = IOBinaryPrimitives::readNBits<int, MDTRACK_VERT_COUNT>(file, unusedBits);
    int vertex = 0;
    for (int j = 0; j < verticesCount; j++) {
      vertex = IOBinaryPrimitives::readNBits<int, MDTRACK_VERT>(file, unusedBits);
      t.addVertex(vertex);
    }

    auto bandCount =
        IOBinaryPrimitives::readNBits<unsigned short, MDTRACK_BANDS_COUNT>(file, unusedBits);
    for (unsigned short j = 0; j < bandCount; j++) {
      types::Band emptyBand;
      t.addBand(emptyBand);
    }

    perception.addTrack(t);
  }

  return true;
}

auto IOBinary::writeTracksHeader(types::Perception &perception, std::vector<bool> &output) -> bool {
  auto trackCount = static_cast<unsigned short>(perception.getTracksSize());
  IOBinaryPrimitives::writeNBits<unsigned short, MDPERCE_TRACK_COUNT>(trackCount, output);
  // for each track
  types::Track myTrack;
  for (unsigned short i = 0; i < trackCount; i++) {
    myTrack = perception.getTrackAt(i);

    auto trackId = static_cast<short>(myTrack.getId());
    IOBinaryPrimitives::writeNBits<short, MDTRACK_ID>(trackId, output);

    std::string trackDescription = myTrack.getDescription();
    IOBinaryPrimitives::writeString(trackDescription, output);

    short deviceId = static_cast<short>(myTrack.getReferenceDeviceId().value_or(-1));
    IOBinaryPrimitives::writeNBits<short, MDTRACK_DEVICE_ID>(deviceId, output);

    float trackGain = myTrack.getGain();
    IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_GAIN>(trackGain, output, -MAX_FLOAT,
                                                                MAX_FLOAT);

    float trackMixingWeight = myTrack.getMixingWeight();
    IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_MIXING_WEIGHT>(trackMixingWeight, output,
                                                                         0, MAX_FLOAT);

    auto optionalMetadataMask = (uint8_t)0b0000'0000;
    if (myTrack.getActuatorResolution().has_value()) {
      optionalMetadataMask |= (uint8_t)0b0000'0010;
    } else {
      optionalMetadataMask |= (uint8_t)0b0000'0001;
    }
    if (myTrack.getDirection().has_value()) {
      optionalMetadataMask |= (uint8_t)0b0000'0100;
    }
    IOBinaryPrimitives::writeNBits<uint8_t, MDTRACK_OPT_FIELDS>(optionalMetadataMask, output);

    if ((optionalMetadataMask & 0b0000'0010) != 0) {
      types::Vector trackResolution = myTrack.getActuatorResolution().value();
      IOBinaryPrimitives::writeVector(trackResolution, output);

      std::vector<types::BodyPartTarget> bodyPartTarget =
          myTrack.getBodyPartTarget().value_or(std::vector<types::BodyPartTarget>{});
      auto bodyPartTargetCount = static_cast<uint8_t>(bodyPartTarget.size());
      IOBinaryPrimitives::writeNBits<uint8_t, MDTRACK_BODY_PART_TARGET_COUNT>(bodyPartTargetCount,
                                                                              output);
      for (uint8_t i = 0; i < bodyPartTargetCount; i++) {
        IOBinaryPrimitives::writeNBits<uint8_t, MDTRACK_BODY_PART_TARGET>(
            static_cast<uint8_t>(bodyPartTarget[i]), output);
      }

      std::vector<types::Vector> actuatorTarget =
          myTrack.getActuatorTarget().value_or(std::vector<types::Vector>{});
      auto actuatorTargetCount = static_cast<uint8_t>(actuatorTarget.size());
      IOBinaryPrimitives::writeNBits<uint8_t, MDTRACK_ACTUATOR_TARGET_COUNT>(actuatorTargetCount,
                                                                             output);
      for (uint8_t i = 0; i < actuatorTargetCount; i++) {
        types::Vector target = actuatorTarget[i];
        IOBinaryPrimitives::writeVector(target, output);
      }
    } else if ((optionalMetadataMask & 0b0000'0001) != 0) {
      uint32_t bodyPartMask = myTrack.getBodyPartMask();
      IOBinaryPrimitives::writeNBits<uint32_t, MDTRACK_BODY_PART_MASK>(bodyPartMask, output);
    }

    uint32_t frequencySampling = myTrack.getFrequencySampling().value_or(0);
    IOBinaryPrimitives::writeNBits<uint32_t, MDTRACK_FREQ_SAMPLING>(frequencySampling, output);

    if (frequencySampling != 0) {
      uint32_t sampleCount = myTrack.getSampleCount().value_or(0);
      IOBinaryPrimitives::writeNBits<uint32_t, MDTRACK_SAMPLE_COUNT>(sampleCount, output);
    }

    if (myTrack.getDirection().has_value()) {
      types::Vector direction = myTrack.getDirection().value();
      IOBinaryPrimitives::writeVector(direction, output);
    }

    auto verticesCount = static_cast<int>(myTrack.getVerticesSize());
    IOBinaryPrimitives::writeNBits<int, MDTRACK_VERT_COUNT>(verticesCount, output);

    int vertex = 0;
    for (int j = 0; j < verticesCount; j++) {
      vertex = myTrack.getVertexAt(j);
      IOBinaryPrimitives::writeNBits<int, MDTRACK_VERT>(vertex, output);
    }

    auto bandCount = static_cast<unsigned short>(myTrack.getBandsSize());
    IOBinaryPrimitives::writeNBits<unsigned short, MDTRACK_BANDS_COUNT>(bandCount, output);
  }

  return true;
}

auto IOBinary::readFileBody(types::Haptics &haptic, std::istream &file,
                            std::vector<bool> &unusedBits) -> bool {
  types::Perception myPerception;
  types::Track myTrack;
  types::Band myBand;

  for (int perceptionIndex = 0; perceptionIndex < static_cast<int>(haptic.getPerceptionsSize());
       perceptionIndex++) {
    myPerception = haptic.getPerceptionAt(perceptionIndex);

    for (int trackIndex = 0; trackIndex < static_cast<int>(myPerception.getTracksSize());
         trackIndex++) {
      myTrack = myPerception.getTrackAt(trackIndex);

      for (int bandIndex = 0; bandIndex < static_cast<int>(myTrack.getBandsSize()); bandIndex++) {
        myBand = myTrack.getBandAt(bandIndex);
        if (!IOBinaryBands::readBandHeader(myBand, file, unusedBits)) {
          continue;
        }

        if (!IOBinaryBands::readBandBody(myBand, file, unusedBits)) {
          return false;
        }

        myTrack.replaceBandAt(bandIndex, myBand);
      }
      myPerception.replaceTrackAt(trackIndex, myTrack);
    }
    haptic.replacePerceptionAt(perceptionIndex, myPerception);
  }

  return true;
}

auto IOBinary::writeFileBody(types::Haptics &haptic, std::vector<bool> &output) -> bool {
  types::Perception myPerception;
  types::Track myTrack;
  types::Band myBand;
  for (unsigned short perceptionIndex = 0;
       perceptionIndex < static_cast<unsigned short>(haptic.getPerceptionsSize());
       perceptionIndex++) {
    myPerception = haptic.getPerceptionAt(perceptionIndex);

    for (unsigned short trackIndex = 0;
         trackIndex < static_cast<unsigned short>(myPerception.getTracksSize()); trackIndex++) {
      myTrack = myPerception.getTrackAt(trackIndex);

      for (unsigned short bandIndex = 0;
           bandIndex < static_cast<unsigned short>(myTrack.getBandsSize()); bandIndex++) {
        myBand = myTrack.getBandAt(bandIndex);
        if (!IOBinaryBands::writeBandHeader(myBand, output)) {
          continue;
        }

        if (!IOBinaryBands::writeBandBody(myBand, output)) {
          return false;
        }
      }
    }
  }

  return true;
}

auto IOBinary::generateReferenceDeviceInformationMask(types::ReferenceDevice &referenceDevice)
    -> uint16_t {
  auto mask = (uint16_t)DeviceInformationMask::NOTHING;

  if (referenceDevice.getMaximumFrequency().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MAXIMUM_FREQUENCY;
  }
  if (referenceDevice.getMinimumFrequency().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MINIMUM_FREQUENCY;
  }
  if (referenceDevice.getResonanceFrequency().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::RESONANCE_FREQUENCY;
  }
  if (referenceDevice.getMaximumAmplitude().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MAXIMUM_AMPLITUDE;
  }
  if (referenceDevice.getImpedance().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::IMPEDANCE;
  }
  if (referenceDevice.getMaximumVoltage().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MAXIMUM_VOLTAGE;
  }
  if (referenceDevice.getMaximumCurrent().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MAXIMUM_CURRENT;
  }
  if (referenceDevice.getMaximumDisplacement().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::MAXIMUM_DISPLACEMENT;
  }
  if (referenceDevice.getWeight().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::WEIGHT;
  }
  if (referenceDevice.getSize().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::SIZE;
  }
  if (referenceDevice.getCustom().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::CUSTOM;
  }
  if (referenceDevice.getType().has_value()) {
    mask |= (uint16_t)DeviceInformationMask::TYPE;
  }

  return mask;
}
} // namespace haptics::io
