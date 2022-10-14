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

#include <IOHaptics/include/IOBinaryBands.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <IOHaptics/include/IOStream.h>

namespace haptics::io {

auto IOStream::writeFile(types::Haptics &haptic, const std::string &filePath) -> bool {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    return false;
  }

  std::vector<std::vector<bool>> packetsBytes = std::vector<std::vector<bool>>();
  bool success = IOStream::writePacket(haptic, packetsBytes);
  std::vector<bool> binary = std::vector<bool>();
  if (success) {
    for (auto &packet : packetsBytes) {
      binary.insert(binary.end(), packet.begin(), packet.end());
    }
    IOBinaryPrimitives::writeBitset(binary, file);
  }

  file.close();
  return success;
}
auto IOStream::readFile(const std::string &filePath, types::Haptics &haptic) -> bool {
  std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
  loadFile(filePath, bitstream);
  Buffer buffer = initializeStream(haptic);
  for (auto &packet : bitstream) {
    if (!readNALu(haptic, packet, buffer)) {
      return EXIT_FAILURE;
    }
  }
  return true;
}

auto IOStream::loadFile(const std::string &filePath, std::vector<std::vector<bool>> &bitset)
    -> bool {
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

  std::vector<std::vector<bool>> packetBits = std::vector<std::vector<bool>>();
  unsigned int byteCount = 0;
  while (byteCount < length) {
    std::vector<bool> bufPacket = std::vector<bool>();
    // read packet header
    IOBinaryPrimitives::readNBytes(file, H_BYTES_LENGTH, bufPacket);
    byteCount += H_BYTES_LENGTH;
    // read packet payload length
    int bytesToRead = readPacketLength(bufPacket);
    // read paylaod
    IOBinaryPrimitives::readNBytes(file, bytesToRead, bufPacket);
    byteCount += bytesToRead;
    bitset.push_back(bufPacket);
  }
  file.close();
  return true;
}

auto IOStream::writePacket(types::Haptics &haptic, std::ofstream &file) -> bool {
  std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
  writeNALu(NALuType::MetadataHaptics, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataPerception, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataTrack, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataBand, haptic, 0, bitstream);
  writeNALu(NALuType::Data, haptic, 0, bitstream);

  std::string strBitstream;
  for (auto &packet : bitstream) {
    for (auto c : packet) {
      if (c) {
        strBitstream += "1";
      } else {
        strBitstream += "0";
      }
    }
    strBitstream += "/n";
  }
  file.write(strBitstream.c_str(), static_cast<int>(strBitstream.size()));
  return true;
}
auto IOStream::writePacket(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream)
    -> bool {
  writeNALu(NALuType::MetadataHaptics, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataPerception, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataTrack, haptic, 0, bitstream);
  writeNALu(NALuType::MetadataBand, haptic, 0, bitstream);
  writeNALu(NALuType::Data, haptic, 0, bitstream);

  return true;
}

auto IOStream::initializeStream(types::Haptics &haptic) -> Buffer {
  haptic = types::Haptics();
  Buffer buffer;
  buffer.perceptionsBuffer = std::vector<types::Perception>();
  buffer.tracksBuffer = std::vector<types::Track>();
  buffer.bandStreamsBuffer = std::vector<BandStream>();
  buffer.bandStreamsHaptic = std::vector<BandStream>();

  return buffer;
}

auto IOStream::writeNALu(NALuType naluType, types::Haptics &haptic, int level,
                         std::vector<std::vector<bool>> &bitstream) -> bool {

  checkHapticComponent(haptic);
  std::vector<bool> naluHeader = std::vector<bool>();
  switch (naluType) {
  case NALuType::MetadataHaptics: {
    std::vector<bool> naluPayload = std::vector<bool>();
    writeMetadataHaptics(haptic, naluPayload);
    padToByteBoundary(naluPayload);
    writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.push_back(naluHeader);
    return true;
  }
  case NALuType::MetadataPerception: {
    std::vector<bool> naluPayload = std::vector<bool>();
    for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
      writeMetadataPerception(haptic.getPerceptionAt(i), naluPayload);
      padToByteBoundary(naluPayload);
      writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
      naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
      bitstream.push_back(naluHeader);
      naluPayload.clear();
      naluHeader.clear();
    }

    return true;
  }
  case NALuType::EffectLibrary: {
    std::vector<bool> naluPayload = std::vector<bool>();
    for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
      writeLibrary(haptic.getPerceptionAt(i), naluPayload);
      padToByteBoundary(naluPayload);
      writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
      naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
      bitstream.push_back(naluHeader);
      naluPayload.clear();
      naluHeader.clear();
    }

    return true;
  }
  case NALuType::MetadataTrack: {
    std::vector<bool> naluPayload = std::vector<bool>();
    for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
      for (int j = 0; j < haptic.getPerceptionAt(i).getTracksSize(); j++) {
        writeMetadataTrack(haptic.getPerceptionAt(i).getTrackAt(j), naluPayload);
        padToByteBoundary(naluPayload);
        writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
        naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
        bitstream.push_back(naluHeader);
        naluPayload.clear();
        naluHeader.clear();
      }
    }
    return true;
  }
  case NALuType::MetadataBand: {
    return writeAllBands(haptic, naluType, level, naluHeader, bitstream);
  }
  case NALuType::Data: {
    std::vector<std::vector<bool>> naluPayload = std::vector<std::vector<bool>>();
    writeData(haptic, naluPayload);
    for (auto data : naluPayload) {
      padToByteBoundary(data);
      writeNALuHeader(naluType, level, static_cast<int>(data.size()), naluHeader);
      naluHeader.insert(naluHeader.end(), data.begin(), data.end());
      bitstream.push_back(naluHeader);
      naluHeader.clear();
    }
    return true;
  }
  case NALuType::CRC: {
    std::vector<bool> naluPayload = std::vector<bool>();
    // writeCRC(bitstream);
    padToByteBoundary(naluPayload);
    writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.push_back(naluHeader);
    return true;
  }
  case NALuType::ByteStuffing: {
    std::vector<bool> naluPayload = std::vector<bool>();
    padToByteBoundary(naluPayload);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.push_back(naluHeader);
    return true;
  }
  default:
    return false;
  }

  return false;
}

auto IOStream::writeAllBands(types::Haptics &haptic, NALuType naluType, int level,
                             std::vector<bool> &naluHeader,
                             std::vector<std::vector<bool>> &bitstream) -> bool {
  std::vector<bool> naluPayload = std::vector<bool>();
  int bandId = 0;
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (int j = 0; j < haptic.getPerceptionAt(i).getTracksSize(); j++) {
      for (int k = 0; k < haptic.getPerceptionAt(i).getTrackAt(j).getBandsSize(); k++) {
        writeMetadataBand(haptic.getPerceptionAt(i).getTrackAt(j).getBandAt(k), naluPayload,
                          bandId++);
        padToByteBoundary(naluPayload);
        writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
        naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
        bitstream.push_back(naluHeader);
        naluPayload.clear();
        naluHeader.clear();
      }
    }
  }
  return true;
}

auto IOStream::checkHapticComponent(types::Haptics &haptic) -> void {
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    types::Perception &perception = haptic.getPerceptionAt(i);
    for (int j = 0; j < perception.getTracksSize(); j++) {
      types::Track &track = perception.getTrackAt(j);
      std::vector<types::Band> bands = std::vector<types::Band>();
      for (int k = 0; k < track.getBandsSize(); k++) {
        types::Band &band = track.getBandAt(k);
        for (int l = 0; l < band.getEffectsSize(); l++) {
          types::Effect &effect = band.getEffectAt(l);
          if (band.getBandType() != types::BandType::WaveletWave &&
              effect.getEffectType() == types::EffectType::Basis &&
              effect.getKeyframesSize() == 0) {
            band.removeEffectAt(l);
          }
        }
        if (band.getEffectsSize() == 0) {
          track.removeBandAt(k);
        }
      }
      if (track.getBandsSize() == 0) {
        perception.removeTrackAt(j);
      }
    }
    if (perception.getTracksSize() == 0) {
      haptic.removePerceptionAt(i);
    }
  }
}

