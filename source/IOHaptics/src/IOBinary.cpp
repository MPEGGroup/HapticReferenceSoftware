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
#include <array>
#include <fstream>
#include <iostream>

namespace haptics::io {

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

  bool res = IOBinary::readFileHeader(out, file);
  if (res) {
    IOBinary::readFileBody(out, file);
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

  bool res = IOBinary::writeFileHeader(haptic, file);
  if (res) {
    res = IOBinary::writeFileBody(haptic, file);
  }

  file.close();
  return res;
}

auto IOBinary::readFileHeader(types::Haptics &haptic, std::ifstream &file) -> bool {
  std::string version = IOBinaryPrimitives::readString(file);
  std::string date = IOBinaryPrimitives::readString(file);
  std::string description = IOBinaryPrimitives::readString(file);
  haptic.setVersion(version);
  haptic.setDate(date);
  haptic.setDescription(description);

  // Get avatars
  if (!IOBinary::readAvatars(haptic, file)) {
    return false;
  }

  // Get perceptions
  return IOBinary::readPerceptionsHeader(haptic, file);
}

auto IOBinary::writeFileHeader(types::Haptics &haptic, std::ofstream &file) -> bool {
  const std::string version = haptic.getVersion();
  const std::string date = haptic.getDate();
  const std::string description = haptic.getDescription();

  IOBinaryPrimitives::writeString(version, file);
  IOBinaryPrimitives::writeString(date, file);
  IOBinaryPrimitives::writeString(description, file);

  if (!IOBinary::writeAvatars(haptic, file)) {
    return false;
  }

  return IOBinary::writePerceptionsHeader(haptic, file);
}

auto IOBinary::readAvatars(types::Haptics &haptic, std::ifstream &file) -> bool {
  auto avatarCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

  types::Avatar myAvatar;
  for (unsigned short i = 0; i < avatarCount; i++) {
    auto avatarId = IOBinaryPrimitives::readNBytes<short, 2>(file);
    auto avatarLod = IOBinaryPrimitives::readNBytes<int, 4>(file);
    auto avatarType = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

    std::string avatarURI;
    myAvatar = types::Avatar(avatarId, avatarLod, static_cast<types::AvatarType>(avatarType));
    if (myAvatar.getType() == types::AvatarType::Custom) {
      avatarURI = IOBinaryPrimitives::readString(file);
      myAvatar.setMesh(avatarURI);
    }

    haptic.addAvatar(myAvatar);
  }

  return true;
}

auto IOBinary::writeAvatars(types::Haptics &haptic, std::ofstream &file) -> bool {
  auto avatarCount = static_cast<unsigned short>(haptic.getAvatarsSize());
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(avatarCount, file);

  types::Avatar myAvatar;
  for (unsigned short i = 0; i < avatarCount; i++) {
    myAvatar = haptic.getAvatarAt(i);

    auto avatarId = static_cast<short>(myAvatar.getId());
    IOBinaryPrimitives::writeNBytes<short, 2>(avatarId, file);

    int avatarLod = myAvatar.getLod();
    IOBinaryPrimitives::writeNBytes<int, 4>(avatarLod, file);

    auto avatarType = static_cast<unsigned short>(myAvatar.getType());
    IOBinaryPrimitives::writeNBytes<unsigned short, 2>(avatarType, file);

    if (myAvatar.getType() == types::AvatarType::Custom) {
      const std::string avatarURI = myAvatar.getMesh().value_or("");
      IOBinaryPrimitives::writeString(avatarURI, file);
    }
  }
  return true;
}

auto IOBinary::readPerceptionsHeader(types::Haptics &haptic, std::ifstream &file) -> bool {
  auto perceptionCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

  types::Perception myPerception;
  for (unsigned short i = 0; i < perceptionCount; i++) {
    auto perceptionId = IOBinaryPrimitives::readNBytes<short, 2>(file);
    auto perceptionModality = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);
    std::string perceptionDescription = IOBinaryPrimitives::readString(file);
    auto avatarId = IOBinaryPrimitives::readNBytes<int, 4>(file);

    myPerception = types::Perception(perceptionId, avatarId, perceptionDescription,
                                     static_cast<types::PerceptionModality>(perceptionModality));
    if (!IOBinary::readReferenceDevices(myPerception, file)) {
      return false;
    }
    if (!IOBinary::readTracksHeader(myPerception, file)) {
      return false;
    }
    haptic.addPerception(myPerception);
  }

  return true;
}

