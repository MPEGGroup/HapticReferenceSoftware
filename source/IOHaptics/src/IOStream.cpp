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
#include <IOHaptics/include/IOBinaryFields.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <IOHaptics/include/IOStream.h>

namespace haptics::io {

auto IOStream::writeFile(types::Haptics &haptic, const std::string &filePath, int packetDuration)
    -> bool {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    return false;
  }

  std::vector<std::vector<bool>> packetsBytes = std::vector<std::vector<bool>>();
  bool success = writeUnits(haptic, packetsBytes, packetDuration);
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
  StreamReader sreader = initializeStream();
  CRC crc;
  int index = 0;
  for (auto &packet : bitstream) {
    if (!readMIHSUnit(packet, sreader, crc)) {
      return EXIT_FAILURE;
    }
    if (crc.nbPackets != 0) {
      if (!checkCRC(bitstream, crc)) {
        sreader.waitSync = true;
      }
    }
    index++;
  }
  haptic = sreader.haptic;
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
    int unitNBits = UNIT_TYPE + UNIT_SYNC + UNIT_DURATION + UNIT_LENGTH;
    IOBinaryPrimitives::readNBytes(file, static_cast<int>(unitNBits / BYTE_SIZE), bufPacket);
    byteCount += static_cast<int>(H_NBITS / BYTE_SIZE);
    // read packet payload length
    int lengthIdx = unitNBits - UNIT_LENGTH;
    int bytesToRead = IOBinaryPrimitives::readUInt(bufPacket, lengthIdx, UNIT_LENGTH);

    // int bytesToRead = readPacketLength(bufPacket);
    //  read paylaod
    IOBinaryPrimitives::readNBytes(file, bytesToRead, bufPacket);
    byteCount += bytesToRead;
    bitset.push_back(bufPacket);
  }
  file.close();
  return true;
}

auto IOStream::writePacket(types::Haptics &haptic, std::ofstream &file) -> bool {
  std::vector<std::vector<bool>> bitstream = std::vector<std::vector<bool>>();
  StreamWriter swriter;
  swriter.haptic = haptic;
  writeNALu(NALuType::MetadataHaptics, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataPerception, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataTrack, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataBand, swriter, 0, bitstream);
  writeNALu(NALuType::Data, swriter, 0, bitstream);

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
auto IOStream::writePacket(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream,
                           int packetDuration) -> bool {
  StreamWriter swriter;
  swriter.haptic = haptic;
  swriter.packetDuration = packetDuration;
  writeNALu(NALuType::MetadataHaptics, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataPerception, swriter, 0, bitstream);
  writeNALu(NALuType::EffectLibrary, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataTrack, swriter, 0, bitstream);
  writeNALu(NALuType::MetadataBand, swriter, 0, bitstream);
  writeNALu(NALuType::Data, swriter, 0, bitstream);

  return true;
}

auto IOStream::writeUnits(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream,
                          int packetDuration) -> bool {
  StreamWriter swriter;
  swriter.haptic = haptic;
  swriter.packetDuration = packetDuration;
  std::vector<std::vector<bool>> initPackets = std::vector<std::vector<bool>>();
  writeNALu(NALuType::MetadataHaptics, swriter, 0, initPackets);
  writeNALu(NALuType::MetadataPerception, swriter, 0, initPackets);
  writeNALu(NALuType::EffectLibrary, swriter, 0, initPackets);
  writeNALu(NALuType::MetadataTrack, swriter, 0, initPackets);
  writeNALu(NALuType::MetadataBand, swriter, 0, initPackets);
  std::vector<bool> initUnit = std::vector<bool>();
  writeMIHSUnit(MIHSUnitType::Initialization, initPackets, initUnit, swriter);
  bitstream.push_back(initUnit);

  std::vector<std::vector<bool>> dataPackets = std::vector<std::vector<bool>>();
  writeNALu(NALuType::Data, swriter, 0, dataPackets);
  std::vector<std::vector<bool>> bufUnit = std::vector<std::vector<bool>>();
  swriter.time = 0;
  for (auto &packet : dataPackets) {
    if (bufUnit.size() == 0) {
      bufUnit.push_back(packet);
    } else {
      std::vector<bool> lastDataPacketPayload = std::vector<bool>(
          bufUnit[bufUnit.size() - 1].begin() + H_NBITS, bufUnit[bufUnit.size() - 1].end());
      int tLast = readPacketTS(lastDataPacketPayload);
      std::vector<bool> packetPayload = std::vector<bool>(packet.begin() + H_NBITS, packet.end());
      int packetTS = readPacketTS(packetPayload);
      if (tLast == packetTS) {
        bufUnit.push_back(packet);
      } else {
        std::vector<bool> temporalUnit = std::vector<bool>();
        writeMIHSUnit(MIHSUnitType::Temporal, bufUnit, temporalUnit, swriter);
        bitstream.push_back(temporalUnit);
        if (swriter.time != packetTS) {
          std::vector<std::vector<bool>> silentPackets{bufUnit[bufUnit.size() - 1], packet};
          std::vector<bool> silentUnit = std::vector<bool>();
          if (writeMIHSUnit(MIHSUnitType::Silent, silentPackets, silentUnit, swriter)) {
            bitstream.push_back(silentUnit);
          }
        }
        bufUnit.clear();
        bufUnit.push_back(packet);
      }
    }
  }
  if (!bufUnit.empty()) {
    std::vector<bool> temporalUnit = std::vector<bool>();
    writeMIHSUnit(MIHSUnitType::Temporal, bufUnit, temporalUnit, swriter);
    bitstream.push_back(temporalUnit);
    bufUnit.clear();
  }
  return true;
}

auto IOStream::readMIHSUnit(std::vector<bool> &mihsunit, StreamReader &sreader, CRC &crc) -> bool {
  int index = 0;
  int unitTypeInt = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_TYPE);
  MIHSUnitType unitType = static_cast<MIHSUnitType>(unitTypeInt);
  int syncInt = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_SYNC);
  bool sync = syncInt == 1;
  if (sreader.waitSync && sync) {
    sreader.waitSync = false;
  }
  sreader.packetDuration = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_DURATION);
  int unitLength = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_LENGTH) * BYTE_SIZE;

  std::vector<bool> packets = std::vector<bool>(mihsunit.begin() + index, mihsunit.end());
  while (index < unitLength) {
    if (!readNALu(packets, sreader, crc)) {
      return EXIT_FAILURE;
    }
    index += sreader.packetLength + H_NBITS;
    packets = std::vector<bool>(mihsunit.begin() + index, mihsunit.end());
  }
  sreader.time += (sreader.packetDuration * TIME_TO_MS) / sreader.timescale;
  return true;
}
auto IOStream::writeMIHSUnit(MIHSUnitType unitType, std::vector<std::vector<bool>> &listPackets,
                             std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  std::bitset<UNIT_TYPE> unitTypeBits(static_cast<int>(unitType));
  std::string unitTypeStr = unitTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(unitTypeStr, mihsunit);
  switch (unitType) {
  case MIHSUnitType::Initialization: {
    return writeMIHSUnitInitialization(listPackets, mihsunit, swriter);
  }
  case MIHSUnitType::Temporal: {
    return writeMIHSUnitTemporal(listPackets, mihsunit, swriter);
  }
  case MIHSUnitType::Spatial: {
    return writeMIHSUnitSpatial(listPackets, mihsunit, swriter);
  }
  case MIHSUnitType::Silent: {
    return writeMIHSUnitSilent(listPackets, mihsunit, swriter);
  }
  default:
    return false;
  }
}
auto IOStream::writeMIHSUnitInitialization(std::vector<std::vector<bool>> &listPackets,
                                           std::vector<bool> &mihsunit, StreamWriter &swriter)
    -> bool {
  std::bitset<UNIT_SYNC> syncBits(1);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  std::bitset<UNIT_DURATION> durationBits(0);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);

  int length = (listPackets.size() + 1) * static_cast<int>(H_NBITS / BYTE_SIZE);
  std::vector<bool> packetFusion = std::vector<bool>();
  std::vector<std::vector<bool>> timingPacket = std::vector<std::vector<bool>>();
  // Add a mandatory timing packet in mihs unit of type initialization
  writeNALu(NALuType::Timing, swriter, 0, timingPacket);
  listPackets.insert(listPackets.begin(), timingPacket.begin(), timingPacket.end());
  for (auto &packet : listPackets) {
    packetFusion.insert(packetFusion.end(), packet.begin(), packet.end());
    length += readPacketLength(packet);
  }
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  mihsunit.insert(mihsunit.end(), packetFusion.begin(), packetFusion.end());
  return true;
}
auto IOStream::writeMIHSUnitTemporal(std::vector<std::vector<bool>> &listPackets,
                                     std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  bool sync = true;
  int length = 0;
  int nbPacketData = 0;
  std::vector<bool> payload = std::vector<bool>();
  for (auto &packet : listPackets) {
    std::vector<bool> bufPacket = packet;
    NALuType mihsPacketType = readNALuType(packet);
    if (mihsPacketType == NALuType::Data) {
      bufPacket.erase(bufPacket.begin() + H_NBITS, bufPacket.begin() + H_NBITS + DB_DURATION);
      nbPacketData++;
      sync &= packet[H_NBITS];
      int packetStartTime = readPacketTS(std::vector<bool>(packet.begin() + H_NBITS, packet.end()));
      swriter.time = packetStartTime;
    }
    length += static_cast<int>(H_NBITS / BYTE_SIZE) + readPacketLength(bufPacket);
    payload.insert(payload.end(), bufPacket.begin(), bufPacket.end());
  }
  std::bitset<UNIT_SYNC> syncBits(sync);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  int duration = 0;
  if (nbPacketData > 0) {
    duration = swriter.packetDuration;
  }
  std::bitset<UNIT_DURATION> durationBits(duration);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  mihsunit.insert(mihsunit.end(), payload.begin(), payload.end());
  swriter.time += duration;
  return true;
}
auto IOStream::writeMIHSUnitSpatial(std::vector<std::vector<bool>> &listPackets,
                                    std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  int length = 0;
  std::vector<bool> payload = std::vector<bool>();
  for (auto &packet : listPackets) {
    std::vector<bool> bufPacket = packet;
    NALuType mihsPacketType = readNALuType(packet);
    if (mihsPacketType == NALuType::Data) {
      bufPacket.erase(bufPacket.begin() + H_NBITS, bufPacket.begin() + H_NBITS + DB_DURATION);
    }
    length += static_cast<int>(H_NBITS / BYTE_SIZE) + readPacketLength(bufPacket);
    payload.insert(payload.end(), bufPacket.begin(), bufPacket.end());
  }
  bool sync = true;
  std::bitset<UNIT_SYNC> syncBits(sync);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  int duration = 0;
  std::bitset<UNIT_DURATION> durationBits(duration);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  mihsunit.insert(mihsunit.end(), payload.begin(), payload.end());
  return true;
}
auto IOStream::writeMIHSUnitSilent(std::vector<std::vector<bool>> &listPackets,
                                   std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  if (listPackets.size() != 2) {
    return false;
  }
  bool sync = false;
  std::bitset<UNIT_SYNC> syncBits(sync);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  int start = swriter.time;
  int end = readPacketTS(std::vector<bool>(listPackets[1].begin() + H_NBITS, listPackets[1].end()));
  int duration = end - start;
  if (duration % swriter.packetDuration != 0) {
    duration = duration - (duration % swriter.packetDuration);
  }
  std::bitset<UNIT_DURATION> durationBits(duration);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
  std::bitset<UNIT_LENGTH> lengthBits(0);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  swriter.time += duration;
  return true;
}
auto IOStream::initializeStream() -> StreamReader {
  StreamReader sreader;
  sreader.haptic = types::Haptics();
  sreader.bandStreamsHaptic = std::vector<BandStream>();

  return sreader;
}