auto IOStream::writeNALuHeader(NALuType naluType, int level, int payloadSize,
                               std::vector<bool> &bitstream) -> bool {
  std::bitset<H_NALU_TYPE> naluTypeBits(static_cast<int>(naluType));
  IOBinaryPrimitives::writeStrBits(naluTypeBits.to_string(), bitstream);

  std::bitset<H_LEVEL> lvlBits(level);
  IOBinaryPrimitives::writeStrBits(lvlBits.to_string(), bitstream);
  const int residual = H_NBITS - (H_NALU_TYPE + H_LEVEL + H_PAYLOAD_LENGTH);
  std::bitset<residual> resBits(0);
  IOBinaryPrimitives::writeStrBits(resBits.to_string(), bitstream);
  int payloadSizeByte = payloadSize / BYTE_SIZE;
  std::bitset<H_PAYLOAD_LENGTH> payloadSizeBits(payloadSizeByte);
  IOBinaryPrimitives::writeStrBits(payloadSizeBits.to_string(), bitstream);

  return true;
}
auto IOStream::readNALu(types::Haptics &haptic, std::vector<bool> packet, Buffer &buffer) -> bool {

  NALuType naluType = readNALuType(packet);
  int index = H_NALU_TYPE;
  int level = IOBinaryPrimitives::readInt(packet, index, H_LEVEL);
  index = H_NBITS - H_PAYLOAD_LENGTH;
  int packetLength = IOBinaryPrimitives::readInt(packet, index, H_PAYLOAD_LENGTH) * BYTE_SIZE;
  std::vector<bool> payload = std::vector<bool>(packet.begin() + index, packet.end());
  switch (naluType) {
  case (NALuType::MetadataHaptics): {
    return readMetadataHaptics(haptic, payload);
  }
  case (NALuType::MetadataPerception): {
    types::Perception perception = types::Perception();
    if (!readMetadataPerception(perception, payload)) {
      return false;
    }
    buffer.perceptionsBuffer.push_back(perception);
    return true;
  }
  case (NALuType::EffectLibrary): {
    types::Perception perception = types::Perception();
    if (!readMetadataPerception(perception, payload)) {
      return false;
    }
    buffer.perceptionsBuffer.push_back(perception);
    return true;
  }
  case (NALuType::MetadataTrack): {
    types::Track track = types::Track();
    if (!readMetadataTrack(track, payload)) {
      return false;
    }
    buffer.tracksBuffer.push_back(track);
    return true;
  }
  case (NALuType::MetadataBand): {
    BandStream bandStream;
    if (!readMetadataBand(bandStream, payload)) {
      return false;
    }
    buffer.bandStreamsBuffer.push_back(bandStream);
    return true;
  }
  case (NALuType::Data): {
    return readData(haptic, buffer, payload);
  }
  }
  return false;
}

auto IOStream::readPacketTS(std::vector<bool> bitstream) -> int {
  std::string tsBits;
  for (int i = DB_AU_TYPE; i < DB_AU_TYPE + DB_TIMESTAMP; i++) {
    if ((bitstream[i])) {
      tsBits += "1";
    } else {
      tsBits += "0";
    }
  }
  return std::stoi(tsBits, nullptr, 2);
}
auto IOStream::readNALuType(std::vector<bool> &packet) -> NALuType {
  int idx = 0;
  int typeInt = IOBinaryPrimitives::readInt(packet, idx, H_NALU_TYPE);

  return static_cast<NALuType>(typeInt);
}
auto IOStream::readPacketLength(std::vector<bool> &bitstream) -> int {
  int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
  std::vector<bool> packetLength = std::vector<bool>(
      bitstream.begin() + beginIdx, bitstream.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
  std::string packetLengthStr = "";
  for (auto c : packetLength) {
    if (c) {
      packetLengthStr += "1";
    } else {
      packetLengthStr += "0";
    }
  }
  return static_cast<int>(std::bitset<H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
}

auto IOStream::writeMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDEXP_VERSION> versionCountBits(haptic.getVersion().size());
  IOBinaryPrimitives::writeStrBits(versionCountBits.to_string(), bitstream);
  for (auto c : haptic.getVersion()) {
    std::bitset<BYTE_SIZE> cBits(c);
    IOBinaryPrimitives::writeStrBits(cBits.to_string(), bitstream);
  }

  std::bitset<MDEXP_DATE> dateCountBits(haptic.getDate().size());
  IOBinaryPrimitives::writeStrBits(dateCountBits.to_string(), bitstream);
  for (auto c : haptic.getDate()) {
    std::bitset<BYTE_SIZE> cBits(c);
    IOBinaryPrimitives::writeStrBits(cBits.to_string(), bitstream);
  }
  std::bitset<MDEXP_DESC_SIZE> descSizeBits(haptic.getDescription().length());
  IOBinaryPrimitives::writeStrBits(descSizeBits.to_string(), bitstream);

  for (char &c : haptic.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    IOBinaryPrimitives::writeStrBits(descBits.to_string(), bitstream);
  }

  std::bitset<MDEXP_PERC_COUNT> perceCountBits(haptic.getPerceptionsSize());
  IOBinaryPrimitives::writeStrBits(perceCountBits.to_string(), bitstream);

  std::bitset<MDEXP_AVATAR_COUNT> avatarCountBits(haptic.getAvatarsSize());
  IOBinaryPrimitives::writeStrBits(avatarCountBits.to_string(), bitstream);

  for (int i = 0; i < haptic.getAvatarsSize(); i++) {
    writeAvatar(haptic.getAvatarAt(i), bitstream);
  }
  return true;
}
auto IOStream::readMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool {
  int index = 0;

  int versionLength = IOBinaryPrimitives::readInt(bitstream, index, MDEXP_VERSION);
  std::string version = IOBinaryPrimitives::readString(bitstream, index, versionLength);
  haptic.setVersion(version);

  int dateLength = IOBinaryPrimitives::readInt(bitstream, index, MDEXP_DATE);
  std::string date = IOBinaryPrimitives::readString(bitstream, index, dateLength);
  haptic.setDate(date);

  int descLength = IOBinaryPrimitives::readInt(bitstream, index, MDEXP_DESC_SIZE);
  std::string description = IOBinaryPrimitives::readString(bitstream, index, descLength);
  haptic.setDescription(description);

  int perceCount = IOBinaryPrimitives::readInt(bitstream, index, MDEXP_PERC_COUNT);

  int avatarCount = IOBinaryPrimitives::readInt(bitstream, index, MDEXP_AVATAR_COUNT);
  std::vector<types::Avatar> avatarList = std::vector<types::Avatar>();
  std::vector<bool> avaratListBits(bitstream.begin() + index, bitstream.end());
  if (!readListObject(avaratListBits, avatarCount, avatarList)) {
    return false;
  }
  for (auto avatar : avatarList) {
    haptic.addAvatar(avatar);
  }
  return true;
}

auto IOStream::writeAvatar(types::Avatar &avatar, std::vector<bool> &bitstream) -> bool {
  std::bitset<AVATAR_ID> avatarIDBits(avatar.getId());
  IOBinaryPrimitives::writeStrBits(avatarIDBits.to_string(), bitstream);

  std::bitset<AVATAR_LOD> avatarLODBits(avatar.getLod());
  IOBinaryPrimitives::writeStrBits(avatarLODBits.to_string(), bitstream);

  std::bitset<AVATAR_TYPE> avatarTypeBits(static_cast<int>(avatar.getType()));
  IOBinaryPrimitives::writeStrBits(avatarTypeBits.to_string(), bitstream);

  if (static_cast<int>(avatar.getType()) == 0) {
    std::string mesh = avatar.getMesh().value();
    std::bitset<AVATAR_MESH_COUNT> avatarMeshCountBits(mesh.size());
    IOBinaryPrimitives::writeStrBits(avatarMeshCountBits.to_string(), bitstream);

    for (char &c : mesh) {
      std::bitset<BYTE_SIZE> descBits(c);
      IOBinaryPrimitives::writeStrBits(descBits.to_string(), bitstream);
    }
  }
  return true;
}
auto IOStream::readAvatar(std::vector<bool> &bitstream, types::Avatar &avatar, int &length)
    -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readInt(bitstream, idx, AVATAR_ID);
  int lod = IOBinaryPrimitives::readInt(bitstream, idx, AVATAR_LOD);
  int type = IOBinaryPrimitives::readInt(bitstream, idx, AVATAR_TYPE);
  avatar.setId(id);
  avatar.setLod(lod);
  avatar.setType(static_cast<types::AvatarType>(type));
  if (type == 0) {
    int meshCount = IOBinaryPrimitives::readInt(bitstream, idx, AVATAR_MESH_COUNT);
    std::string mesh = IOBinaryPrimitives::readString(bitstream, idx, meshCount);
    avatar.setMesh(mesh);
  }
  length += idx;
  return true;
}