auto IOBinary::writePerceptionsHeader(types::Haptics &haptic, std::ofstream &file) -> bool {
  auto perceptionCount = static_cast<unsigned short>(haptic.getPerceptionsSize());
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(perceptionCount, file);

  types::Perception myPerception;
  for (unsigned short i = 0; i < perceptionCount; i++) {
    myPerception = haptic.getPerceptionAt(i);

    auto perceptionId = static_cast<short>(myPerception.getId());
    IOBinaryPrimitives::writeNBytes<short, 2>(perceptionId, file);

    auto perceptionModality = static_cast<unsigned short>(myPerception.getPerceptionModality());
    IOBinaryPrimitives::writeNBytes<unsigned short, 2>(perceptionModality, file);

    std::string perceptionDescription = myPerception.getDescription();
    IOBinaryPrimitives::writeString(perceptionDescription, file);

    int avatarId = myPerception.getAvatarId();
    IOBinaryPrimitives::writeNBytes<int, 4>(avatarId, file);

    if (!IOBinary::writeReferenceDevices(myPerception, file)) {
      return false;
    }

    if (!IOBinary::writeTracksHeader(myPerception, file)) {
      return false;
    }
  }
  return true;
}

auto IOBinary::readReferenceDevices(types::Perception &perception, std::ifstream &file) -> bool {
  auto referenceDeviceCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

  for (unsigned short i = 0; i < referenceDeviceCount; i++) {
    auto referenceDeviceId = IOBinaryPrimitives::readNBytes<short, 2>(file);
    std::string referenceDeviceName = IOBinaryPrimitives::readString(file);
    auto bodyPartMask = IOBinaryPrimitives::readNBytes<uint32_t, 4>(file);

    types::ReferenceDevice myReferenceDevice(referenceDeviceId, referenceDeviceName);
    myReferenceDevice.setBodyPartMask(bodyPartMask);

    auto deviceInformationMask = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);

    float value = 0;
    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMaximumFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MINIMUM_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMinimumFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::RESONANCE_FREQUENCY) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setResonanceFrequency(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_AMPLITUDE) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMaximumAmplitude(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::IMPEDANCE) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setImpedance(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_VOLTAGE) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMaximumVoltage(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_CURRENT) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMaximumCurrent(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_DISPLACEMENT) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setMaximumDisplacement(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::WEIGHT) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setWeight(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::SIZE) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setSize(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::CUSTOM) != 0) {
      value = IOBinaryPrimitives::readFloat(file);
      myReferenceDevice.setCustom(value);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::TYPE) != 0) {
      auto type = IOBinaryPrimitives::readNBytes<uint8_t, 1>(file);
      myReferenceDevice.setType(static_cast<types::ActuatorType>(type));
    }

    perception.addReferenceDevice(myReferenceDevice);
  }

  return true;
}

auto IOBinary::writeReferenceDevices(types::Perception &perception, std::ofstream &file) -> bool {
  auto referenceDeviceCount = static_cast<unsigned short>(perception.getReferenceDevicesSize());
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(referenceDeviceCount, file);

  // for each reference device
  types::ReferenceDevice myReferenceDevice;
  for (unsigned short i = 0; i < referenceDeviceCount; i++) {
    myReferenceDevice = perception.getReferenceDeviceAt(i);

    auto referenceDeviceId = static_cast<short>(myReferenceDevice.getId());
    IOBinaryPrimitives::writeNBytes<short, 2>(referenceDeviceId, file);

    IOBinaryPrimitives::writeString(myReferenceDevice.getName(), file);

    uint32_t bodyPartMask = 0;
    if (myReferenceDevice.getBodyPartMask().has_value()) {
      bodyPartMask = myReferenceDevice.getBodyPartMask().value();
    }
    IOBinaryPrimitives::writeNBytes<uint32_t, 4>(bodyPartMask, file);

    uint16_t deviceInformationMask =
        IOBinary::generateReferenceDeviceInformationMask(myReferenceDevice);
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(deviceInformationMask, file);

    float value = 0;
    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_FREQUENCY) != 0) {
      value = myReferenceDevice.getMaximumFrequency().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MINIMUM_FREQUENCY) != 0) {
      value = myReferenceDevice.getMinimumFrequency().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::RESONANCE_FREQUENCY) != 0) {
      value = myReferenceDevice.getResonanceFrequency().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_AMPLITUDE) != 0) {
      value = myReferenceDevice.getMaximumAmplitude().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::IMPEDANCE) != 0) {
      value = myReferenceDevice.getImpedance().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_VOLTAGE) != 0) {
      value = myReferenceDevice.getMaximumVoltage().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_CURRENT) != 0) {
      value = myReferenceDevice.getMaximumCurrent().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::MAXIMUM_DISPLACEMENT) != 0) {
      value = myReferenceDevice.getMaximumDisplacement().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::WEIGHT) != 0) {
      value = myReferenceDevice.getWeight().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::SIZE) != 0) {
      value = myReferenceDevice.getSize().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::CUSTOM) != 0) {
      value = myReferenceDevice.getCustom().value();
      IOBinaryPrimitives::writeFloat(value, file);
    }

    if ((deviceInformationMask & (uint16_t)DeviceInformationMask::TYPE) != 0) {
      uint8_t type = static_cast<uint8_t>(myReferenceDevice.getType().value());
      IOBinaryPrimitives::writeNBytes<uint8_t, 1>(type, file);
    }
  }

  return true;
}