auto IOStream::writeNALu(NALuType naluType, StreamWriter &swriter, int level,
                         std::vector<std::vector<bool>> &bitstream) -> bool {

  checkHapticComponent(swriter.haptic);
  std::vector<bool> naluHeader = std::vector<bool>();
  switch (naluType) {
  case NALuType::Timing: {
    std::vector<bool> naluPayload = std::vector<bool>();
    writeTiming(swriter, naluPayload);
    writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.push_back(naluHeader);
    return true;
  }
  case NALuType::MetadataHaptics: {
    std::vector<bool> naluPayload = std::vector<bool>();
    writeMetadataHaptics(swriter.haptic, naluPayload);
    padToByteBoundary(naluPayload);
    writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.push_back(naluHeader);
    return true;
  }
  case NALuType::MetadataPerception: {
    std::vector<bool> naluPayload = std::vector<bool>();
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      swriter.perception = swriter.haptic.getPerceptionAt(i);
      writeMetadataPerception(swriter, naluPayload);
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
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      if (swriter.haptic.getPerceptionAt(i).getEffectLibrarySize() != 0) {
        writeLibrary(swriter.haptic.getPerceptionAt(i), naluPayload);
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
  case NALuType::MetadataTrack: {
    std::vector<bool> naluPayload = std::vector<bool>();
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      swriter.perception = swriter.haptic.getPerceptionAt(i);
      for (auto j = 0; j < static_cast<int>(swriter.haptic.getPerceptionAt(i).getTracksSize());
           j++) {
        swriter.track = swriter.perception.getTrackAt(j);
        writeMetadataTrack(swriter, naluPayload);
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
    return writeAllBands(swriter, naluType, level, naluHeader, bitstream);
  }
  case NALuType::Data: {
    std::vector<std::vector<bool>> naluPayload = std::vector<std::vector<bool>>();
    writeData(swriter, naluPayload);
    for (auto data : naluPayload) {
      padToByteBoundary(data);
      writeNALuHeader(naluType, level, static_cast<int>(data.size()), naluHeader);
      naluHeader.insert(naluHeader.end(), data.begin(), data.end());
      bitstream.push_back(naluHeader);
      naluHeader.clear();
    }
    return true;
  }
  case NALuType::CRC16:
  case NALuType::GlobalCRC16:
  case NALuType::CRC32:
  case NALuType::GlobalCRC32: {
    std::vector<bool> naluPayload = std::vector<bool>();
    int crcLevel = 0;
    if (naluType == NALuType::CRC32 || naluType == NALuType::GlobalCRC32) {
      crcLevel = 1;
    }
    writeCRC(bitstream, naluPayload, crcLevel);
    writeNALuHeader(naluType, level, static_cast<int>(naluPayload.size()), naluHeader);
    naluHeader.insert(naluHeader.end(), naluPayload.begin(), naluPayload.end());
    bitstream.clear();
    bitstream.push_back(naluHeader);
    return true;
  }
  default:
    return false;
  }
}

auto IOStream::writeAllBands(StreamWriter &swriter, NALuType naluType, int level,
                             std::vector<bool> &naluHeader,
                             std::vector<std::vector<bool>> &bitstream) -> bool {
  std::vector<bool> naluPayload = std::vector<bool>();
  int bandId = 0;
  for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
    swriter.perception = swriter.haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(swriter.perception.getTracksSize()); j++) {
      swriter.track = swriter.perception.getTrackAt(j);
      for (auto k = 0; k < static_cast<int>(swriter.track.getBandsSize()); k++) {
        swriter.bandStream.band = swriter.track.getBandAt(k);
        swriter.bandStream.id = bandId++;
        writeMetadataBand(swriter, naluPayload);
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
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    types::Perception &perception = haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(perception.getTracksSize()); j++) {
      types::Track &track = perception.getTrackAt(j);
      std::vector<types::Band> bands = std::vector<types::Band>();
      for (auto k = 0; k < static_cast<int>(track.getBandsSize()); k++) {
        types::Band &band = track.getBandAt(k);
        for (auto l = 0; l < static_cast<int>(band.getEffectsSize()); l++) {
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
  const std::string naluTypeStr = naluTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(naluTypeStr, bitstream);

  std::bitset<H_LEVEL> lvlBits(level);
  const std::string lvlStr = lvlBits.to_string();
  IOBinaryPrimitives::writeStrBits(lvlStr, bitstream);
  const int residual = H_NBITS - (H_NALU_TYPE + H_LEVEL + H_PAYLOAD_LENGTH);
  std::bitset<residual> resBits(0);
  const std::string resStr = resBits.to_string();
  IOBinaryPrimitives::writeStrBits(resStr, bitstream);
  int payloadSizeByte = payloadSize / BYTE_SIZE;
  if (naluType == NALuType::Data) {
    payloadSizeByte -= DB_DURATION / BYTE_SIZE;
  }
  std::bitset<H_PAYLOAD_LENGTH> payloadSizeBits(payloadSizeByte);
  const std::string payloadSizeStr = payloadSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(payloadSizeStr, bitstream);

  return true;
}
auto IOStream::readNALu(std::vector<bool> packet, StreamReader &sreader, CRC &crc) -> bool {
  NALuType naluType = readNALuType(packet);
  int index = H_NALU_TYPE;
  sreader.level = IOBinaryPrimitives::readUInt(packet, index, H_LEVEL);
  index = H_NBITS - H_PAYLOAD_LENGTH;
  sreader.packetLength = IOBinaryPrimitives::readUInt(packet, index, H_PAYLOAD_LENGTH) * BYTE_SIZE;
  std::vector<bool> payload = std::vector<bool>(packet.begin() + index, packet.end());
  switch (naluType) {
  case (NALuType::Timing): {
    sreader.time = IOBinaryPrimitives::readUInt(packet, index, TIMING_TIME);
    sreader.timescale = IOBinaryPrimitives::readUInt(packet, index, TIMING_TIMESCALE);
    sreader.time = (sreader.time * TIME_TO_MS) / sreader.timescale;
    return true;
  }
  case (NALuType::MetadataHaptics): {
    return readMetadataHaptics(sreader.haptic, payload);
  }
  case (NALuType::MetadataPerception): {
    if (!readMetadataPerception(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    if (perceIndex == -1) {
      sreader.haptic.addPerception(sreader.perception);
    } else {
      sreader.haptic.replacePerceptionAt(perceIndex, sreader.perception);
    }
    return true;
  }
  case (NALuType::EffectLibrary): {
    return readLibrary(sreader, payload);
  }
  case (NALuType::MetadataTrack): {
    if (!readMetadataTrack(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    int trackIndex = searchTrackInHaptic(sreader.haptic, sreader.track.getId());
    if (trackIndex == -1) {
      sreader.haptic.getPerceptionAt(perceIndex).addTrack(sreader.track);
    } else {
      sreader.haptic.getPerceptionAt(perceIndex).replaceTrackAt(trackIndex, sreader.track);
    }
    return true;
  }
  case (NALuType::MetadataBand): {
    if (!readMetadataBand(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    int trackIndex = searchTrackInHaptic(sreader.haptic, sreader.track.getId());
    int bandIndex = searchBandInHaptic(sreader, sreader.bandStream.id);
    if (bandIndex == -1) {
      sreader.haptic.getPerceptionAt(perceIndex)
          .getTrackAt(trackIndex)
          .addBand(sreader.bandStream.band);
      sreader.bandStream.index =
          static_cast<int>(
              sreader.haptic.getPerceptionAt(perceIndex).getTrackAt(trackIndex).getBandsSize()) -
          1;
      sreader.bandStreamsHaptic.push_back(sreader.bandStream);
    } else {
      sreader.haptic.getPerceptionAt(perceIndex)
          .getTrackAt(trackIndex)
          .replaceBandAt(bandIndex, sreader.bandStream.band);
    }
    return true;
  }
  case (NALuType::Data): {
    return readData(sreader, payload);
  }
  case (NALuType::CRC16):
  case (NALuType::CRC32):
  case (NALuType::GlobalCRC16):
  case (NALuType::GlobalCRC32): {
    return readCRC(payload, crc, naluType);
  }
  }
  return false;
}

auto IOStream::readPacketTS(std::vector<bool> bitstream) -> int {
  std::string tsBits;
  for (int i = 0; i < DB_DURATION; i++) {
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
  int typeInt = IOBinaryPrimitives::readUInt(packet, idx, H_NALU_TYPE);

  return static_cast<NALuType>(typeInt);
}
auto IOStream::readPacketLength(std::vector<bool> &bitstream) -> int {
  int beginIdx = haptics::io::H_NBITS - haptics::io::H_PAYLOAD_LENGTH;
  std::vector<bool> packetLength = std::vector<bool>(
      bitstream.begin() + beginIdx, bitstream.begin() + beginIdx + haptics::io::H_PAYLOAD_LENGTH);
  std::string packetLengthStr;
  for (auto c : packetLength) {
    if (c) {
      packetLengthStr += "1";
    } else {
      packetLengthStr += "0";
    }
  }
  return static_cast<int>(std::bitset<H_PAYLOAD_LENGTH>(packetLengthStr).to_ulong());
}
auto IOStream::writeTiming(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool {
  std::bitset<TIMING_TIME> timestampBits = swriter.time;
  std::string timestampStr = timestampBits.to_string();
  IOBinaryPrimitives::writeStrBits(timestampStr, bitstream);
  std::bitset<TIMING_TIMESCALE> timescaleBits = swriter.timescale;
  std::string timescaleStr = timescaleBits.to_string();
  IOBinaryPrimitives::writeStrBits(timescaleStr, bitstream);
  return true;
}

auto IOStream::writeMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDEXP_VERSION> versionCountBits(haptic.getVersion().size());
  const std::string versionCountStr = versionCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(versionCountStr, bitstream);
  for (auto c : haptic.getVersion()) {
    std::bitset<BYTE_SIZE> cBits(c);
    const std::string cStr = cBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }

  std::bitset<MDEXP_DATE> dateCountBits(haptic.getDate().size());
  std::string dateCountStr = dateCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(dateCountStr, bitstream);
  for (auto c : haptic.getDate()) {
    std::bitset<BYTE_SIZE> cBits(c);
    const std::string cStr = cBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }
  std::bitset<MDEXP_DESC_SIZE> descSizeBits(haptic.getDescription().length());
  const std::string descSizeStr = descSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(descSizeStr, bitstream);

  for (char &c : haptic.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    const std::string cStr = descBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }

  std::bitset<MDEXP_PERC_COUNT> perceCountBits(haptic.getPerceptionsSize());
  std::string perceCountStr = perceCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceCountStr, bitstream);

  std::bitset<MDEXP_AVATAR_COUNT> avatarCountBits(haptic.getAvatarsSize());
  std::string avatarCountStr = avatarCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(avatarCountStr, bitstream);

  for (auto i = 0; i < static_cast<int>(haptic.getAvatarsSize()); i++) {
    writeAvatar(haptic.getAvatarAt(i), bitstream);
  }
  return true;
}
auto IOStream::readMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool {
  int index = 0;

  int versionLength = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_VERSION);
  std::string version = IOBinaryPrimitives::readString(bitstream, index, versionLength);
  haptic.setVersion(version);

  int dateLength = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_DATE);
  std::string date = IOBinaryPrimitives::readString(bitstream, index, dateLength);
  haptic.setDate(date);

  int descLength = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_DESC_SIZE);
  std::string description = IOBinaryPrimitives::readString(bitstream, index, descLength);
  haptic.setDescription(description);

  // read number of perception, not used but could be for check
  IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_PERC_COUNT);

  int avatarCount = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_AVATAR_COUNT);
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
  std::string avatarIDStr = avatarIDBits.to_string();
  IOBinaryPrimitives::writeStrBits(avatarIDStr, bitstream);

  std::bitset<AVATAR_LOD> avatarLODBits(avatar.getLod());
  std::string avatarLODStr = avatarLODBits.to_string();
  IOBinaryPrimitives::writeStrBits(avatarLODStr, bitstream);

  std::bitset<AVATAR_TYPE> avatarTypeBits(static_cast<int>(avatar.getType()));
  std::string avatarTypeStr = avatarTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(avatarTypeStr, bitstream);

  if (static_cast<int>(avatar.getType()) == 0) {
    std::string mesh = avatar.getMesh().value();
    std::bitset<AVATAR_MESH_COUNT> avatarMeshCountBits(mesh.size());
    std::string avatarMeshCountStr = avatarMeshCountBits.to_string();
    IOBinaryPrimitives::writeStrBits(avatarMeshCountStr, bitstream);

    for (char &c : mesh) {
      std::bitset<BYTE_SIZE> descBits(c);
      std::string cStr = descBits.to_string();
      IOBinaryPrimitives::writeStrBits(cStr, bitstream);
    }
  }
  return true;
}
auto IOStream::readAvatar(std::vector<bool> &bitstream, types::Avatar &avatar, int &length)
    -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_ID);
  int lod = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_LOD);
  int type = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_TYPE);
  avatar.setId(id);
  avatar.setLod(lod);
  avatar.setType(static_cast<types::AvatarType>(type));
  if (type == 0) {
    int meshCount = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_MESH_COUNT);
    std::string mesh = IOBinaryPrimitives::readString(bitstream, idx, meshCount);
    avatar.setMesh(mesh);
  }
  length += idx;
  return true;
}

auto IOStream::writeMetadataPerception(StreamWriter &swriter, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<MDPERCE_ID> perceIDBits(swriter.perception.getId());
  std::string perceIdStr = perceIDBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceIdStr, bitstream);

  std::bitset<MDPERCE_DESC_SIZE> descSizeBits(swriter.perception.getDescription().size());
  std::string descSizeStr = descSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(descSizeStr, bitstream);

  for (char &c : swriter.perception.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    std::string cStr = descBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }

  std::bitset<MDPERCE_MODALITY> modalityBits(
      static_cast<int>(swriter.perception.getPerceptionModality()));
  std::string modalityStr = modalityBits.to_string();
  IOBinaryPrimitives::writeStrBits(modalityStr, bitstream);

  std::bitset<AVATAR_ID> avatarIDBits(swriter.perception.getAvatarId());
  std::string avatarIDStr = avatarIDBits.to_string();
  IOBinaryPrimitives::writeStrBits(avatarIDStr, bitstream);

  std::bitset<MDPERCE_LIBRARY_COUNT> fxLibCountBits(swriter.perception.getEffectLibrarySize());
  std::string fxLibCountStr = fxLibCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxLibCountStr, bitstream);

  std::bitset<MDPERCE_UNIT_EXP> unitExpBits(swriter.perception.getUnitExponentOrDefault());
  std::string unitExpStr = unitExpBits.to_string();
  IOBinaryPrimitives::writeStrBits(unitExpStr, bitstream);

  std::bitset<MDPERCE_PERCE_UNIT_EXP> perceUnitExpBits(
      swriter.perception.getPerceptionUnitExponentOrDefault());
  std::string perceUnitExpStr = perceUnitExpBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceUnitExpStr, bitstream);

  std::bitset<MDPERCE_REFDEVICE_COUNT> refDeviceCountBits(
      swriter.perception.getReferenceDevicesSize());
  std::string refDeviceCountStr = refDeviceCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(refDeviceCountStr, bitstream);

  for (auto i = 0; i < static_cast<int>(swriter.perception.getReferenceDevicesSize()); i++) {
    writeReferenceDevice(swriter.perception.getReferenceDeviceAt(i), bitstream);
  }

  std::bitset<MDPERCE_TRACK_COUNT> trackCountBits(swriter.perception.getTracksSize());
  std::string trackCountStr = trackCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(trackCountStr, bitstream);
  return true;
}
auto IOStream::readMetadataPerception(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  int idx = 0;

  int id = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  sreader.perception = types::Perception();

  sreader.perception.setId(id);

  int descLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_DESC_SIZE);

  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  sreader.perception.setDescription(desc);

  int modal = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_MODALITY);
  sreader.perception.setPerceptionModality(static_cast<types::PerceptionModality>(modal));

  int avatarId = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_ID);
  sreader.perception.setAvatarId(avatarId);

  // read effect library size, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_LIBRARY_COUNT);

  int unitExp = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_UNIT_EXP);
  sreader.perception.setUnitExponent(unitExp);

  int perceUnitExp = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_PERCE_UNIT_EXP);
  sreader.perception.setPerceptionUnitExponent(perceUnitExp);

  int refDevCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_REFDEVICE_COUNT);
  std::vector<types::ReferenceDevice> referenceDeviceList = std::vector<types::ReferenceDevice>();
  std::vector<bool> refDeviceListBits(bitstream.begin() + idx, bitstream.end());
  if (!readListObject(refDeviceListBits, refDevCount, referenceDeviceList, idx)) {
    return false;
  }
  for (auto refDev : referenceDeviceList) {
    sreader.perception.addReferenceDevice(refDev);
  }
  // read track count, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_TRACK_COUNT);

  return true;
}