auto IOStream::writeMetadataPerception(types::Perception &perception, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<MDPERCE_ID> perceIDBits(perception.getId());
  IOBinaryPrimitives::writeStrBits(perceIDBits.to_string(), bitstream);

  std::bitset<MDPERCE_DESC_SIZE> descSizeBits(perception.getDescription().size());
  IOBinaryPrimitives::writeStrBits(descSizeBits.to_string(), bitstream);

  for (char &c : perception.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    IOBinaryPrimitives::writeStrBits(descBits.to_string(), bitstream);
  }

  std::bitset<MDPERCE_MODALITY> modalityBits(static_cast<int>(perception.getPerceptionModality()));
  IOBinaryPrimitives::writeStrBits(modalityBits.to_string(), bitstream);

  std::bitset<AVATAR_ID> avatarIDBits(perception.getAvatarId());
  IOBinaryPrimitives::writeStrBits(avatarIDBits.to_string(), bitstream);

  std::bitset<MDPERCE_FXLIB_COUNT> fxLibCountBits(perception.getEffectLibrarySize());
  IOBinaryPrimitives::writeStrBits(fxLibCountBits.to_string(), bitstream);

  std::bitset<MDPERCE_UNIT_EXP> unitExpBits(perception.getUnitExponentOrDefault());
  IOBinaryPrimitives::writeStrBits(unitExpBits.to_string(), bitstream);

  std::bitset<MDPERCE_PERCE_UNIT_EXP> perceUnitExpBits(
      perception.getPerceptionUnitExponentOrDefault());
  IOBinaryPrimitives::writeStrBits(perceUnitExpBits.to_string(), bitstream);

  std::bitset<MDPERCE_REFDEVICE_COUNT> refDeviceCountBits(perception.getReferenceDevicesSize());
  IOBinaryPrimitives::writeStrBits(refDeviceCountBits.to_string(), bitstream);

  for (int i = 0; i < perception.getReferenceDevicesSize(); i++) {
    writeReferenceDevice(perception.getReferenceDeviceAt(i), bitstream);
  }

  std::bitset<MDPERCE_TRACK_COUNT> trackCountBits(perception.getTracksSize());
  IOBinaryPrimitives::writeStrBits(trackCountBits.to_string(), bitstream);
  return true;
}
auto IOStream::readMetadataPerception(types::Perception &perception, std::vector<bool> &bitstream)
    -> bool {
  int idx = 0;

  int id = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_ID);
  perception.setId(id);

  int descLength = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_DESC_SIZE);

  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  perception.setDescription(desc);

  int modal = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_MODALITY);
  perception.setPerceptionModality(static_cast<types::PerceptionModality>(modal));

  int avatarId = IOBinaryPrimitives::readInt(bitstream, idx, AVATAR_ID);
  perception.setAvatarId(avatarId);

  int fxCount = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_FXLIB_COUNT);

  int unitExp = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_UNIT_EXP);
  perception.setUnitExponent(unitExp);

  int perceUnitExp = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_PERCE_UNIT_EXP);
  perception.setPerceptionUnitExponent(perceUnitExp);

  int refDevCount = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_REFDEVICE_COUNT);
  std::vector<types::ReferenceDevice> referenceDeviceList = std::vector<types::ReferenceDevice>();
  std::vector<bool> refDeviceListBits(bitstream.begin() + idx,
                                      bitstream.end() - MDPERCE_TRACK_COUNT);
  if (!readListObject(refDeviceListBits, refDevCount, referenceDeviceList)) {
    return false;
  }
  for (auto refDev : referenceDeviceList) {
    perception.addReferenceDevice(refDev);
  }
  idx = static_cast<int>(bitstream.size()) - MDPERCE_TRACK_COUNT;
  int trackCount = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_TRACK_COUNT);

  return true;
}

auto IOStream::writeLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDPERCE_FXLIB_COUNT> fxCountBits(perception.getEffectLibrarySize());
  IOBinaryPrimitives::writeStrBits(fxCountBits.to_string(), bitstream);

  types::Effect libraryEffect;
  bool success = true;
  for (int i = 0; i < perception.getEffectLibrarySize(); i++) {
    libraryEffect = perception.getBasisEffectAt(i);
    success &= writeLibraryEffect(libraryEffect, bitstream);
  }

  return success;
}
auto IOStream::readLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool {
  int idx = 0;
  auto effectCount = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_LIBRARY_COUNT);
  bool success = true;

  for (int i = 0; i < effectCount; i++) {
    types::Effect libraryEffect;
    success &= readLibraryEffect(libraryEffect, idx, bitstream);
    perception.addBasisEffect(libraryEffect);
  }
  return success;
}
auto IOStream::readLibraryEffect(types::Effect &libraryEffect, int &idx,
                                 std::vector<bool> &bitstream) -> bool {
  int id = IOBinaryPrimitives::readInt(bitstream, idx, FX_ID);
  libraryEffect.setId(id);

  int position = IOBinaryPrimitives::readInt(bitstream, idx, FX_POSITION);
  libraryEffect.setPosition(position);

  float phase = IOBinaryPrimitives::readFloatNBits<FX_PHASE>(bitstream, idx, -MAX_PHASE, MAX_PHASE);
  libraryEffect.setPhase(phase);

  int effectType = IOBinaryPrimitives::readInt(bitstream, idx, FX_TYPE);
  libraryEffect.setEffectType(static_cast<types::EffectType>(effectType));

  int kfCount = IOBinaryPrimitives::readInt(bitstream, idx, FX_KF_COUNT);
  for (int i = 0; i < kfCount; i++) {
    auto mask = (uint8_t)IOBinaryPrimitives::readInt(bitstream, idx, KF_MASK);
    std::optional<int> position = std::nullopt;
    std::optional<float> amplitude = std::nullopt;
    std::optional<int> frequency = std::nullopt;
    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      position = IOBinaryPrimitives::readInt(bitstream, idx, KF_POSITION);
    }

    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      amplitude = IOBinaryPrimitives::readFloatNBits<KF_AMPLITUDE>(bitstream, idx, -MAX_AMPLITUDE,
                                                                   MAX_AMPLITUDE);
    }

    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      frequency = IOBinaryPrimitives::readInt(bitstream, idx, KF_FREQUENCY);
    }
    types::Keyframe myKeyframe(position, amplitude, frequency);
    libraryEffect.addKeyframe(myKeyframe);
  }
  bool success = true;
  auto timelineEffectCount = IOBinaryPrimitives::readInt(bitstream, idx, FXLIB_TIMELINESIZE);
  for (int i = 0; i < timelineEffectCount; i++) {
    types::Effect timelineEffect;
    success &= readLibraryEffect(timelineEffect, idx, bitstream);
    libraryEffect.addTimelineEffect(timelineEffect);
  }

  return true;
}

auto IOStream::writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<FX_ID> idBits(libraryEffect.getId());
  IOBinaryPrimitives::writeStrBits(idBits.to_string(), bitstream);

  std::bitset<FX_POSITION> posBits(libraryEffect.getPosition());
  IOBinaryPrimitives::writeStrBits(posBits.to_string(), bitstream);

  IOBinaryPrimitives::writeFloatNBits<uint32_t, FX_PHASE>(libraryEffect.getPhase(), bitstream,
                                                          -MAX_PHASE, MAX_PHASE);

  std::bitset<FX_BASE> baseBits(static_cast<int>(libraryEffect.getBaseSignal()));
  IOBinaryPrimitives::writeStrBits(baseBits.to_string(), bitstream);

  std::bitset<FX_TYPE> typeBits(static_cast<int>(libraryEffect.getEffectType()));
  IOBinaryPrimitives::writeStrBits(typeBits.to_string(), bitstream);

  auto kfCount = libraryEffect.getKeyframesSize();
  std::bitset<FX_KF_COUNT> kfCountBits(kfCount);
  IOBinaryPrimitives::writeStrBits(kfCountBits.to_string(), bitstream);

  types::Keyframe keyframe;
  for (int i = 0; i < kfCount; i++) {
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
    std::bitset<KF_MASK> maskBits(mask);
    IOBinaryPrimitives::writeStrBits(maskBits.to_string(), bitstream);

    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      std::bitset<KF_POSITION> posBits(keyframe.getRelativePosition().value());
      IOBinaryPrimitives::writeStrBits(posBits.to_string(), bitstream);
    }
    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      IOBinaryPrimitives::writeFloatNBits<uint8_t, KF_AMPLITUDE>(
          keyframe.getAmplitudeModulation().value(), bitstream, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    }
    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      std::bitset<KF_POSITION> freqBits(keyframe.getFrequencyModulation().value());
      IOBinaryPrimitives::writeStrBits(freqBits.to_string(), bitstream);
    }
  }
  auto timelineEffectCount = static_cast<uint16_t>(libraryEffect.getTimelineSize());
  std::bitset<FXLIB_TIMELINESIZE> timelineSizeBits(timelineEffectCount);
  IOBinaryPrimitives::writeStrBits(timelineSizeBits.to_string(), bitstream);

  types::Effect timelineEffect;
  for (int i = 0; i < timelineEffectCount; i++) {
    timelineEffect = libraryEffect.getTimelineEffectAt(i);
    writeLibraryEffect(timelineEffect, bitstream);
  }
  return true;
}