auto IOBinary::readTracksHeader(types::Perception &perception, std::ifstream &file) -> bool {
  auto trackCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

  // for each track
  for (unsigned short i = 0; i < trackCount; i++) {
    auto trackId = IOBinaryPrimitives::readNBytes<short, 2>(file);
    std::string trackDescription = IOBinaryPrimitives::readString(file);
    auto deviceId = IOBinaryPrimitives::readNBytes<short, 2>(file);
    auto trackGain = IOBinaryPrimitives::readNBytes<float, 4>(file);
    auto trackMixingWeight = IOBinaryPrimitives::readNBytes<float, 4>(file);
    auto bodyPartMask = IOBinaryPrimitives::readNBytes<uint32_t, 4>(file);
    auto verticesCount = IOBinaryPrimitives::readNBytes<int, 4>(file);

    types::Track t(trackId, trackDescription, trackGain, trackMixingWeight, bodyPartMask,
                   std::nullopt, std::nullopt);
    if (deviceId >= 0) {
      t.setReferenceDeviceId(deviceId);
    }
    int vertex = 0;
    for (int j = 0; j < verticesCount; j++) {
      vertex = IOBinaryPrimitives::readNBytes<int, 4>(file);
      t.addVertex(vertex);
    }

    auto bandCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);
    for (unsigned short j = 0; j < bandCount; j++) {
      types::Band emptyBand;
      t.addBand(emptyBand);
    }

    perception.addTrack(t);
  }

  return true;
}

auto IOBinary::writeTracksHeader(types::Perception &perception, std::ofstream &file) -> bool {
  auto trackCount = static_cast<unsigned short>(perception.getTracksSize());
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(trackCount, file);

  // for each track
  types::Track myTrack;
  for (unsigned short i = 0; i < trackCount; i++) {
    myTrack = perception.getTrackAt(i);

    auto trackId = static_cast<short>(myTrack.getId());
    IOBinaryPrimitives::writeNBytes<short, 2>(trackId, file);

    std::string trackDescription = myTrack.getDescription();
    IOBinaryPrimitives::writeString(trackDescription, file);

    short deviceId = static_cast<short>(myTrack.getReferenceDeviceId().value_or(-1));
    IOBinaryPrimitives::writeNBytes<short, 2>(deviceId, file);

    float trackGain = myTrack.getGain();
    IOBinaryPrimitives::writeNBytes<float, 4>(trackGain, file);

    float trackMixingWeight = myTrack.getMixingWeight();
    IOBinaryPrimitives::writeNBytes<float, 4>(trackMixingWeight, file);

    uint32_t bodyPartMask = myTrack.getBodyPartMask();
    IOBinaryPrimitives::writeNBytes<uint32_t, 4>(bodyPartMask, file);

    auto verticesCount = static_cast<int>(myTrack.getVerticesSize());
    IOBinaryPrimitives::writeNBytes<int, 4>(verticesCount, file);

    int vertex = 0;
    for (int j = 0; j < verticesCount; j++) {
      vertex = myTrack.getVertexAt(j);
      IOBinaryPrimitives::writeNBytes<int, 4>(vertex, file);
    }

    auto bandCount = static_cast<unsigned short>(myTrack.getBandsSize());
    IOBinaryPrimitives::writeNBytes<unsigned short, 2>(bandCount, file);
  }

  return true;
}

auto IOBinary::readFileBody(types::Haptics &haptic, std::ifstream &file) -> bool {
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
        if (!IOBinaryBands::readBandHeader(myBand, file)) {
          continue;
        }

        if (!IOBinaryBands::readBandBody(myBand, file)) {
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

auto IOBinary::writeFileBody(types::Haptics &haptic, std::ofstream &file) -> bool {
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
        if (!IOBinaryBands::writeBandHeader(myBand, file)) {
          continue;
        }

        if (!IOBinaryBands::writeBandBody(myBand, file)) {
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