auto IOStream::writeLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDPERCE_ID> perceIdBits(perception.getId());
  std::string perceIdStr = perceIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceIdStr, bitstream);
  std::bitset<MDPERCE_LIBRARY_COUNT> fxCountBits(perception.getEffectLibrarySize());
  std::string fxCountStr = fxCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxCountStr, bitstream);

  types::Effect libraryEffect;
  bool success = true;
  for (auto i = 0; i < static_cast<int>(perception.getEffectLibrarySize()); i++) {
    libraryEffect = perception.getBasisEffectAt(i);
    success &= writeLibraryEffect(libraryEffect, bitstream);
  }

  return success;
}
auto IOStream::readLibrary(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  int idx = 0;
  auto perceId = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  int perceIndex = searchPerceptionInHaptic(sreader.haptic, perceId);
  if (perceIndex == -1) {
    return false;
  }
  types::Perception perception = sreader.haptic.getPerceptionAt(perceIndex);
  auto effectCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_LIBRARY_COUNT);
  bool success = true;

  for (int i = 0; i < effectCount; i++) {
    types::Effect libraryEffect;
    success &= readLibraryEffect(libraryEffect, idx, bitstream);
    perception.addBasisEffect(libraryEffect);
  }
  sreader.haptic.replacePerceptionAt(perceIndex, perception);
  return success;
}
auto IOStream::readLibraryEffect(types::Effect &libraryEffect, int &idx,
                                 std::vector<bool> &bitstream) -> bool {
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_ID);
  libraryEffect.setId(id);

  int position = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_POSITION_STREAMING);
  libraryEffect.setPosition(position);

  float phase = IOBinaryPrimitives::readFloatNBits<EFFECT_PHASE>(bitstream, idx, 0, MAX_PHASE);
  libraryEffect.setPhase(phase);

  int baseSignal = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_BASE_SIGNAL);
  libraryEffect.setBaseSignal(static_cast<types::BaseSignal>(baseSignal));

  int effectType = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TYPE);
  libraryEffect.setEffectType(static_cast<types::EffectType>(effectType));

  int kfCount = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_KEYFRAME_COUNT);
  for (int i = 0; i < kfCount; i++) {
    auto mask = (uint8_t)IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_MASK);
    std::optional<int> position = std::nullopt;
    std::optional<float> amplitude = std::nullopt;
    std::optional<int> frequency = std::nullopt;
    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      position = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_POSITION);
    }

    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      amplitude = IOBinaryPrimitives::readFloatNBits<KEYFRAME_AMPLITUDE>(
          bitstream, idx, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    }

    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      frequency = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_FREQUENCY);
    }
    types::Keyframe myKeyframe(position, amplitude, frequency);
    libraryEffect.addKeyframe(myKeyframe);
  }
  bool success = true;

  auto timelineEffectCount = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TIMELINE_COUNT);
  for (int i = 0; i < timelineEffectCount; i++) {
    types::Effect timelineEffect;
    success &= readLibraryEffect(timelineEffect, idx, bitstream);
    libraryEffect.addTimelineEffect(timelineEffect);
  }

  return true;
}