auto IOStream::writeReferenceDevice(types::ReferenceDevice &refDevice, std::vector<bool> &bitstream)
    -> bool {
  std::vector<bool> vecBuf = std::vector<bool>();
  std::bitset<REFDEV_ID> idBits(refDevice.getId());
  IOBinaryPrimitives::writeStrBits(idBits.to_string(), bitstream);

  std::bitset<REFDEV_NAME_LENGTH> nameLengthsBits(refDevice.getName().size());
  IOBinaryPrimitives::writeStrBits(nameLengthsBits.to_string(), bitstream);

  for (char &c : refDevice.getName()) {
    std::bitset<BYTE_SIZE> descBits(c);
    IOBinaryPrimitives::writeStrBits(descBits.to_string(), bitstream);
  }

  std::bitset<REFDEV_BODY_PART_MASK> bodyPartMaskBits(refDevice.getBodyPartMask().value_or(0));
  IOBinaryPrimitives::writeStrBits(bodyPartMaskBits.to_string(), bitstream);

  generateReferenceDeviceInformationMask(refDevice, vecBuf);
  bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
  vecBuf.clear();
  if (refDevice.getMaximumFrequency().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_FREQ>(
        refDevice.getMaximumFrequency().value(), vecBuf, static_cast<float>(0), MAX_FREQUENCY);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getMinimumFrequency().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MIN_FREQ>(
        refDevice.getMinimumFrequency().value(), vecBuf, static_cast<float>(0), MAX_FREQUENCY);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getResonanceFrequency().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MIN_FREQ>(
        refDevice.getResonanceFrequency().value(), vecBuf, static_cast<float>(0), MAX_FREQUENCY);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getMaximumAmplitude().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_AMP>(
        refDevice.getMaximumAmplitude().value(), vecBuf, static_cast<float>(0), MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getImpedance().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_IMPEDANCE>(
        refDevice.getImpedance().value(), vecBuf, 0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getMaximumVoltage().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_VOLT>(
        refDevice.getMaximumVoltage().value(), vecBuf, 0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getMaximumCurrent().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_CURR>(
        refDevice.getMaximumCurrent().value(), vecBuf, 0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getMaximumDisplacement().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_MAX_DISP>(
        refDevice.getMaximumDisplacement().value(), vecBuf, 0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getWeight().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_WEIGHT>(refDevice.getWeight().value(),
                                                                 vecBuf, 0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getSize().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_SIZE>(refDevice.getSize().value(), vecBuf,
                                                               0, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getCustom().has_value()) {
    IOBinaryPrimitives::writeFloatNBits<uint32_t, REFDEV_CUSTOM>(refDevice.getCustom().value(),
                                                                 vecBuf, -MAX_FLOAT, MAX_FLOAT);
    bitstream.insert(bitstream.end(), vecBuf.begin(), vecBuf.end());
    vecBuf.clear();
  }
  if (refDevice.getType().has_value()) {
    std::bitset<REFDEV_TYPE> typeBits(static_cast<int>(refDevice.getType().value()));
    IOBinaryPrimitives::writeStrBits(typeBits.to_string(), bitstream);
  }
  return true;
}
auto IOStream::generateReferenceDeviceInformationMask(types::ReferenceDevice &referenceDevice,
                                                      std::vector<bool> &informationMask) -> bool {
  if (referenceDevice.getMaximumFrequency().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getMinimumFrequency().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getResonanceFrequency().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getMaximumAmplitude().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getImpedance().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getMaximumVoltage().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getMaximumCurrent().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getMaximumDisplacement().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getWeight().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getSize().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getCustom().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  if (referenceDevice.getType().has_value()) {
    informationMask.push_back(true);
  } else {
    informationMask.push_back(false);
  }
  return true;
}
auto IOStream::readReferenceDevice(std::vector<bool> &bitstream, types::ReferenceDevice &refDevice,
                                   int &length) -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readInt(bitstream, idx, REFDEV_ID);
  refDevice.setId(id);

  int nameLength = IOBinaryPrimitives::readInt(bitstream, idx, REFDEV_NAME_LENGTH);
  std::string name = IOBinaryPrimitives::readString(bitstream, idx, nameLength);
  refDevice.setName(name);
  int bodyPartMask = IOBinaryPrimitives::readInt(bitstream, idx, REFDEV_BODY_PART_MASK);
  refDevice.setBodyPartMask(static_cast<uint32_t>(bodyPartMask));

  std::vector<bool> mask(bitstream.begin() + idx, bitstream.begin() + idx + REFDEV_OPT_FIELDS);
  idx += REFDEV_OPT_FIELDS;
  int maskIdx = 0;
  float value = 0;
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MAX_FREQ>(bitstream, idx, 0, MAX_FREQUENCY);
    refDevice.setMaximumFrequency(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MIN_FREQ>(bitstream, idx, 0, MAX_FREQUENCY);
    refDevice.setMinimumFrequency(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_RES_FREQ>(bitstream, idx, 0, MAX_FREQUENCY);
    refDevice.setResonanceFrequency(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MAX_AMP>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setMaximumAmplitude(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_IMPEDANCE>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setImpedance(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MAX_VOLT>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setMaximumVoltage(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MAX_CURR>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setMaximumCurrent(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_MAX_DISP>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setMaximumDisplacement(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_WEIGHT>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setWeight(value);
  }
  if (mask[maskIdx++]) {
    value = IOBinaryPrimitives::readFloatNBits<REFDEV_SIZE>(bitstream, idx, 0, MAX_FLOAT);
    refDevice.setSize(value);
  }
  if (mask[maskIdx++]) {
    value =
        IOBinaryPrimitives::readFloatNBits<REFDEV_CUSTOM>(bitstream, idx, -MAX_FLOAT, MAX_FLOAT);
    refDevice.setCustom(value);
  }
  if (mask[maskIdx++]) {
    int type = IOBinaryPrimitives::readInt(bitstream, idx, REFDEV_TYPE);
    refDevice.setType(static_cast<types::ActuatorType>(type));
  }
  length += idx;
  return true;
}

auto IOStream::writeMetadataTrack(types::Track &track, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDTRACK_ID> idBits(track.getId());
  IOBinaryPrimitives::writeStrBits(idBits.to_string(), bitstream);

  std::bitset<MDPERCE_DESC_SIZE> descSizeBits(track.getDescription().size());
  IOBinaryPrimitives::writeStrBits(descSizeBits.to_string(), bitstream);

  for (char &c : track.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    IOBinaryPrimitives::writeStrBits(descBits.to_string(), bitstream);
  }

  std::bitset<REFDEV_ID> refDevID(track.getReferenceDeviceId().value_or(-1));
  IOBinaryPrimitives::writeStrBits(refDevID.to_string(), bitstream);

  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_GAIN>(track.getGain(), bufBits, -MAX_FLOAT,
                                                              MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_MIXING_WEIGHT>(track.getMixingWeight(),
                                                                       bufBits, 0, MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  std::bitset<MDTRACK_BODY_PART_MASK> bodyPartMaskBits(track.getBodyPartMask());
  IOBinaryPrimitives::writeStrBits(bodyPartMaskBits.to_string(), bitstream);

  int freqSampling = static_cast<int>(track.getFrequencySampling().value_or(0));
  std::bitset<MDTRACK_SAMPLING_FREQUENCY> maxFreqBits(freqSampling);
  IOBinaryPrimitives::writeStrBits(maxFreqBits.to_string(), bitstream);

  if (track.getFrequencySampling().value_or(0) > 0) {
    std::bitset<REFDEV_MAX_FREQ> sampleCountBits(track.getSampleCount().value_or(0));
    IOBinaryPrimitives::writeStrBits(sampleCountBits.to_string(), bitstream);
  }

  if (track.getDirection().has_value()) {
    types::Direction dir = track.getDirection().value();
    std::bitset<1> flagBit(1);
    IOBinaryPrimitives::writeStrBits(flagBit.to_string(), bitstream);
    std::bitset<MDTRACK_DIRECTION_AXIS> axisValue(dir.X);
    IOBinaryPrimitives::writeStrBits(axisValue.to_string(), bitstream);
    axisValue = std::bitset<MDTRACK_DIRECTION_AXIS>(dir.Y);
    IOBinaryPrimitives::writeStrBits(axisValue.to_string(), bitstream);
    axisValue = std::bitset<MDTRACK_DIRECTION_AXIS>(dir.Z);
    IOBinaryPrimitives::writeStrBits(axisValue.to_string(), bitstream);
  } else {
    std::bitset<1> flagBit(0);
    IOBinaryPrimitives::writeStrBits(flagBit.to_string(), bitstream);
  }

  std::bitset<MDTRACK_VERT_COUNT> vertCountBits(track.getVerticesSize());
  IOBinaryPrimitives::writeStrBits(vertCountBits.to_string(), bitstream);

  for (int i = 0; i < track.getVerticesSize(); i++) {
    std::bitset<MDTRACK_VERT> vertBits(track.getVertexAt(i));
    IOBinaryPrimitives::writeStrBits(vertBits.to_string(), bitstream);
  }

  std::bitset<MDTRACK_BANDS_COUNT> bandsCountBits(track.getBandsSize());
  IOBinaryPrimitives::writeStrBits(bandsCountBits.to_string(), bitstream);

  return true;
}
auto IOStream::readMetadataTrack(types::Track &track, std::vector<bool> &bitstream) -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_ID);
  track.setId(id);

  int descLength = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_DESC_LENGTH);
  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  track.setDescription(desc);

  int deviceId = IOBinaryPrimitives::readInt(bitstream, idx, REFDEV_ID);
  if (deviceId > -1) {
    track.setReferenceDeviceId(deviceId);
  }
  float gain =
      IOBinaryPrimitives::readFloatNBits<MDTRACK_GAIN>(bitstream, idx, -MAX_FLOAT, MAX_FLOAT);
  track.setGain(gain);

  float mixingWeight =
      IOBinaryPrimitives::readFloatNBits<MDTRACK_MIXING_WEIGHT>(bitstream, idx, 0, MAX_FLOAT);
  track.setMixingWeight(mixingWeight);

  uint32_t bodyPartMask = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_BODY_PART_MASK);
  track.setBodyPartMask(bodyPartMask);

  uint32_t frequencySampling =
      IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_SAMPLING_FREQUENCY);
  if (frequencySampling > 0) {
    track.setFrequencySampling(frequencySampling);
  }
  if (frequencySampling > 0) {
    uint32_t sampleCount = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_SAMPLE_COUNT);
  }

  bool directionMask =
      static_cast<bool>(IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_DIRECTION_MASK));
  if (directionMask) {
    types::Direction direction = types::Direction();
    direction.X =
        static_cast<int8_t>(IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    direction.Y =
        static_cast<int8_t>(IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    direction.Z =
        static_cast<int8_t>(IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    track.setDirection(direction);
  }

  int verticesCount = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_VERT_COUNT);
  for (int i = 0; i < verticesCount; i++) {
    int vertex = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_VERT);
    track.addVertex(vertex);
  }

  int bandsCount = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_BANDS_COUNT);

  return true;
}

auto IOStream::writeMetadataBand(types::Band &band, std::vector<bool> &bitstream, int id) -> bool {
  std::bitset<MDBAND_ID> idBits(id);
  IOBinaryPrimitives::writeStrBits(idBits.to_string(), bitstream);

  std::bitset<MDBAND_BAND_TYPE> bandTypeBits(static_cast<int>(band.getBandType()));
  IOBinaryPrimitives::writeStrBits(bandTypeBits.to_string(), bitstream);
  if (band.getBandType() == types::BandType::Curve) {
    std::bitset<MDBAND_CURVE_TYPE> curveTypeBits(static_cast<int>(band.getCurveType()));
    IOBinaryPrimitives::writeStrBits(curveTypeBits.to_string(), bitstream);
  } else if (band.getBandType() == types::BandType::WaveletWave) {
    std::bitset<MDBAND_WIN_LEN> winLengthBits((band.getWindowLength()));
    IOBinaryPrimitives::writeStrBits(winLengthBits.to_string(), bitstream);
  }
  std::bitset<MDBAND_LOW_FREQ> lowFreqBits((band.getLowerFrequencyLimit()));
  IOBinaryPrimitives::writeStrBits(lowFreqBits.to_string(), bitstream);

  std::bitset<MDBAND_UP_FREQ> upFreqBits((band.getUpperFrequencyLimit()));
  IOBinaryPrimitives::writeStrBits(upFreqBits.to_string(), bitstream);

  std::bitset<MDBAND_FX_COUNT> effectCountBits(band.getEffectsSize());
  IOBinaryPrimitives::writeStrBits(effectCountBits.to_string(), bitstream);

  return true;
}
auto IOStream::readMetadataBand(BandStream &bandStream, std::vector<bool> &bitstream) -> bool {
  int idx = 0;
  bandStream.id = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_ID);

  int bandType = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_BAND_TYPE);
  bandStream.band.setBandType(static_cast<types::BandType>(bandType));
  if (bandStream.band.getBandType() == types::BandType::Curve) {
    int curveType = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_CURVE_TYPE);
    bandStream.band.setCurveType(static_cast<types::CurveType>(curveType));
  } else if (bandStream.band.getBandType() == types::BandType::WaveletWave) {
    int windowLength = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_WIN_LEN);
    bandStream.band.setWindowLength(windowLength);
  }
  int lowFreq = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_LOW_FREQ);
  bandStream.band.setLowerFrequencyLimit(lowFreq);
  int upFreq = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_UP_FREQ);
  bandStream.band.setUpperFrequencyLimit(upFreq);

  int effectsCountBits = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_FX_COUNT);
  return true;
}

auto IOStream::readListObject(std::vector<bool> &bitstream, int avatarCount,
                              std::vector<types::Avatar> &avatarList) -> bool {
  int idx = 0;
  for (int i = 0; i < avatarCount; i++) {
    std::vector<bool> avatarBits(bitstream.begin() + idx, bitstream.end());
    types::Avatar avatar;
    if (!readAvatar(avatarBits, avatar, idx)) {
      return false;
    }
    avatarList.push_back(avatar);
  }
  return true;
}

auto IOStream::sortPacket(std::vector<std::vector<std::vector<bool>>> &bandPacket,
                          std::vector<std::vector<bool>> &output) -> bool {
  for (auto &band : bandPacket) {
    for (auto &packet : band) {
      if (!output.empty()) {
        bool sorted = false;
        int k = 0;
        while (!sorted) {
          if (k == output.size()) {
            sorted = true;
            output.push_back(packet);
          } else {
            std::vector<bool> sortedP = output[k];
            int sortedPTime = readPacketTS(sortedP);
            int packetTime = readPacketTS(packet);
            if (sortedPTime > packetTime) {
              sorted = true;
              output.insert(output.begin() + k, packet);
            }
          }
          k++;
        }
      } else {
        output.push_back(packet);
      }
    }
  }
  return true;
}
auto IOStream::packetizeBand(int perceID, int trackID, BandStream &bandStream,
                             std::vector<std::vector<bool>> &bitstreams,
                             std::vector<int> &effectsId) -> bool {
  // Exit this function when all the band is packetised
  StartTimeIdx point;
  StartTimeIdx percetrackID;
  percetrackID.time = perceID;
  percetrackID.idx = trackID;

  std::vector<std::vector<bool>> bufPacketBitstream = std::vector<std::vector<bool>>();
  std::vector<bool> packetBits = std::vector<bool>();
  bool rau = true;
  std::vector<types::Effect> vecEffect = std::vector<types::Effect>();
  std::vector<int> kfCount = std::vector<int>();

  while (!createPayloadPacket(bandStream.band, point, vecEffect, kfCount, rau, bufPacketBitstream,
                              effectsId)) {
    // write packet as vector<bool>
    if (!bufPacketBitstream.empty()) {
      packetBits = writeEffectHeader(point, percetrackID, rau, bandStream);
      packetBits = writePayloadPacket(vecEffect, bandStream, kfCount, bufPacketBitstream,
                                      packetBits, point.time);

      bitstreams.push_back(packetBits);
    }
    vecEffect.clear();
    bufPacketBitstream.clear();
    packetBits.clear();
    kfCount.clear();
    point.time += PACKET_DURATION;
  }
  if (!bufPacketBitstream.empty()) {
    packetBits = writeEffectHeader(point, percetrackID, rau, bandStream);
    packetBits = writePayloadPacket(vecEffect, bandStream, kfCount, bufPacketBitstream, packetBits,
                                    (point.time - PACKET_DURATION));
    bitstreams.push_back(packetBits);
  }

  return true;
}
auto IOStream::setNextEffectId(std::vector<int> &effectsId, types::Effect &effect) -> bool {
  int nextId = 0;
  if (!effectsId.empty()) {
    nextId = *max_element(effectsId.begin(), effectsId.end()) + 1;
  }
  effect.setId(nextId);
  effectsId.push_back(nextId);
  return true;
}

auto IOStream::createPayloadPacket(types::Band &band, StartTimeIdx &startTI,
                                   std::vector<types::Effect> &vecEffect, std::vector<int> &kfCount,
                                   bool &rau, std::vector<std::vector<bool>> &bitstream,
                                   std::vector<int> &effectsId) -> bool {

  // Exit this function only when 1 packet is full or last keyframes of the band is reached

  for (int i = 0; i < band.getEffectsSize(); i++) {
    bool endEffect = false;
    types::Effect &effect = band.getEffectAt(i);
    if (effect.getId() == -1) {
      int nextId = 0;
      if (!effectsId.empty()) {
        nextId = *max_element(effectsId.begin(), effectsId.end()) + 1;
      }
      effect.setId(nextId);
      effectsId.push_back(nextId);
    }

    std::vector<bool> bufEffect = std::vector<bool>();
    if (effect.getEffectType() == types::EffectType::Basis &&
        band.getBandType() != types::BandType::WaveletWave) {
      int bufKFCount = 0;
      bool isRAU = true;
      bool endPacket = false;
      if (writeEffectBasis(effect, band.getBandType(), startTI, bufKFCount, isRAU, bufEffect)) {
        endEffect = true;
      } else {
        endPacket = true;
      }
      if (bufKFCount > 0) {
        kfCount.push_back(bufKFCount);
        bitstream.push_back(bufEffect);
        vecEffect.push_back(effect);
      }
      rau = isRAU;
      if (endEffect && i == band.getEffectsSize() - 1) {
        return true;
      }
      if (endPacket) {
        return false;
      }
    } else if (effect.getEffectType() == types::EffectType::Basis &&
               band.getBandType() == types::BandType::WaveletWave) {
      writeEffectWavelet(effect, startTI, bufEffect);
      rau = true;
      // Create a new waveletEffect for each packet, need to force the creation of a new id
      setNextEffectId(effectsId, effect);
      vecEffect.push_back(effect);
      bitstream.push_back(bufEffect);
    } else if (effect.getEffectType() == types::EffectType::Reference) {
      if (effect.getPosition() >= startTI.time &&
          effect.getPosition() <= startTI.time + PACKET_DURATION) {
        std::bitset<FX_REF_ID> referenceIDBits(effect.getId());
        IOBinaryPrimitives::writeStrBits(referenceIDBits.to_string(), bufEffect);
        bitstream.push_back(bufEffect);
        vecEffect.push_back(effect);
        rau = true;
      } else if (effect.getPosition() > startTI.time + PACKET_DURATION) {
        return false;
      }
      if (i == band.getEffectsSize() - 1) {
        return true;
      }
      //} else if (effect.getEffectType() == types::EffectType::Timeline) {
      //  // writeTimeline()
      //  return true;
    } else {
      return true;
    }
  }
  return true;
}

auto IOStream::writeEffectHeader(StartTimeIdx point, StartTimeIdx percetrackID, bool &rau,
                                 BandStream &bandStream) -> std::vector<bool> {
  std::vector<bool> packetBits = std::vector<bool>();
  packetBits.push_back(rau);
  // packetBits.push_back(rau); // PUSH END ACCESS UNIT
  std::bitset<DB_TIMESTAMP> tsBits(point.time);
  IOBinaryPrimitives::writeStrBits(tsBits.to_string(), packetBits);

  std::bitset<MDPERCE_ID> perceIdBits(percetrackID.time);
  IOBinaryPrimitives::writeStrBits(perceIdBits.to_string(), packetBits);

  std::bitset<MDTRACK_ID> trackIdBits(percetrackID.idx);
  IOBinaryPrimitives::writeStrBits(trackIdBits.to_string(), packetBits);

  std::bitset<MDBAND_ID> bandIdBits(bandStream.id);
  IOBinaryPrimitives::writeStrBits(bandIdBits.to_string(), packetBits);

  return packetBits;
}

auto IOStream::writePayloadPacket(std::vector<types::Effect> &vecEffect, BandStream &bandStream,
                                  std::vector<int> kfCount,
                                  std::vector<std::vector<bool>> bufPacketBitstream,
                                  std::vector<bool> &packetBits, int time) -> std::vector<bool> {
  std::bitset<DB_FX_COUNT> fxCountBits(vecEffect.size());
  IOBinaryPrimitives::writeStrBits(fxCountBits.to_string(), packetBits);

  for (int l = 0; l < vecEffect.size(); l++) {
    std::bitset<FX_ID> effectIDBits(static_cast<int>(vecEffect[l].getId()));
    IOBinaryPrimitives::writeStrBits(effectIDBits.to_string(), packetBits);

    std::bitset<FX_TYPE> effectTypeBits(static_cast<int>(vecEffect[l].getEffectType()));
    IOBinaryPrimitives::writeStrBits(effectTypeBits.to_string(), packetBits);

    std::bitset<FX_POSITION> effectPosBits(static_cast<int>(vecEffect[l].getPosition()) - time);
    IOBinaryPrimitives::writeStrBits(effectPosBits.to_string(), packetBits);

    if (vecEffect[l].getEffectType() == types::EffectType::Basis &&
        bandStream.band.getBandType() != types::BandType::WaveletWave) {
      std::bitset<FX_KF_COUNT> kfCountBits(kfCount[l]);
      IOBinaryPrimitives::writeStrBits(kfCountBits.to_string(), packetBits);
      if (bandStream.band.getBandType() == types::BandType::VectorialWave) {
        IOBinaryPrimitives::writeFloatNBits<uint16_t, FX_PHASE>(vecEffect[l].getPhase(), packetBits,
                                                                -MAX_PHASE, MAX_PHASE);
        std::bitset<FX_BASE> fxBaseBits(static_cast<int>(vecEffect[l].getBaseSignal()));
        IOBinaryPrimitives::writeStrBits(fxBaseBits.to_string(), packetBits);
      }
    }

    packetBits.insert(packetBits.end(), bufPacketBitstream[l].begin(), bufPacketBitstream[l].end());
  }
  return packetBits;
}

auto IOStream::writeData(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream)
    -> bool {
  std::vector<int> effectsId = getEffectsId(haptic);
  std::vector<std::vector<std::vector<bool>>> expBitstream =
      std::vector<std::vector<std::vector<bool>>>();
  std::vector<std::vector<bool>> bandBitstream = std::vector<std::vector<bool>>();
  int bandId = 0;
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    int perceptionID = perception.getId();
    std::bitset<MDPERCE_ID> perceptionIDBits(perceptionID);
    for (int j = 0; j < perception.getTracksSize(); j++) {
      types::Track track = perception.getTrackAt(j);
      int trackID = track.getId();
      std::bitset<MDTRACK_ID> trackIDBits(trackID);
      for (int k = 0; k < track.getBandsSize(); k++) {
        BandStream bandStream;
        bandStream.id = bandId++;
        bandStream.band = track.getBandAt(k);

        packetizeBand(perceptionID, trackID, bandStream, bandBitstream, effectsId);
        expBitstream.push_back(bandBitstream);
        bandBitstream.clear();
      }
    }
  }
  sortPacket(expBitstream, bitstream);
  return true;
  // Function to order all packet depending on time
}

auto IOStream::getEffectsId(types::Haptics &haptic) -> std::vector<int> {
  std::vector<int> effectsId = std::vector<int>();
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    for (int j = 0; j < perception.getTracksSize(); j++) {
      types::Track track = perception.getTrackAt(j);
      for (int k = 0; k < track.getBandsSize(); k++) {
        types::Band band = track.getBandAt(k);
        for (int l = 0; l < band.getEffectsSize(); l++) {
          types::Effect effect = band.getEffectAt(l);
          if (effect.getId() != -1) {
            effectsId.push_back(effect.getId());
          }
        }
      }
    }
  }
  return effectsId;
}
auto IOStream::readData(types::Haptics &haptic, Buffer &buffer, std::vector<bool> &bitstream)
    -> bool {
  int idx = 0;
  bool auType = IOBinaryPrimitives::readInt(bitstream, idx, DB_AU_TYPE) == 1;
  int timestamp = IOBinaryPrimitives::readInt(bitstream, idx, DB_TIMESTAMP);

  int perceptionId = IOBinaryPrimitives::readInt(bitstream, idx, MDPERCE_ID);
  types::Perception perception;
  auto perceptionIndex = searchPerceptionInHaptic(haptic, perceptionId);
  if (perceptionIndex == -1) {
    if (!searchInList<types::Perception>(buffer.perceptionsBuffer, perception, perceptionId)) {
      return false;
    }
    haptic.addPerception(perception);
    perceptionIndex = static_cast<int>(haptic.getPerceptionsSize()) - 1;
  }

  int trackId = IOBinaryPrimitives::readInt(bitstream, idx, MDTRACK_ID);
  types::Track track;
  auto trackIndex = searchTrackInHaptic(haptic, trackId);
  if (trackIndex == -1) {
    if (!searchInList<types::Track>(buffer.tracksBuffer, track, trackId)) {
      return false;
    }
    haptic.getPerceptionAt(perceptionIndex).addTrack(track);
    trackIndex = static_cast<int>(haptic.getPerceptionAt(perceptionIndex).getTracksSize()) - 1;
  }

  int bandId = IOBinaryPrimitives::readInt(bitstream, idx, MDBAND_ID);
  BandStream bandStream;
  auto bandIndex = searchBandInHaptic(buffer, bandId);
  if (bandIndex == -1) {
    if (!searchInList(buffer.bandStreamsBuffer, bandStream, bandId)) {
      return false;
    }
    haptic.getPerceptionAt(perceptionIndex).getTrackAt(trackIndex).addBand(bandStream.band);
    bandIndex = static_cast<int>(
                    haptic.getPerceptionAt(perceptionIndex).getTrackAt(trackIndex).getBandsSize()) -
                1;
    bandStream.index = bandIndex;
    buffer.bandStreamsHaptic.push_back(bandStream);
  }

  types::Band band =
      haptic.getPerceptionAt(perceptionIndex).getTrackAt(trackIndex).getBandAt(bandIndex);

  int fxCount = IOBinaryPrimitives::readInt(bitstream, idx, DB_FX_COUNT);
  std::vector<types::Effect> effects;
  std::vector<bool> effectsBitsList(bitstream.begin() + idx, bitstream.end());
  if (!readListObject(effectsBitsList, fxCount, band, effects, idx)) {
    return false;
  }
  addTimestampEffect(effects, timestamp);
  if (!addEffectToHaptic(haptic, perceptionIndex, trackIndex, bandIndex, effects)) {
    return false;
  }
  return true;
}