auto IOStream::writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<EFFECT_ID> idBits(libraryEffect.getId());
  std::string idStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(idStr, bitstream);

  std::bitset<EFFECT_POSITION> posBits(libraryEffect.getPosition());
  std::string posStr = posBits.to_string();
  IOBinaryPrimitives::writeStrBits(posStr, bitstream);

  IOBinaryPrimitives::writeFloatNBits<uint32_t, EFFECT_PHASE>(libraryEffect.getPhase(), bitstream,
                                                              0, MAX_PHASE);

  std::bitset<EFFECT_BASE_SIGNAL> baseBits(static_cast<int>(libraryEffect.getBaseSignal()));
  std::string baseStr = baseBits.to_string();
  IOBinaryPrimitives::writeStrBits(baseStr, bitstream);

  std::bitset<EFFECT_TYPE> typeBits(static_cast<int>(libraryEffect.getEffectType()));
  std::string typeStr = typeBits.to_string();
  IOBinaryPrimitives::writeStrBits(typeStr, bitstream);

  auto kfCount = libraryEffect.getKeyframesSize();
  std::bitset<EFFECT_KEYFRAME_COUNT> kfCountBits(kfCount);
  std::string kfCountStr = kfCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(kfCountStr, bitstream);

  types::Keyframe keyframe;
  for (size_t i = 0; i < kfCount; i++) {
    keyframe = libraryEffect.getKeyframeAt(static_cast<int>(i));
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
    std::bitset<KEYFRAME_MASK> maskBits(mask);
    std::string maskStr = maskBits.to_string();
    IOBinaryPrimitives::writeStrBits(maskStr, bitstream);

    if ((mask & (uint8_t)KeyframeMask::RELATIVE_POSITION) != 0) {
      std::bitset<KEYFRAME_POSITION> posBits(keyframe.getRelativePosition().value());
      std::string posStr = posBits.to_string();
      IOBinaryPrimitives::writeStrBits(posStr, bitstream);
    }
    if ((mask & (uint8_t)KeyframeMask::AMPLITUDE_MODULATION) != 0) {
      IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
          keyframe.getAmplitudeModulation().value(), bitstream, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    }
    if ((mask & (uint8_t)KeyframeMask::FREQUENCY_MODULATION) != 0) {
      std::bitset<KEYFRAME_POSITION> freqBits(keyframe.getFrequencyModulation().value());
      std::string freqStr = freqBits.to_string();
      IOBinaryPrimitives::writeStrBits(freqStr, bitstream);
    }
  }
  auto timelineEffectCount = static_cast<uint16_t>(libraryEffect.getTimelineSize());
  std::bitset<EFFECT_TIMELINE_COUNT> timelineSizeBits(timelineEffectCount);
  std::string timelineSizeStr = timelineSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(timelineSizeStr, bitstream);

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
  std::string idStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(idStr, bitstream);

  std::bitset<REFDEV_NAME_LENGTH> nameLengthBits(refDevice.getName().size());
  std::string nameLengthStr = nameLengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(nameLengthStr, bitstream);

  for (char &c : refDevice.getName()) {
    std::bitset<BYTE_SIZE> descBits(c);
    std::string cStr = descBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }

  std::bitset<REFDEV_BODY_PART_MASK> bodyPartMaskBits(refDevice.getBodyPartMask().value_or(0));
  std::string bodypartMaskStr = bodyPartMaskBits.to_string();
  IOBinaryPrimitives::writeStrBits(bodypartMaskStr, bitstream);

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
    std::string typeStr = typeBits.to_string();
    IOBinaryPrimitives::writeStrBits(typeStr, bitstream);
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
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_ID);
  refDevice.setId(id);

  int nameLength = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_NAME_LENGTH);
  std::string name = IOBinaryPrimitives::readString(bitstream, idx, nameLength);
  refDevice.setName(name);
  int bodyPartMask = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_BODY_PART_MASK);
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
    int type = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_TYPE);
    refDevice.setType(static_cast<types::ActuatorType>(type));
  }
  length += idx;
  return true;
}

auto IOStream::writeMetadataTrack(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDTRACK_ID> idBits(swriter.track.getId());
  std::string valueStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::bitset<MDPERCE_ID> perceidBits(swriter.perception.getId());
  valueStr = perceidBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::bitset<MDPERCE_DESC_SIZE> descSizeBits(swriter.track.getDescription().size());
  valueStr = descSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  for (char &c : swriter.track.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    valueStr = descBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<REFDEV_ID> refDevID(swriter.track.getReferenceDeviceId().value_or(-1));
  valueStr = refDevID.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_GAIN>(swriter.track.getGain(), bufBits,
                                                              -MAX_FLOAT, MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDTRACK_MIXING_WEIGHT>(
      swriter.track.getMixingWeight(), bufBits, 0, MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  std::bitset<MDTRACK_BODY_PART_MASK> bodyPartMaskBits(swriter.track.getBodyPartMask());
  valueStr = bodyPartMaskBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  int freqSampling = static_cast<int>(swriter.track.getFrequencySampling().value_or(0));
  std::bitset<MDTRACK_FREQ_SAMPLING> maxFreqBits(freqSampling);
  valueStr = maxFreqBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  if (freqSampling > 0) {
    std::bitset<MDTRACK_SAMPLE_COUNT> sampleCountBits(swriter.track.getSampleCount().value_or(0));
    valueStr = sampleCountBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  if (swriter.track.getDirection().has_value()) {
    types::Direction dir = swriter.track.getDirection().value();
    std::bitset<1> flagBit(1);
    valueStr = flagBit.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
    std::bitset<MDTRACK_DIRECTION_AXIS> axisValue(dir.X);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
    axisValue = std::bitset<MDTRACK_DIRECTION_AXIS>(dir.Y);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
    axisValue = std::bitset<MDTRACK_DIRECTION_AXIS>(dir.Z);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  } else {
    std::bitset<1> flagBit(0);
    valueStr = flagBit.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<MDTRACK_VERT_COUNT> vertCountBits(swriter.track.getVerticesSize());
  valueStr = vertCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  for (auto i = 0; i < static_cast<int>(swriter.track.getVerticesSize()); i++) {
    std::bitset<MDTRACK_VERT> vertBits(swriter.track.getVertexAt(i));
    valueStr = vertBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<MDTRACK_BANDS_COUNT> bandsCountBits(swriter.track.getBandsSize());
  valueStr = bandsCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  return true;
}
auto IOStream::readMetadataTrack(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {

  sreader.track = types::Track();
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_ID);
  sreader.track.setId(id);

  int perceId = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  int perceIndex = searchPerceptionInHaptic(sreader.haptic, perceId);
  if (perceIndex == -1) {
    return false;
  }
  sreader.perception = sreader.haptic.getPerceptionAt(perceIndex);

  int descLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_DESC_LENGTH);
  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  sreader.track.setDescription(desc);

  int deviceId = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_ID);
  if (deviceId < REFDEV_MAX_ID) {
    sreader.track.setReferenceDeviceId(deviceId);
  }
  float gain =
      IOBinaryPrimitives::readFloatNBits<MDTRACK_GAIN>(bitstream, idx, -MAX_FLOAT, MAX_FLOAT);
  sreader.track.setGain(gain);

  float mixingWeight =
      IOBinaryPrimitives::readFloatNBits<MDTRACK_MIXING_WEIGHT>(bitstream, idx, 0, MAX_FLOAT);
  sreader.track.setMixingWeight(mixingWeight);

  uint32_t bodyPartMask = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_BODY_PART_MASK);
  sreader.track.setBodyPartMask(bodyPartMask);

  uint32_t frequencySampling = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_FREQ_SAMPLING);
  if (frequencySampling > 0) {
    sreader.track.setFrequencySampling(frequencySampling);
  }
  if (frequencySampling > 0) {
    uint32_t sampleCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_SAMPLE_COUNT);
    sreader.track.setSampleCount(sampleCount);
  }

  bool directionMask =
      static_cast<bool>(IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_DIRECTION_MASK));
  if (directionMask) {
    types::Direction direction = types::Direction();
    direction.X =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    direction.Y =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    direction.Z =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_DIRECTION_AXIS));
    sreader.track.setDirection(direction);
  }

  int verticesCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_VERT_COUNT);
  for (int i = 0; i < verticesCount; i++) {
    int vertex = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_VERT);
    sreader.track.addVertex(vertex);
  }

  // read band count, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_BANDS_COUNT);

  return true;
}