auto IOStream::addTimestampEffect(std::vector<types::Effect> &effects, int timestamp) -> bool {
  for (auto &e : effects) {
    int relativeTime = e.getPosition();
    e.setPosition(relativeTime + timestamp);
  }
  return true;
}

auto IOStream::readEffect(std::vector<bool> &bitstream, types::Effect &effect, types::Band &band,
                          int &length) -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readInt(bitstream, idx, FX_ID);
  effect.setId(id);

  types::EffectType effectType =
      static_cast<types::EffectType>(IOBinaryPrimitives::readInt(bitstream, idx, FX_TYPE));
  effect.setEffectType(effectType);

  int effectPos = IOBinaryPrimitives::readInt(bitstream, idx, FX_POSITION);
  effect.setPosition(effectPos);

  if (effectType == types::EffectType::Basis &&
      band.getBandType() != types::BandType::WaveletWave) {
    if (!readEffectBasis(bitstream, effect, band.getBandType(), idx)) {
      return false;
    }
  } else if (effectType == types::EffectType::Basis &&
             band.getBandType() != types::BandType::WaveletWave) {
    IOBinaryBands::readWaveletEffect(effect, band, bitstream, idx);
  }
  length += idx;
  return true;
}

auto IOStream::addEffectToHaptic(types::Haptics &haptic, int perceptionIndex, int trackIndex,
                                 int bandIndex, std::vector<types::Effect> &effects) -> bool {
  for (auto &effect : effects) {
    bool effectExist = false;
    types::Band &band =
        haptic.getPerceptionAt(perceptionIndex).getTrackAt(trackIndex).getBandAt(bandIndex);
    for (int i = 0; i < band.getEffectsSize(); i++) {
      types::Effect &hapticEffect = haptic.getPerceptionAt(perceptionIndex)
                                        .getTrackAt(trackIndex)
                                        .getBandAt(bandIndex)
                                        .getEffectAt(i);

      if (effect.getId() == hapticEffect.getId()) {
        for (int j = 0; j < effect.getKeyframesSize(); j++) {
          hapticEffect.addKeyframe(effect.getKeyframeAt(j));
        }
        effectExist = true;
        break;
      }
    }
    if (!effectExist) {
      band.addEffect(effect);
    }
  }
  return true;
}