auto IOStream::writeMetadataBand(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDBAND_ID> idBits(swriter.bandStream.id);
  std::string idStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(idStr, bitstream);
  std::bitset<MDPERCE_ID> perceidBits(swriter.perception.getId());
  std::string perceidStr = perceidBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceidStr, bitstream);
  std::bitset<MDTRACK_ID> trackidBits(swriter.track.getId());
  std::string trackidStr = trackidBits.to_string();
  IOBinaryPrimitives::writeStrBits(trackidStr, bitstream);

  std::bitset<MDBAND_BAND_TYPE> bandTypeBits(
      static_cast<int>(swriter.bandStream.band.getBandType()));
  std::string bandTypeStr = bandTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(bandTypeStr, bitstream);
  if (swriter.bandStream.band.getBandType() == types::BandType::Curve) {
    std::bitset<MDBAND_CURVE_TYPE> curveTypeBits(
        static_cast<int>(swriter.bandStream.band.getCurveType()));
    std::string curveTypeStr = curveTypeBits.to_string();
    IOBinaryPrimitives::writeStrBits(curveTypeStr, bitstream);
  } else if (swriter.bandStream.band.getBandType() == types::BandType::WaveletWave) {
    std::bitset<MDBAND_WIN_LEN> winLengthBits((swriter.bandStream.band.getWindowLength()));
    std::string winLengthStr = winLengthBits.to_string();
    IOBinaryPrimitives::writeStrBits(winLengthStr, bitstream);
  }
  std::bitset<MDBAND_LOW_FREQ> lowFreqBits((swriter.bandStream.band.getLowerFrequencyLimit()));
  std::string lowFreqStr = lowFreqBits.to_string();
  IOBinaryPrimitives::writeStrBits(lowFreqStr, bitstream);

  std::bitset<MDBAND_UP_FREQ> upFreqBits((swriter.bandStream.band.getUpperFrequencyLimit()));
  std::string upFreqStr = upFreqBits.to_string();
  IOBinaryPrimitives::writeStrBits(upFreqStr, bitstream);

  std::bitset<MDBAND_EFFECT_COUNT> effectCountBits(swriter.bandStream.band.getEffectsSize());
  std::string effectCountStr = effectCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(effectCountStr, bitstream);

  return true;
}
auto IOStream::readMetadataBand(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  sreader.bandStream = BandStream();
  int idx = 0;
  sreader.bandStream.id = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_ID);
  int perceId = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  int perceIndex = searchPerceptionInHaptic(sreader.haptic, perceId);
  if (perceIndex == -1) {
    return false;
  }
  sreader.perception = sreader.haptic.getPerceptionAt(perceIndex);

  int trackId = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_ID);
  int trackIndex = searchTrackInHaptic(sreader.haptic, trackId);
  if (trackIndex == -1) {
    return false;
  }
  sreader.track = sreader.perception.getTrackAt(trackIndex);

  int bandType = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_BAND_TYPE);
  sreader.bandStream.band.setBandType(static_cast<types::BandType>(bandType));
  if (sreader.bandStream.band.getBandType() == types::BandType::Curve) {
    int curveType = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_CURVE_TYPE);
    sreader.bandStream.band.setCurveType(static_cast<types::CurveType>(curveType));
  } else if (sreader.bandStream.band.getBandType() == types::BandType::WaveletWave) {
    int windowLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_WIN_LEN);
    sreader.bandStream.band.setWindowLength(windowLength);
  }
  int lowFreq = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_LOW_FREQ);
  sreader.bandStream.band.setLowerFrequencyLimit(lowFreq);
  int upFreq = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_UP_FREQ);
  sreader.bandStream.band.setUpperFrequencyLimit(upFreq);

  // read effects count, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_EFFECT_COUNT);
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
          if (k == static_cast<int>(output.size())) {
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

auto IOStream::linearizeTimeline(types::Band &band) -> void {
  std::vector<types::Effect> effects = std::vector<types::Effect>();
  for (auto i = 0; i < static_cast<int>(band.getEffectsSize()); i++) {
    auto effect = band.getEffectAt(i);
    if (effect.getEffectType() == types::EffectType::Timeline) {
      linearizeTimelineEffect(effect, effects);
    } else {
      effects.push_back(effect);
    }
  }
  for (int i = static_cast<int>(band.getEffectsSize()) - 1; i > -1; i--) {
    band.removeEffectAt(i);
  }
  for (auto &e : effects) {
    band.addEffect(e);
  }
}
auto IOStream::linearizeTimelineEffect(types::Effect &effect, std::vector<types::Effect> &effects)
    -> void {
  for (int j = 0; j < static_cast<int>(effect.getTimelineSize()); j++) {
    auto effectTimeline = effect.getTimelineEffectAt(j);
    if (effectTimeline.getEffectType() == types::EffectType::Timeline) {
      linearizeTimelineEffect(effectTimeline, effects);
    } else {
      effects.push_back(effectTimeline);
    }
  }
}

auto IOStream::packetizeBand(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstreams)
    -> bool {
  // Exit this function when all the band is packetised
  std::vector<std::vector<bool>> bufPacketBitstream = std::vector<std::vector<bool>>();
  std::vector<bool> packetBits = std::vector<bool>();
  swriter.effects = std::vector<types::Effect>();
  swriter.keyframesCount = std::vector<int>();
  swriter.time = 0;
  if (swriter.bandStream.band.getBandType() == types::BandType::WaveletWave) {
    createWaveletPayload(swriter, bufPacketBitstream);
    for (auto &bufpacket : bufPacketBitstream) {
      packetBits = writeEffectHeader(swriter);
      packetBits = writeWaveletPayloadPacket(bufpacket, packetBits, swriter.effectsId);
      bitstreams.push_back(packetBits);
      swriter.time += swriter.bandStream.band.getWindowLength();
    }
  } else {
    while (!createPayloadPacket(swriter, bufPacketBitstream)) {
      // write packet as vector<bool>
      if (!bufPacketBitstream.empty()) {
        packetBits = writeEffectHeader(swriter);
        packetBits = writePayloadPacket(swriter, bufPacketBitstream, packetBits);
        bitstreams.push_back(packetBits);
      }
      swriter.effects.clear();
      bufPacketBitstream.clear();
      packetBits.clear();
      swriter.keyframesCount.clear();
      swriter.time += swriter.packetDuration;
    }
    if (!bufPacketBitstream.empty()) {
      packetBits = writeEffectHeader(swriter);
      packetBits = writePayloadPacket(swriter, bufPacketBitstream, packetBits);
      bitstreams.push_back(packetBits);
    }
  }
  return true;
}

auto IOStream::createWaveletPayload(StreamWriter &swriter,
                                    std::vector<std::vector<bool>> &bitstream) -> bool {
  int nbWaveBlock = swriter.packetDuration / swriter.bandStream.band.getWindowLength();
  for (auto i = 0; i < static_cast<int>(swriter.bandStream.band.getEffectsSize());
       i += nbWaveBlock) {
    std::vector<bool> bufbitstream = std::vector<bool>();
    types::Effect bufEffect = swriter.bandStream.band.getEffectAt(i);
    if (bufEffect.getKeyframesSize() > 1) {
      IOBinaryBands::writeWaveletEffect(bufEffect, bufbitstream);
      bitstream.push_back(bufbitstream);
    }
    int overflow = static_cast<int>(swriter.bandStream.band.getEffectsSize()) - (i + nbWaveBlock);
    if (overflow < 0) {
      nbWaveBlock = static_cast<int>(swriter.bandStream.band.getEffectsSize()) - i;
    }
  }
  return true;
}
auto IOStream::createPayloadPacket(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream)
    -> bool {

  // Exit this function only when 1 packet is full or last keyframes of the band is reached
  for (auto i = 0; i < static_cast<int>(swriter.bandStream.band.getEffectsSize()); i++) {
    bool endEffect = false;
    types::Effect &effect = swriter.bandStream.band.getEffectAt(i);
    if (effect.getId() == -1) {
      int nextId = 0;
      if (!swriter.effectsId.empty()) {
        nextId = *max_element(swriter.effectsId.begin(), swriter.effectsId.end()) + 1;
      }
      effect.setId(nextId);
      swriter.effectsId.push_back(nextId);
    }

    std::vector<bool> bufEffect = std::vector<bool>();
    if (effect.getEffectType() == types::EffectType::Basis) {
      int bufKFCount = 0;
      bool isRAU = true;
      bool endPacket = false;
      if (writeEffectBasis(effect, swriter, bufKFCount, isRAU, bufEffect)) {
        endEffect = true;
      } else {
        endPacket = true;
      }
      if (bufKFCount > 0) {
        swriter.keyframesCount.push_back(bufKFCount);
        bitstream.push_back(bufEffect);
        swriter.effects.push_back(effect);
      }
      swriter.auType = isRAU ? AUType::RAU : AUType::DAU;
      if (endEffect && i == static_cast<int>(swriter.bandStream.band.getEffectsSize()) - 1) {
        return true;
      }
      if (endPacket) {
        return false;
      }
    } else if (effect.getEffectType() == types::EffectType::Reference) {
      if (effect.getPosition() >= swriter.time &&
          effect.getPosition() < swriter.time + swriter.packetDuration) {
        bitstream.push_back(bufEffect);
        swriter.effects.push_back(effect);
        swriter.auType = AUType::RAU;
        swriter.keyframesCount.push_back(0);
      } else if (effect.getPosition() > swriter.time + swriter.packetDuration) {
        return false;
      }
      if (i == static_cast<int>(swriter.bandStream.band.getEffectsSize()) - 1) {
        return true;
      }
    }
  }
  return true;
}

auto IOStream::writeEffectHeader(StreamWriter &swriter) -> std::vector<bool> {
  std::vector<bool> packetBits = std::vector<bool>();
  std::bitset<DB_DURATION> tsBits(swriter.time);
  std::string tsStr = tsBits.to_string();
  IOBinaryPrimitives::writeStrBits(tsStr, packetBits);
  int autype = static_cast<int>(swriter.auType);
  std::bitset<DB_AU_TYPE> auBits(autype);
  std::string auStr = auBits.to_string();
  IOBinaryPrimitives::writeStrBits(auStr, packetBits);
  std::bitset<MDPERCE_ID> perceIdBits(swriter.perception.getId());
  std::string perceIdStr = perceIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceIdStr, packetBits);
  std::bitset<MDTRACK_ID> trackIdBits(swriter.track.getId());
  std::string trackIdStr = trackIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(trackIdStr, packetBits);

  std::bitset<MDBAND_ID> bandIdBits(swriter.bandStream.id);
  std::string bandIdStr = bandIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(bandIdStr, packetBits);

  return packetBits;
}
auto IOStream::writeWaveletPayloadPacket(std::vector<bool> bufPacketBitstream,
                                         std::vector<bool> &packetBits, std::vector<int> &effectsId)
    -> std::vector<bool> {
  std::bitset<DB_EFFECT_COUNT> fxCountBits(1);
  std::string fxCountStr = fxCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxCountStr, packetBits);

  int id = getNextEffectId(effectsId);
  std::bitset<EFFECT_ID> effectIDBits(static_cast<int>(id));
  std::string effectIDStr = effectIDBits.to_string();
  IOBinaryPrimitives::writeStrBits(effectIDStr, packetBits);

  std::bitset<EFFECT_TYPE> effectTypeBits(static_cast<int>(types::EffectType::Basis));
  std::string effectTypeStr = effectTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(effectTypeStr, packetBits);

  packetBits.insert(packetBits.end(), bufPacketBitstream.begin(), bufPacketBitstream.end());

  return packetBits;
}
auto IOStream::writePayloadPacket(StreamWriter &swriter,
                                  std::vector<std::vector<bool>> bufPacketBitstream,
                                  std::vector<bool> &packetBits) -> std::vector<bool> {
  std::bitset<DB_EFFECT_COUNT> fxCountBits(swriter.effects.size());
  std::string fxCountStr = fxCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxCountStr, packetBits);

  for (auto l = 0; l < static_cast<int>(swriter.effects.size()); l++) {
    std::bitset<EFFECT_ID> effectIDBits(static_cast<int>(swriter.effects[l].getId()));
    std::string effectIDStr = effectIDBits.to_string();
    IOBinaryPrimitives::writeStrBits(effectIDStr, packetBits);

    std::bitset<EFFECT_TYPE> effectTypeBits(static_cast<int>(swriter.effects[l].getEffectType()));
    std::string effectTypeStr = effectTypeBits.to_string();
    IOBinaryPrimitives::writeStrBits(effectTypeStr, packetBits);

    int effectPos = static_cast<int>(swriter.effects[l].getPosition()) - swriter.time;
    // const int FX_POSITION_SIGNED = FX_POSITION -1;
    bool carry = true;
    std::bitset<EFFECT_POSITION> effectPosBits(effectPos);
    std::string effectPosStr = effectPosBits.to_string();
    IOBinaryPrimitives::writeStrBits(effectPosStr, packetBits);

    if (swriter.effects[l].getEffectType() == types::EffectType::Basis &&
        swriter.bandStream.band.getBandType() != types::BandType::WaveletWave) {
      std::bitset<EFFECT_KEYFRAME_COUNT> kfCountBits(swriter.keyframesCount[l]);
      std::string kfCountStr = kfCountBits.to_string();
      IOBinaryPrimitives::writeStrBits(kfCountStr, packetBits);
      if (swriter.bandStream.band.getBandType() == types::BandType::VectorialWave) {
        IOBinaryPrimitives::writeFloatNBits<uint16_t, EFFECT_PHASE>(swriter.effects[l].getPhase(),
                                                                    packetBits, 0, MAX_PHASE);
        std::bitset<EFFECT_BASE_SIGNAL> fxBaseBits(
            static_cast<int>(swriter.effects[l].getBaseSignal()));
        std::string fxBaseStr = fxBaseBits.to_string();
        IOBinaryPrimitives::writeStrBits(fxBaseStr, packetBits);
      }
    }

    packetBits.insert(packetBits.end(), bufPacketBitstream[l].begin(), bufPacketBitstream[l].end());
  }
  return packetBits;
}

auto IOStream::writeSpatialData(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream)
    -> bool {
  swriter.auType = AUType::RAU;
  swriter.time = 0;

  std::vector<bool> bandBitstream = writeEffectHeader(swriter);
  std::bitset<DB_EFFECT_COUNT> fxCountBits(swriter.bandStream.band.getEffectsSize());
  std::string fxCountStr = fxCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxCountStr, bandBitstream);

  IOBinaryBands::writeBandBody(swriter.bandStream.band, bandBitstream);
  bitstream.push_back(bandBitstream);

  return true;
}
auto IOStream::writeData(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream) -> bool {

  swriter.effectsId = getEffectsId(swriter.haptic);
  std::vector<std::vector<std::vector<bool>>> expBitstream =
      std::vector<std::vector<std::vector<bool>>>();
  std::vector<std::vector<bool>> bandBitstream = std::vector<std::vector<bool>>();
  int bandId = 0;
  for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
    swriter.perception = swriter.haptic.getPerceptionAt(i);
    auto modality = swriter.perception.getPerceptionModality();
    bool spatial = false;
    if (modality == types::PerceptionModality::VibrotactileTexture ||
        modality == types::PerceptionModality::Stiffness ||
        modality == types::PerceptionModality::Friction) {
      spatial = true;
    }
    for (auto j = 0; j < static_cast<int>(swriter.perception.getTracksSize()); j++) {
      swriter.track = swriter.perception.getTrackAt(j);
      for (auto k = 0; k < static_cast<int>(swriter.track.getBandsSize()); k++) {
        BandStream bandStream;
        bandStream.id = bandId++;
        bandStream.band = swriter.track.getBandAt(k);
        swriter.bandStream = bandStream;
        if (spatial) {
          writeSpatialData(swriter, bandBitstream);
          expBitstream.push_back(bandBitstream);
        } else {
          linearizeTimeline(swriter.bandStream.band);
          packetizeBand(swriter, bandBitstream);
          expBitstream.push_back(bandBitstream);
          bandBitstream.clear();
        }
      }
    }
  }
  sortPacket(expBitstream, bitstream);
  return true;
}