auto IOStream::writeEffectBasis(types::Effect effect, types::BandType bandType,
                                StartTimeIdx &startTI, int &kfCount, bool &rau,
                                std::vector<bool> &bitstream) -> bool {
  bool firstKf = true;
  int tsFX = effect.getPosition();
  for (int j = 0; j < effect.getKeyframesSize(); j++) {
    types::Keyframe kf = effect.getKeyframeAt(j);
    int currentTime = kf.getRelativePosition().value() + tsFX;
    if (currentTime <= startTI.time + PACKET_DURATION && currentTime >= startTI.time) {
      if (firstKf) {
        firstKf = false;
        if (j != 0) {
          rau = false;
        }
      }
      writeKeyframe(bandType, kf, bitstream);
      kfCount++;
      if (j == effect.getKeyframesSize() - 1) {
        return true;
      }
    } else if (currentTime > startTI.time + PACKET_DURATION) {
      return false;
    }
  }
  return true;
}

auto IOStream::readEffectBasis(std::vector<bool> &bitstream, types::Effect &effect,
                               types::BandType bandType, int &idx) -> bool {
  int kfCount = IOBinaryPrimitives::readInt(bitstream, idx, FX_KF_COUNT);
  if (bandType == types::BandType::VectorialWave) {
    float phase =
        IOBinaryPrimitives::readFloatNBits<FX_PHASE>(bitstream, idx, -MAX_PHASE, MAX_PHASE);
    effect.setPhase(phase);
    int baseSignal = IOBinaryPrimitives::readInt(bitstream, idx, FX_BASE);
    effect.setBaseSignal(static_cast<types::BaseSignal>(baseSignal));
  }
  std::vector<types::Keyframe> keyframes = std::vector<types::Keyframe>();
  std::vector<bool> kfBitsList(bitstream.begin() + idx, bitstream.end());
  if (!readListObject(kfBitsList, kfCount, bandType, keyframes, idx)) {
    return false;
  }

  for (auto kf : keyframes) {
    effect.addKeyframe(kf);
  }
  return true;
}

auto IOStream::writeEffectWavelet(types::Effect &effect, StartTimeIdx startTI,
                                  std::vector<bool> &bitstream) -> bool {
  int tsFX = effect.getPosition();
  types::Effect packetEffect = effect;
  packetEffect.setPosition(0); // position relative to the AU timestamp

  for (int j = static_cast<int>(effect.getKeyframesSize()) - 1; j >= 0; j--) {
    types::Keyframe kf = effect.getKeyframeAt(j);
    int currentTime = kf.getRelativePosition().value() + tsFX;
    if (currentTime < startTI.time || currentTime > startTI.time + PACKET_DURATION) {
      packetEffect.removeKeyframeAt(j);
    }
  }
  IOBinaryBands::writeWaveletEffect(packetEffect, bitstream);
  return true;
}

// auto writeEffectTimeline(types::Effect &effect, std::vector<bool> &bitstream) -> bool {
//   return true;
// }
auto IOStream::writeKeyframe(types::BandType bandType, types::Keyframe &keyframe,
                             std::vector<bool> &bitstream) -> bool {
  switch (bandType) {
  case types::BandType::Transient:
    return writeTransient(keyframe, bitstream);
  case types::BandType::Curve:
    return writeCurve(keyframe, bitstream);
  case types::BandType::VectorialWave:
    return writeVectorial(keyframe, bitstream);
  // case types::BandType::WaveletWave:
  //   return writeWavelet(bitstream); // not implemented
  default:
    return false;
  }
  return true;
}
auto IOStream::readKeyframe(std::vector<bool> &bitstream, types::Keyframe &keyframe,
                            types::BandType &bandType, int &length) -> bool {
  switch (bandType) {
  case types::BandType::Transient:
    return readTransient(bitstream, keyframe, length);
  case types::BandType::Curve:
    return readCurve(bitstream, keyframe, length);
  case types::BandType::VectorialWave:
    return readVectorial(bitstream, keyframe, length);
  default:
    return false;
  }
  return false;
}