auto IOStream::readData(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  int idx = 0;
  sreader.auType = static_cast<AUType>(IOBinaryPrimitives::readUInt(bitstream, idx, DB_AU_TYPE));

  if (sreader.waitSync && sreader.auType == AUType::DAU) {
    return false;
  }
  if (sreader.waitSync) {
    sreader.waitSync = false;
  }
  // int duration = IOBinaryPrimitives::readInt(bitstream, idx, DB_DURATION);

  int perceptionId = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  auto perceptionIndex = searchPerceptionInHaptic(sreader.haptic, perceptionId);
  if (perceptionIndex == -1) {
    return false;
  }
  sreader.perception = sreader.haptic.getPerceptionAt(perceptionIndex);

  int trackId = IOBinaryPrimitives::readUInt(bitstream, idx, MDTRACK_ID);
  auto trackIndex = searchTrackInHaptic(sreader.haptic, trackId);
  if (trackIndex == -1) {
    return false;
  }
  sreader.track = sreader.perception.getTrackAt(trackIndex);
  sreader.bandStream.index = -1;
  sreader.bandStream.id = -1;
  sreader.bandStream.id = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_ID);
  sreader.bandStream.index = searchBandInHaptic(sreader, sreader.bandStream.id);
  if (sreader.bandStream.index == -1) {
    return false;
  }
  sreader.bandStream.band = sreader.track.getBandAt(sreader.bandStream.index);

  int fxCount = IOBinaryPrimitives::readUInt(bitstream, idx, DB_EFFECT_COUNT);
  if (fxCount > 0) {
    std::vector<types::Effect> effects;
    std::vector<bool> effectsBitsList(bitstream.begin() + idx, bitstream.end());
    if (sreader.bandStream.band.getBandType() != types::BandType::WaveletWave) {
      if (!readListObject(effectsBitsList, fxCount, sreader.bandStream.band, effects, idx)) {
        return false;
      }
      addTimestampEffect(effects, sreader.time);
    } else {
      types::Effect effect;
      readWaveletEffect(effectsBitsList, sreader.bandStream.band, effect, idx);
      effects.push_back(effect);
    }
    return addEffectToHaptic(sreader.haptic, perceptionIndex, trackIndex, sreader.bandStream.index,
                             effects);
  } else {
    return true;
  }
}

auto IOStream::writeCRC(std::vector<std::vector<bool>> &bitstream, std::vector<bool> &packetCRC,
                        int crcLevel) -> bool {
  std::vector<bool> polynomial = std::vector<bool>();
  if (crcLevel == 0) {
    std::bitset<CRC16_NB_BITS> polynomialBits(CRC16_POLYNOMIAL);
    std::string polynomialStr = polynomialBits.to_string();
    IOBinaryPrimitives::writeStrBits(polynomialStr, polynomial);
  } else if (crcLevel == 1) {
    std::bitset<CRC32_NB_BITS> polynomialBits(CRC32_POLYNOMIAL);
    std::string polynomialStr = polynomialBits.to_string();
    IOBinaryPrimitives::writeStrBits(polynomialStr, polynomial);
  } else {
    return false;
  }
  if (polynomial.empty()) {
    return false;
  }

  std::vector<bool> quotient = std::vector<bool>();
  int nbPackets = 0;
  if (bitstream.empty()) {
    quotient = bitstream[0];
    nbPackets++;
  } else if (bitstream.size() > 1) {
    for (auto &packet : bitstream) {
      quotient.insert(quotient.end(), packet.begin(), packet.end());
      nbPackets++;
    }
  } else {
    return false;
  }
  if (quotient.empty()) {
    return false;
  }
  if (nbPackets > 1) {
    std::bitset<GCRC_NB_PACKET> nbPacketsBits(nbPackets);
    std::string nbPacketsStr = nbPacketsBits.to_string();
    IOBinaryPrimitives::writeStrBits(nbPacketsBits.to_string(), packetCRC);
  }
  computeCRC(quotient, polynomial);
  packetCRC.insert(packetCRC.end(), quotient.begin(), quotient.end());
  return true;
}
auto IOStream::readCRC(std::vector<bool> &bitstream, CRC &crc, NALuType naluType) -> bool {
  int idx = 0;
  if (naluType == NALuType::CRC16) {
    crc.nbPackets = 1;
    crc.value16 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC16_NB_BITS);
    crc.value32 = 0;
    return true;
  }
  if (naluType == NALuType::CRC32) {
    crc.nbPackets = 1;
    crc.value32 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC32_NB_BITS);
    crc.value16 = 0;
    return true;
  }
  if (naluType == NALuType::GlobalCRC16) {
    crc.nbPackets = IOBinaryPrimitives::readUInt(bitstream, idx, GCRC_NB_PACKET);
    crc.value16 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC16_NB_BITS);
    crc.value32 = 0;
    return true;
  }
  if (naluType == NALuType::GlobalCRC32) {
    crc.nbPackets = IOBinaryPrimitives::readUInt(bitstream, idx, GCRC_NB_PACKET);
    crc.value32 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC32_NB_BITS);
    crc.value16 = 0;
    return true;
  }
  return false;
}

auto IOStream::checkCRC(std::vector<std::vector<bool>> &bitstream, CRC &crc) -> bool {
  std::vector<bool> protectedPackets = std::vector<bool>();
  if (bitstream.size() < crc.nbPackets) {
    return false;
  }
  int index = 0;
  while (crc.nbPackets-- > 0) {
    protectedPackets.insert(protectedPackets.begin(), bitstream[index].begin(),
                            bitstream[index].end());
    index++;
  }
  bool res = false;
  if (crc.value16 > 0) {
    std::bitset<CRC16_NB_BITS> polynomialBits(crc.polynomial16);
    std::vector<bool> polynomial = std::vector<bool>();
    std::string polynomialStr = polynomialBits.to_string();
    IOBinaryPrimitives::writeStrBits(polynomialStr, polynomial);
    computeCRC(protectedPackets, polynomial);
    index = 0;
    int protectedPacketInt = IOBinaryPrimitives::readUInt(protectedPackets, index, CRC16_NB_BITS);
    if (static_cast<int>(crc.value16) == protectedPacketInt) {
      crc.value16 = 0;
      res = true;
    } else {
      crc.value16 = 0;
      res = false;
    }
  } else if (crc.value32 > 0) {
    std::bitset<CRC32_NB_BITS> polynomialBits(crc.polynomial32);
    std::vector<bool> polynomial = std::vector<bool>();
    std::string polynomialStr = polynomialBits.to_string();
    IOBinaryPrimitives::writeStrBits(polynomialStr, polynomial);
    computeCRC(protectedPackets, polynomial);
    index = 0;
    int protectedPacketInt = IOBinaryPrimitives::readUInt(protectedPackets, index, CRC32_NB_BITS);
    if (static_cast<int>(crc.value32) == protectedPacketInt) {
      crc.value32 = 0;
      res = true;
    } else {
      crc.value32 = 0;
      res = false;
    }
  }
  return res;
}

auto IOStream::computeCRC(std::vector<bool> &bitstream, std::vector<bool> &polynomial) -> bool {
  for (size_t i = 0; i < polynomial.size(); i++) {
    bitstream.push_back(false);
  }
  while (bitstream.size() > polynomial.size()) {
    bool msb = bitstream[0];
    bitstream = std::vector<bool>(bitstream.begin() + 1, bitstream.end());
    if (msb) {
      for (size_t i = 0; i < polynomial.size(); i++) {
        bitstream[i] = bitstream[i] != polynomial[i];
      }
    }
  }
  return true;
}

auto IOStream::readWaveletEffect(std::vector<bool> &bitstream, types::Band &band,
                                 types::Effect &effect, int &length) -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_ID);
  effect.setId(id);

  types::EffectType effectType =
      static_cast<types::EffectType>(IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TYPE));
  effect.setEffectType(effectType);

  int effectPos = static_cast<int>(band.getWindowLength() * band.getEffectsSize());
  effect.setPosition(effectPos);

  IOBinaryBands::readWaveletEffect(effect, band, bitstream, idx);
  length += idx;
  return true;
}
auto IOStream::readEffect(std::vector<bool> &bitstream, types::Effect &effect, types::Band &band,
                          int &length) -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_ID);
  effect.setId(id);

  types::EffectType effectType =
      static_cast<types::EffectType>(IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TYPE));
  effect.setEffectType(effectType);

  int effectPos = IOBinaryPrimitives::readInt(bitstream, idx, EFFECT_POSITION);
  effect.setPosition(effectPos);

  if (effectType == types::EffectType::Basis &&
      band.getBandType() != types::BandType::WaveletWave) {
    if (!readEffectBasis(bitstream, effect, band.getBandType(), idx)) {
      return false;
    }
  } else if (effectType == types::EffectType::Basis &&
             band.getBandType() == types::BandType::WaveletWave) {
    IOBinaryBands::readWaveletEffect(effect, band, bitstream, idx);
  }
  length += idx;
  return true;
}

auto IOStream::writeEffectBasis(types::Effect effect, StreamWriter &swriter, int &kfCount,
                                bool &rau, std::vector<bool> &bitstream) -> bool {
  bool firstKf = true;
  int tsFX = effect.getPosition();
  for (auto j = 0; j < static_cast<int>(effect.getKeyframesSize()); j++) {
    types::Keyframe kf = effect.getKeyframeAt(j);
    int currentTime = kf.getRelativePosition().value() + tsFX;
    if (currentTime < swriter.time + swriter.packetDuration && currentTime >= swriter.time) {
      if (firstKf) {
        firstKf = false;
        if (j != 0) {
          rau = false;
        }
      }
      writeKeyframe(swriter.bandStream.band.getBandType(), kf, bitstream);
      kfCount++;
      if (j == static_cast<int>(effect.getKeyframesSize()) - 1) {
        return true;
      }
    } else if (currentTime >= swriter.time + swriter.packetDuration) {
      return false;
    }
  }
  return true;
}
auto IOStream::readEffectBasis(std::vector<bool> &bitstream, types::Effect &effect,
                               types::BandType bandType, int &idx) -> bool {
  int kfCount = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_KEYFRAME_COUNT);
  if (bandType == types::BandType::VectorialWave) {
    float phase = IOBinaryPrimitives::readFloatNBits<EFFECT_PHASE>(bitstream, idx, 0, MAX_PHASE);
    effect.setPhase(phase);
    int baseSignal = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_BASE_SIGNAL);
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