auto IOStream::writeTransient(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint8_t, KF_AMPLITUDE>(
      keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());

  std::bitset<KF_POSITION> posBits(keyframe.getRelativePosition().value());
  IOBinaryPrimitives::writeStrBits(posBits.to_string(), bitstream);

  std::bitset<KF_FREQUENCY> freqBits(keyframe.getFrequencyModulation().value());
  IOBinaryPrimitives::writeStrBits(freqBits.to_string(), bitstream);
  return true;
}
auto IOStream::readTransient(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  float amplitude = IOBinaryPrimitives::readFloatNBits<KF_AMPLITUDE>(bitstream, idx, -MAX_AMPLITUDE,
                                                                     MAX_AMPLITUDE);
  keyframe.setAmplitudeModulation(amplitude);
  int position = IOBinaryPrimitives::readInt(bitstream, idx, KF_POSITION);
  keyframe.setRelativePosition(position);
  int frequency = IOBinaryPrimitives::readInt(bitstream, idx, KF_FREQUENCY);
  keyframe.setFrequencyModulation(frequency);
  length += idx;
  return true;
}

auto IOStream::writeCurve(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint8_t, KF_AMPLITUDE>(
      keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  std::bitset<KF_POSITION> posBits(keyframe.getRelativePosition().value());
  IOBinaryPrimitives::writeStrBits(posBits.to_string(), bitstream);
  return true;
}
auto IOStream::readCurve(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  float amplitude = IOBinaryPrimitives::readFloatNBits<KF_AMPLITUDE>(bitstream, idx, -MAX_AMPLITUDE,
                                                                     MAX_AMPLITUDE);
  keyframe.setAmplitudeModulation(amplitude);
  int position = IOBinaryPrimitives::readInt(bitstream, idx, KF_POSITION);
  keyframe.setRelativePosition(position);
  length += idx;
  return true;
}

auto IOStream::writeVectorial(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufbitstream = std::vector<bool>();
  std::bitset<2> informationMask{'00'};
  if (keyframe.getAmplitudeModulation().has_value()) {
    std::vector<bool> bufBits = std::vector<bool>();
    IOBinaryPrimitives::writeFloatNBits<uint8_t, KF_AMPLITUDE>(
        keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    bufbitstream.insert(bufbitstream.end(), bufBits.begin(), bufBits.end());
    informationMask |= 0b01;
  }
  std::bitset<KF_POSITION> posBits(keyframe.getRelativePosition().value());
  IOBinaryPrimitives::writeStrBits(posBits.to_string(), bufbitstream);

  if (keyframe.getFrequencyModulation().has_value()) {
    std::bitset<KF_FREQUENCY> freqBits(keyframe.getFrequencyModulation().value());
    IOBinaryPrimitives::writeStrBits(freqBits.to_string(), bufbitstream);
    informationMask |= 0b10;
  }
  IOBinaryPrimitives::writeStrBits(informationMask.to_string(), bitstream);
  bitstream.insert(bitstream.end(), bufbitstream.begin(), bufbitstream.end());
  return true;
}
auto IOStream::readVectorial(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  std::bitset<KF_INFORMATION_MASK> informationMask(
      IOBinaryPrimitives::readInt(bitstream, idx, KF_INFORMATION_MASK));
  if (static_cast<int>(informationMask[0]) == 1) {
    float amplitude = IOBinaryPrimitives::readFloatNBits<KF_AMPLITUDE>(
        bitstream, idx, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    keyframe.setAmplitudeModulation(amplitude);
  }
  int position = IOBinaryPrimitives::readInt(bitstream, idx, KF_POSITION);
  keyframe.setRelativePosition(position);
  if (static_cast<int>(informationMask[1]) == 1) {
    int frequency = IOBinaryPrimitives::readInt(bitstream, idx, KF_FREQUENCY);
    keyframe.setFrequencyModulation(frequency);
  }
  length += idx;
  return true;
}

// auto IOStream::writeCRC(std::vector<bool> &bitstream) -> bool { return false; }

auto IOStream::searchPerceptionInHaptic(types::Haptics &haptic, int id) -> int {
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    if (id == haptic.getPerceptionAt(i).getId()) {
      return i;
    }
  }
  return -1;
}
auto IOStream::searchTrackInHaptic(types::Haptics &haptic, int id) -> int {
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    for (int j = 0; j < perception.getTracksSize(); j++) {
      if (id == perception.getTrackAt(j).getId()) {
        return j;
      }
    }
  }
  return -1;
}

auto IOStream::searchBandInHaptic(Buffer &buffer, int id) -> int {
  for (auto &bandBuf : buffer.bandStreamsHaptic) {
    if (bandBuf.id == id) {
      return bandBuf.index;
    }
  }
  return -1;
}

template <class T> auto IOStream::searchInList(std::vector<T> &list, T &item, int id) -> bool {
  for (int i = 0; i < list.size(); i++) {
    if (list[i].getId() == id) {
      item = list[i];
      list.erase(list.begin() + i);
      return true;
    }
  }
  return false;
}
auto IOStream::searchInList(std::vector<BandStream> &list, BandStream &item, int id) -> bool {
  for (int i = 0; i < list.size(); i++) {
    if (list[i].id == id) {
      item = list[i];
      list.erase(list.begin() + i);

      return true;
    }
  }
  return false;
}

auto IOStream::readListObject(std::vector<bool> &bitstream, int refDevCount,
                              std::vector<types::ReferenceDevice> &refDevList) -> bool {
  int idx = 0;
  for (int i = 0; i < refDevCount; i++) {
    std::vector<bool> refDevBits(bitstream.begin() + idx, bitstream.end());
    types::ReferenceDevice refDev;
    if (!readReferenceDevice(refDevBits, refDev, idx)) {
      return false;
    }
    refDevList.push_back(refDev);
  }
  return true;
}
auto IOStream::readListObject(std::vector<bool> &bitstream, int fxCount, types::Band &band,
                              std::vector<types::Effect> &fxList, int &length) -> bool {
  int idx = 0;
  for (int i = 0; i < fxCount; i++) {
    std::vector<bool> fxBits(bitstream.begin() + idx, bitstream.end());
    types::Effect effect;
    if (!readEffect(fxBits, effect, band, idx)) {
      return false;
    }
    fxList.push_back(effect);
  }
  length += idx;
  return true;
}
auto IOStream::readListObject(std::vector<bool> &bitstream, int kfCount, types::BandType &bandType,
                              std::vector<types::Keyframe> &kfList, int &length) -> bool {
  int idx = 0;
  for (int i = 0; i < kfCount; i++) {
    std::vector<bool> kfBits(bitstream.begin() + idx, bitstream.end());
    types::Keyframe keyframe;
    if (!readKeyframe(kfBits, keyframe, bandType, idx)) {
      return false;
    }
    kfList.push_back(keyframe);
  }
  length += idx;
  return true;
}

auto IOStream::padToByteBoundary(std::vector<bool> &bitstream) -> void {
  int byte_stuffing = BYTE_SIZE - (static_cast<int>(bitstream.size()) % BYTE_SIZE);
  for (int i = 0; i < byte_stuffing; i++) {
    bitstream.push_back(false);
  }
}
} // namespace haptics::io