auto IOStream::writeKeyframe(types::BandType bandType, types::Keyframe &keyframe,
                             std::vector<bool> &bitstream) -> bool {
  switch (bandType) {
  case types::BandType::Transient:
    return writeTransient(keyframe, bitstream);
  case types::BandType::Curve:
    return writeCurve(keyframe, bitstream);
  case types::BandType::VectorialWave:
    return writeVectorial(keyframe, bitstream);
  default:
    return false;
  }
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
}

auto IOStream::writeTransient(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
      keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());

  std::bitset<KEYFRAME_POSITION> posBits(keyframe.getRelativePosition().value());
  std::string posStr = posBits.to_string();
  IOBinaryPrimitives::writeStrBits(posStr, bitstream);

  std::bitset<KEYFRAME_FREQUENCY> freqBits(keyframe.getFrequencyModulation().value());
  std::string freqStr = freqBits.to_string();
  IOBinaryPrimitives::writeStrBits(freqStr, bitstream);
  return true;
}
auto IOStream::readTransient(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  float amplitude = IOBinaryPrimitives::readFloatNBits<KEYFRAME_AMPLITUDE>(
      bitstream, idx, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  keyframe.setAmplitudeModulation(amplitude);
  int position = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_POSITION);
  keyframe.setRelativePosition(position);
  int frequency = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_FREQUENCY);
  keyframe.setFrequencyModulation(frequency);
  length += idx;
  return true;
}

auto IOStream::writeCurve(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
      keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  std::bitset<KEYFRAME_POSITION> posBits(keyframe.getRelativePosition().value());
  std::string posStr = posBits.to_string();
  IOBinaryPrimitives::writeStrBits(posStr, bitstream);
  return true;
}
auto IOStream::readCurve(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  float amplitude = IOBinaryPrimitives::readFloatNBits<KEYFRAME_AMPLITUDE>(
      bitstream, idx, -MAX_AMPLITUDE, MAX_AMPLITUDE);
  keyframe.setAmplitudeModulation(amplitude);
  int position = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_POSITION);
  keyframe.setRelativePosition(position);
  length += idx;
  return true;
}

auto IOStream::writeVectorial(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool {
  std::vector<bool> bufbitstream = std::vector<bool>();
  std::bitset<2> informationMask{"00"};
  if (keyframe.getAmplitudeModulation().has_value()) {
    std::vector<bool> bufBits = std::vector<bool>();
    IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
        keyframe.getAmplitudeModulation().value(), bufBits, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    bufbitstream.insert(bufbitstream.end(), bufBits.begin(), bufBits.end());
    informationMask |= 0b01;
  }
  std::bitset<KEYFRAME_POSITION> posBits(keyframe.getRelativePosition().value());
  std::string posStr = posBits.to_string();
  IOBinaryPrimitives::writeStrBits(posStr, bufbitstream);

  if (keyframe.getFrequencyModulation().has_value()) {
    std::bitset<KEYFRAME_FREQUENCY> freqBits(keyframe.getFrequencyModulation().value());
    std::string freqStr = freqBits.to_string();
    IOBinaryPrimitives::writeStrBits(freqStr, bufbitstream);
    informationMask |= 0b10;
  }
  std::string informationMaskStr = informationMask.to_string();
  IOBinaryPrimitives::writeStrBits(informationMaskStr, bitstream);
  bitstream.insert(bitstream.end(), bufbitstream.begin(), bufbitstream.end());
  return true;
}
auto IOStream::readVectorial(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
    -> bool {
  int idx = 0;
  std::bitset<KEYFRAME_VECTORIAL_MASK> informationMask(
      IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_VECTORIAL_MASK));
  if (static_cast<int>(informationMask[0]) == 1) {
    float amplitude = IOBinaryPrimitives::readFloatNBits<KEYFRAME_AMPLITUDE>(
        bitstream, idx, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    keyframe.setAmplitudeModulation(amplitude);
  }
  int position = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_POSITION);
  keyframe.setRelativePosition(position);
  if (static_cast<int>(informationMask[1]) == 1) {
    int frequency = IOBinaryPrimitives::readUInt(bitstream, idx, KEYFRAME_FREQUENCY);
    keyframe.setFrequencyModulation(frequency);
  }
  length += idx;
  return true;
}

// auto IOStream::writeCRC(std::vector<bool> &bitstream) -> bool { return false; }

auto IOStream::searchPerceptionInHaptic(types::Haptics &haptic, int id) -> int {
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    if (id == haptic.getPerceptionAt(i).getId()) {
      return i;
    }
  }
  return -1;
}
auto IOStream::searchTrackInHaptic(types::Haptics &haptic, int id) -> int {
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(perception.getTracksSize()); j++) {
      if (id == perception.getTrackAt(j).getId()) {
        return j;
      }
    }
  }
  return -1;
}

auto IOStream::searchBandInHaptic(StreamReader &sreader, int id) -> int {
  for (auto &bandBuf : sreader.bandStreamsHaptic) {
    if (bandBuf.id == id) {
      return bandBuf.index;
    }
  }
  return -1;
}

template <class T> auto IOStream::searchInList(std::vector<T> &list, T &item, int id) -> bool {
  for (auto i = 0; i < static_cast<int>(list.size()); i++) {
    if (list[i].getId() == id) {
      item = list[i];
      list.erase(list.begin() + i);
      return true;
    }
  }
  return false;
}
auto IOStream::searchInList(std::vector<BandStream> &list, BandStream &item, int id) -> bool {
  for (auto i = 0; i < static_cast<int>(list.size()); i++) {
    if (list[i].id == id) {
      item = list[i];
      list.erase(list.begin() + i);

      return true;
    }
  }
  return false;
}

auto IOStream::readListObject(std::vector<bool> &bitstream, int refDevCount,
                              std::vector<types::ReferenceDevice> &refDevList, int &length)
    -> bool {
  int idx = 0;
  for (int i = 0; i < refDevCount; i++) {
    std::vector<bool> refDevBits(bitstream.begin() + idx, bitstream.end());
    types::ReferenceDevice refDev;
    if (!readReferenceDevice(refDevBits, refDev, idx)) {
      return false;
    }
    refDevList.push_back(refDev);
  }
  length += idx;
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

auto IOStream::addEffectToHaptic(types::Haptics &haptic, int perceptionIndex, int trackIndex,
                                 int bandIndex, std::vector<types::Effect> &effects) -> bool {
  for (auto &effect : effects) {
    bool effectExist = false;
    types::Band &band =
        haptic.getPerceptionAt(perceptionIndex).getTrackAt(trackIndex).getBandAt(bandIndex);
    if (effect.getEffectType() == types::EffectType::Basis) {
      for (auto i = 0; i < static_cast<int>(band.getEffectsSize()); i++) {
        types::Effect &hapticEffect = haptic.getPerceptionAt(perceptionIndex)
                                          .getTrackAt(trackIndex)
                                          .getBandAt(bandIndex)
                                          .getEffectAt(i);

        if (effect.getId() == hapticEffect.getId()) {
          for (size_t j = 0; j < effect.getKeyframesSize(); j++) {
            hapticEffect.addKeyframe(effect.getKeyframeAt(static_cast<int>(j)));
          }
          effectExist = true;
          break;
        }
      }
    } else {
      band.addEffect(effect);
      effectExist = true;
    }
    if (!effectExist) {
      band.addEffect(effect);
    }
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

auto IOStream::getEffectsId(types::Haptics &haptic) -> std::vector<int> {
  std::vector<int> effectsId = std::vector<int>();
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(perception.getTracksSize()); j++) {
      types::Track track = perception.getTrackAt(j);
      for (auto k = 0; k < static_cast<int>(track.getBandsSize()); k++) {
        types::Band band = track.getBandAt(k);
        for (int l = 0; l < static_cast<int>(band.getEffectsSize()); l++) {
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
auto IOStream::setNextEffectId(std::vector<int> &effectsId, types::Effect &effect) -> bool {
  int nextId = 0;
  if (!effectsId.empty()) {
    nextId = *max_element(effectsId.begin(), effectsId.end()) + 1;
  }
  effect.setId(nextId);
  effectsId.push_back(nextId);
  return true;
}
auto IOStream::getNextEffectId(std::vector<int> &effectsId) -> int {
  int nextId = 0;
  if (!effectsId.empty()) {
    nextId = *max_element(effectsId.begin(), effectsId.end()) + 1;
  }
  effectsId.push_back(nextId);
  return nextId;
}

auto IOStream::padToByteBoundary(std::vector<bool> &bitstream) -> void {
  int byte_stuffing = BYTE_SIZE - (static_cast<int>(bitstream.size()) % BYTE_SIZE);
  for (int i = 0; i < byte_stuffing; i++) {
    bitstream.push_back(false);
  }
}
} // namespace haptics::io