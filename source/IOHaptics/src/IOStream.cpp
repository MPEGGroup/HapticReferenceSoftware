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
  sreader.haptic.setTimescale(sreader.timescale); // TODO: earlier?
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
    int unitNBits =
        UNIT_TYPE + UNIT_SYNC + UNIT_LAYER + UNIT_DURATION + UNIT_LENGTH + UNIT_RESERVED;
    IOBinaryPrimitives::readNBytes(file, static_cast<int>(unitNBits / BYTE_SIZE), bufPacket);
    byteCount += static_cast<int>(unitNBits / BYTE_SIZE);
    // read packet payload length
    int lengthIdx = unitNBits - (UNIT_LENGTH + UNIT_RESERVED);
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

auto IOStream::writeUnits(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream,
                          int packetDuration) -> bool {
  StreamWriter swriter;
  swriter.haptic = haptic;
  swriter.packetDuration = packetDuration;
  swriter.timescale = haptic.getTimescaleOrDefault();
  std::vector<std::vector<bool>> initPackets = std::vector<std::vector<bool>>();
  writeMIHSPacket(MIHSPacketType::MetadataHaptics, swriter, initPackets);
  writeMIHSPacket(MIHSPacketType::MetadataPerception, swriter, initPackets);
  writeMIHSPacket(MIHSPacketType::EffectLibrary, swriter, initPackets);
  writeMIHSPacket(MIHSPacketType::MetadataChannel, swriter, initPackets);
  writeMIHSPacket(MIHSPacketType::MetadataBand, swriter, initPackets);
  std::vector<bool> initUnit = std::vector<bool>();
  writeMIHSUnit(MIHSUnitType::Initialization, initPackets, initUnit, swriter);
  bitstream.push_back(initUnit);
  types::Sync nextSync;
  int syncIdx = 0;
  getNextSync(haptic, nextSync, syncIdx);

  std::vector<std::vector<bool>> dataPackets = std::vector<std::vector<bool>>();
  writeMIHSPacket(MIHSPacketType::Data, swriter, dataPackets);
  std::vector<std::vector<bool>> bufUnit = std::vector<std::vector<bool>>();
  swriter.time = 0;
  bool first = true;
  for (auto &packet : dataPackets) {
    if (first) {
      std::vector<std::vector<bool>> firstPacket = std::vector<std::vector<bool>>{packet};
      std::vector<bool> silentUnit = std::vector<bool>();
      writeMIHSUnit(MIHSUnitType::Silent, firstPacket, silentUnit, swriter);
      if (silentUnit.size() > UNIT_TYPE) {
        bitstream.push_back(silentUnit);
      }
      first = false;
    }
    if (bufUnit.empty()) {
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
        if (syncIdx != -1 && swriter.time == nextSync.getTimestamp()) {
          std::vector<bool> syncUnit = std::vector<bool>();
          writeMIHSUnit(MIHSUnitType::Initialization, initPackets, syncUnit, swriter);
          getNextSync(haptic, nextSync, syncIdx);
          bitstream.push_back(syncUnit);
        }
        if (swriter.time != packetTS) {
          std::vector<std::vector<bool>> silentPackets{bufUnit[bufUnit.size() - 1], packet};
          std::vector<bool> silentUnit = std::vector<bool>();
          if (writeMIHSUnit(MIHSUnitType::Silent, silentPackets, silentUnit, swriter)) {
            bitstream.push_back(silentUnit);
            if (syncIdx != -1 && swriter.time == nextSync.getTimestamp()) {
              std::vector<bool> syncUnit = std::vector<bool>();
              writeMIHSUnit(MIHSUnitType::Initialization, initPackets, syncUnit, swriter);
              getNextSync(haptic, nextSync, syncIdx);
              bitstream.push_back(syncUnit);
            }
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
    if (syncIdx != -1 && swriter.time == nextSync.getTimestamp()) {
      std::vector<bool> syncUnit = std::vector<bool>();
      writeMIHSUnit(MIHSUnitType::Initialization, initPackets, syncUnit, swriter);
      getNextSync(haptic, nextSync, syncIdx);
      bitstream.push_back(syncUnit);
    }
  }
  silentUnitSyncFlag(bitstream);
  return true;
}

auto IOStream::silentUnitSyncFlag(std::vector<std::vector<bool>> &bitstream) -> void {
  // Check Silent Unit sync flag to match next temporal unit sync flag
  for (int i = 0; i < static_cast<int>(bitstream.size()); i++) {
    int index = 0;
    std::vector<bool> mihsunit =
        std::vector<bool>(bitstream[i].begin() + index, bitstream[i].end());
    int unitTypeInt = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_TYPE);
    auto unitType = static_cast<MIHSUnitType>(unitTypeInt);
    if (unitType == MIHSUnitType::Silent) {
      if (i < static_cast<int>(bitstream.size()) - 1) {
        for (int j = i + 1; j < static_cast<int>(bitstream.size()); j++) {
          int bufindex = 0;
          std::vector<bool> bufunit =
              std::vector<bool>(bitstream[j].begin() + bufindex, bitstream[j].end());
          int bufunitTypeInt = IOBinaryPrimitives::readUInt(bufunit, bufindex, UNIT_TYPE);
          auto bufunitType = static_cast<MIHSUnitType>(bufunitTypeInt);
          if (bufunitType == MIHSUnitType::Temporal ||
              bufunitType == MIHSUnitType::Initialization) {
            std::copy(bitstream[j].begin() + bufindex, bitstream[j].begin() + bufindex + UNIT_SYNC,
                      bitstream[i].begin() + index);
            break;
          }
        }
      }
    }
  }
}

auto IOStream::readMIHSUnit(std::vector<bool> &mihsunit, StreamReader &sreader, CRC &crc) -> bool {
  int index = 0;
  IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_TYPE);
  // MIHSUnitType unitType = static_cast<MIHSUnitType>(unitTypeInt);
  int syncInt = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_SYNC);
  bool sync = syncInt == 0;
  if (sreader.waitSync && sync) {
    sreader.waitSync = false;
  }
  sreader.layer = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_LAYER);

  sreader.packetDuration = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_DURATION);
  int unitLength = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_LENGTH) * BYTE_SIZE;
  index += UNIT_RESERVED;

  std::vector<bool> packets = std::vector<bool>(mihsunit.begin() + index, mihsunit.end());
  while (index < unitLength) {
    if (!readMIHSPacket(packets, sreader, crc)) {
      return EXIT_FAILURE;
    }
    index += static_cast<int>(sreader.packetLength) + H_NBITS;
    packets = std::vector<bool>(mihsunit.begin() + index, mihsunit.end());
  }
  sreader.time += sreader.packetDuration;
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
  std::bitset<UNIT_SYNC> syncBits(0);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  std::bitset<UNIT_LAYER> layerBits(swriter.layer);
  std::string layerStr = layerBits.to_string();
  IOBinaryPrimitives::writeStrBits(layerStr, mihsunit);
  std::bitset<UNIT_DURATION> durationBits(0);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);

  std::vector<bool> packetFusion = std::vector<bool>();
  std::vector<std::vector<bool>> timingPacket = std::vector<std::vector<bool>>();
  // Add a mandatory timing packet in mihs unit of type initialization
  writeMIHSPacket(MIHSPacketType::InitializationTiming, swriter, timingPacket);
  packetFusion.insert(packetFusion.end(), timingPacket[0].begin(), timingPacket[0].end());
  for (auto &packet : listPackets) {
    packetFusion.insert(packetFusion.end(), packet.begin(), packet.end());
  }

  int length = static_cast<int>(packetFusion.size()) / BYTE_SIZE;
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  std::bitset<UNIT_RESERVED> resBits(0);
  const std::string resStr = resBits.to_string();
  IOBinaryPrimitives::writeStrBits(resStr, mihsunit);
  mihsunit.insert(mihsunit.end(), packetFusion.begin(), packetFusion.end());
  return true;
}
auto IOStream::writeMIHSUnitTemporal(std::vector<std::vector<bool>> &listPackets,
                                     std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  bool sync = true;
  int nbPacketData = 0;
  std::vector<bool> payload = std::vector<bool>();
  for (auto &packet : listPackets) {
    std::vector<bool> bufPacket = packet;
    MIHSPacketType mihsPacketType = readMIHSPacketType(packet);
    if (mihsPacketType == MIHSPacketType::Data) {
      bufPacket.erase(bufPacket.begin() + H_NBITS, bufPacket.begin() + H_NBITS + DB_DURATION);
      nbPacketData++;
      sync &= !bufPacket[H_NBITS];
      int packetStartTime = readPacketTS(std::vector<bool>(packet.begin() + H_NBITS, packet.end()));
      swriter.time = packetStartTime;
    }

    payload.insert(payload.end(), bufPacket.begin(), bufPacket.end());
  }

  int syncInt = sync ? 0 : 1;
  std::bitset<UNIT_SYNC> syncBits(syncInt);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  std::bitset<UNIT_LAYER> layerBits(swriter.layer);
  std::string layerStr = layerBits.to_string();
  IOBinaryPrimitives::writeStrBits(layerStr, mihsunit);
  int duration = 0;
  if (nbPacketData > 0) {
    duration = static_cast<int>(swriter.packetDuration);
  }
  std::bitset<UNIT_DURATION> durationBits(duration);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
  int length = static_cast<int>(payload.size()) / BYTE_SIZE;
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  std::bitset<UNIT_RESERVED> reservedBits(0);
  std::string reservedStr = reservedBits.to_string();
  IOBinaryPrimitives::writeStrBits(reservedStr, mihsunit);
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
    MIHSPacketType mihsPacketType = readMIHSPacketType(packet);
    if (mihsPacketType == MIHSPacketType::Data) {
      bufPacket.erase(bufPacket.begin() + H_NBITS, bufPacket.begin() + H_NBITS + DB_DURATION);
    }
    length += static_cast<int>(H_NBITS / BYTE_SIZE) + readPacketLength(bufPacket);
    payload.insert(payload.end(), bufPacket.begin(), bufPacket.end());
  }
  swriter.auType = AUType::RAU;
  int sync = swriter.auType == AUType::RAU ? 0 : 1;
  std::bitset<UNIT_SYNC> syncBits(sync);
  std::string syncStr = syncBits.to_string();
  IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
  std::bitset<UNIT_LAYER> layerBits(swriter.layer);
  std::string layerStr = layerBits.to_string();
  IOBinaryPrimitives::writeStrBits(layerStr, mihsunit);
  int duration = 0;
  std::bitset<UNIT_DURATION> durationBits(duration);
  std::string durationStr = durationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
  std::bitset<UNIT_LENGTH> lengthBits(length);
  std::string lengthStr = lengthBits.to_string();
  IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
  std::bitset<UNIT_RESERVED> reservedBits(0);
  std::string reservedStr = reservedBits.to_string();
  IOBinaryPrimitives::writeStrBits(reservedStr, mihsunit);
  mihsunit.insert(mihsunit.end(), payload.begin(), payload.end());
  return true;
}
auto IOStream::writeMIHSUnitSilent(std::vector<std::vector<bool>> &listPackets,
                                   std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool {
  if (listPackets.size() == 1) {
    int tFirst =
        readPacketTS(std::vector<bool>(listPackets[0].begin() + H_NBITS, listPackets[0].end()));
    if (tFirst >= static_cast<int>(swriter.packetDuration)) {
      std::bitset<UNIT_SYNC> syncBits(0);
      std::string syncStr = syncBits.to_string();
      IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
      std::bitset<UNIT_LAYER> layerBits(swriter.layer);
      std::string layerStr = layerBits.to_string();
      IOBinaryPrimitives::writeStrBits(layerStr, mihsunit);
      int duration = tFirst;
      if (duration % swriter.packetDuration != 0) {
        duration = duration - (duration % static_cast<int>(swriter.packetDuration));
      }
      std::bitset<UNIT_DURATION> durationBits(duration);
      std::string durationStr = durationBits.to_string();
      IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
      std::bitset<UNIT_LENGTH> lengthBits(0);
      std::string lengthStr = lengthBits.to_string();
      IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
      std::bitset<UNIT_RESERVED> reservedBits(0);
      std::string reservedStr = reservedBits.to_string();
      IOBinaryPrimitives::writeStrBits(reservedStr, mihsunit);
      swriter.time += duration;
    }
    return true;
  }
  if (listPackets.size() == 2) {

    std::bitset<UNIT_SYNC> syncBits(0);
    std::string syncStr = syncBits.to_string();
    IOBinaryPrimitives::writeStrBits(syncStr, mihsunit);
    std::bitset<UNIT_LAYER> layerBits(swriter.layer);
    std::string layerStr = layerBits.to_string();
    IOBinaryPrimitives::writeStrBits(layerStr, mihsunit);
    int start = swriter.time;
    int end =
        readPacketTS(std::vector<bool>(listPackets[1].begin() + H_NBITS, listPackets[1].end()));
    int duration = end - start;
    if (duration % swriter.packetDuration != 0) {
      duration = duration - (duration % static_cast<int>(swriter.packetDuration));
    }
    std::bitset<UNIT_DURATION> durationBits(duration);
    std::string durationStr = durationBits.to_string();
    IOBinaryPrimitives::writeStrBits(durationStr, mihsunit);
    std::bitset<UNIT_LENGTH> lengthBits(0);
    std::string lengthStr = lengthBits.to_string();
    IOBinaryPrimitives::writeStrBits(lengthStr, mihsunit);
    std::bitset<UNIT_RESERVED> reservedBits(0);
    std::string reservedStr = reservedBits.to_string();
    IOBinaryPrimitives::writeStrBits(reservedStr, mihsunit);
    swriter.time += duration;
    return true;
  }
  return false;
}
auto IOStream::initializeStream() -> StreamReader {
  StreamReader sreader;
  sreader.haptic = types::Haptics();
  sreader.bandStreamsHaptic = std::vector<BandStream>();

  return sreader;
}

auto IOStream::writeMIHSPacket(MIHSPacketType mihsPacketType, StreamWriter &swriter,
                               std::vector<std::vector<bool>> &bitstream) -> bool {

  checkHapticComponent(swriter.haptic);
  std::vector<bool> mihsPacketHeader = std::vector<bool>();
  switch (mihsPacketType) {
  case MIHSPacketType::Timing: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    writeTiming(swriter, mihsPacketPayload);
    writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                          mihsPacketHeader);
    mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                            mihsPacketPayload.end());
    padToByteBoundary(mihsPacketHeader);
    bitstream.push_back(mihsPacketHeader);
    return true;
  }
  case MIHSPacketType::MetadataHaptics: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    writeMetadataHaptics(swriter.haptic, mihsPacketPayload);
    writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                          mihsPacketHeader);
    mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                            mihsPacketPayload.end());
    padToByteBoundary(mihsPacketHeader);
    bitstream.push_back(mihsPacketHeader);
    return true;
  }
  case MIHSPacketType::MetadataPerception: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      swriter.perception = swriter.haptic.getPerceptionAt(i);
      writeMetadataPerception(swriter, mihsPacketPayload);
      writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                            mihsPacketHeader);
      mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                              mihsPacketPayload.end());
      padToByteBoundary(mihsPacketHeader);
      bitstream.push_back(mihsPacketHeader);
      mihsPacketPayload.clear();
      mihsPacketHeader.clear();
    }
    return true;
  }
  case MIHSPacketType::EffectLibrary: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      if (swriter.haptic.getPerceptionAt(i).getEffectLibrarySize() != 0) {
        writeLibrary(swriter.haptic.getPerceptionAt(i), mihsPacketPayload);
        writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                              mihsPacketHeader);
        mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                                mihsPacketPayload.end());
        padToByteBoundary(mihsPacketHeader);
        bitstream.push_back(mihsPacketHeader);
        mihsPacketPayload.clear();
        mihsPacketHeader.clear();
      }
    }
    return true;
  }
  case MIHSPacketType::MetadataChannel: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
      swriter.perception = swriter.haptic.getPerceptionAt(i);
      for (auto j = 0; j < static_cast<int>(swriter.haptic.getPerceptionAt(i).getChannelsSize());
           j++) {
        swriter.channel = swriter.perception.getChannelAt(j);
        writeMetadataChannel(swriter, mihsPacketPayload);
        writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                              mihsPacketHeader);
        mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                                mihsPacketPayload.end());
        padToByteBoundary(mihsPacketHeader);
        bitstream.push_back(mihsPacketHeader);
        mihsPacketPayload.clear();
        mihsPacketHeader.clear();
      }
    }
    return true;
  }
  case MIHSPacketType::MetadataBand: {
    return writeAllBands(swriter, mihsPacketType, mihsPacketHeader, bitstream);
  }
  case MIHSPacketType::Data: {
    std::vector<std::vector<bool>> mihsPacketPayload = std::vector<std::vector<bool>>();
    writeData(swriter, mihsPacketPayload);
    for (auto data : mihsPacketPayload) {
      writeMIHSPacketHeader(mihsPacketType, static_cast<int>(data.size()), mihsPacketHeader);
      mihsPacketHeader.insert(mihsPacketHeader.end(), data.begin(), data.end());
      padToByteBoundary(mihsPacketHeader);
      bitstream.push_back(mihsPacketHeader);
      mihsPacketHeader.clear();
    }
    return true;
  }
  case MIHSPacketType::InitializationTiming: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    writeInitializationTiming(swriter, mihsPacketPayload);
    writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                          mihsPacketHeader);
    mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                            mihsPacketPayload.end());
    padToByteBoundary(mihsPacketHeader);
    bitstream.push_back(mihsPacketHeader);
    return true;
  }
  case MIHSPacketType::CRC16:
  case MIHSPacketType::GlobalCRC16:
  case MIHSPacketType::CRC32:
  case MIHSPacketType::GlobalCRC32: {
    std::vector<bool> mihsPacketPayload = std::vector<bool>();
    int crcLevel = 0;
    if (mihsPacketType == MIHSPacketType::CRC32 || mihsPacketType == MIHSPacketType::GlobalCRC32) {
      crcLevel = 1;
    }
    writeCRC(bitstream, mihsPacketPayload, crcLevel);
    writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                          mihsPacketHeader);
    mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                            mihsPacketPayload.end());
    padToByteBoundary(mihsPacketHeader);
    bitstream.clear();
    bitstream.push_back(mihsPacketHeader);
    return true;
  }
  default:
    return false;
  }
}

auto IOStream::writeAllBands(StreamWriter &swriter, MIHSPacketType mihsPacketType,
                             std::vector<bool> &mihsPacketHeader,
                             std::vector<std::vector<bool>> &bitstream) -> bool {
  std::vector<bool> mihsPacketPayload = std::vector<bool>();
  int bandId = 0;
  for (auto i = 0; i < static_cast<int>(swriter.haptic.getPerceptionsSize()); i++) {
    swriter.perception = swriter.haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(swriter.perception.getChannelsSize()); j++) {
      swriter.channel = swriter.perception.getChannelAt(j);
      for (auto k = 0; k < static_cast<int>(swriter.channel.getBandsSize()); k++) {
        swriter.bandStream.band = swriter.channel.getBandAt(k);
        swriter.bandStream.id = bandId++;
        writeMetadataBand(swriter, mihsPacketPayload);
        padToByteBoundary(mihsPacketPayload);
        writeMIHSPacketHeader(mihsPacketType, static_cast<int>(mihsPacketPayload.size()),
                              mihsPacketHeader);
        mihsPacketHeader.insert(mihsPacketHeader.end(), mihsPacketPayload.begin(),
                                mihsPacketPayload.end());
        bitstream.push_back(mihsPacketHeader);
        mihsPacketPayload.clear();
        mihsPacketHeader.clear();
      }
    }
  }
  return true;
}

auto IOStream::checkHapticComponent(types::Haptics &haptic) -> void {
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    types::Perception &perception = haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(perception.getChannelsSize()); j++) {
      types::Channel &channel = perception.getChannelAt(j);
      std::vector<types::Band> bands = std::vector<types::Band>();
      for (auto k = 0; k < static_cast<int>(channel.getBandsSize()); k++) {
        types::Band &band = channel.getBandAt(k);
        for (auto l = 0; l < static_cast<int>(band.getEffectsSize()); l++) {
          types::Effect &effect = band.getEffectAt(l);
          if (band.getBandType() != types::BandType::WaveletWave &&
              effect.getEffectType() == types::EffectType::Basis &&
              effect.getKeyframesSize() == 0) {
            band.removeEffectAt(l);
          }
        }
        if (band.getEffectsSize() == 0) {
          channel.removeBandAt(k);
        }
      }
      if (channel.getBandsSize() == 0) {
        perception.removeChannelAt(j);
      }
    }
    if (perception.getChannelsSize() == 0) {
      haptic.removePerceptionAt(i);
    }
  }
}

auto IOStream::writeMIHSPacketHeader(MIHSPacketType mihsPacketType, int payloadSize,
                                     std::vector<bool> &bitstream) -> bool {
  std::bitset<H_MIHS_PACKET_TYPE> mihsPacketTypeBits(static_cast<int>(mihsPacketType));
  const std::string mihsPacketTypeStr = mihsPacketTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(mihsPacketTypeStr, bitstream);
  int missing = (payloadSize % BYTE_SIZE) == 0 ? 0 : (BYTE_SIZE - (payloadSize % BYTE_SIZE));
  int payloadSizeByte = (payloadSize + missing) / BYTE_SIZE;
  if (mihsPacketType == MIHSPacketType::Data) {
    payloadSizeByte -= DB_DURATION / BYTE_SIZE;
  }
  std::bitset<H_PAYLOAD_LENGTH> payloadSizeBits(payloadSizeByte);
  const std::string payloadSizeStr = payloadSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(payloadSizeStr, bitstream);
  std::bitset<H_RESERVED> resBits(0);
  const std::string resStr = resBits.to_string();
  IOBinaryPrimitives::writeStrBits(resStr, bitstream);
  return true;
}
auto IOStream::readMIHSPacket(std::vector<bool> packet, StreamReader &sreader, CRC &crc) -> bool {
  MIHSPacketType mihsPacketType = readMIHSPacketType(packet);
  int index = H_MIHS_PACKET_TYPE;
  sreader.packetLength = IOBinaryPrimitives::readUInt(packet, index, H_PAYLOAD_LENGTH) * BYTE_SIZE;
  index += H_RESERVED;
  std::vector<bool> payload = std::vector<bool>(packet.begin() + index, packet.end());
  switch (mihsPacketType) {
  case (MIHSPacketType::Timing): {
    readTiming(sreader, payload);
    sreader.time = (sreader.time * TIME_TO_MS) / sreader.timescale;
    return true;
  }
  case (MIHSPacketType::MetadataHaptics): {
    return readMetadataHaptics(sreader.haptic, payload);
  }
  case (MIHSPacketType::MetadataPerception): {
    if (!readMetadataPerception(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    if (perceIndex == -1) {
      sreader.haptic.addPerception(sreader.perception);
    } else {
      sreader.haptic.replacePerceptionMetadataAt(perceIndex, sreader.perception);
    }
    return true;
  }
  case (MIHSPacketType::EffectLibrary): {
    return readLibrary(sreader, payload);
  }
  case (MIHSPacketType::MetadataChannel): {
    if (!readMetadataChannel(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    int channelIndex = searchChannelInHaptic(sreader.haptic, sreader.channel.getId());
    if (channelIndex == -1) {
      sreader.haptic.getPerceptionAt(perceIndex).addChannel(sreader.channel);
    } else {
      sreader.haptic.getPerceptionAt(perceIndex)
          .replaceChannelMetadataAt(channelIndex, sreader.channel);
    }
    return true;
  }
  case (MIHSPacketType::MetadataBand): {
    if (!readMetadataBand(sreader, payload)) {
      return false;
    }
    int perceIndex = searchPerceptionInHaptic(sreader.haptic, sreader.perception.getId());
    int channelIndex = searchChannelInHaptic(sreader.haptic, sreader.channel.getId());
    int bandIndex = searchBandInHaptic(sreader, sreader.bandStream.id);
    if (bandIndex == -1 ||
        sreader.haptic.getPerceptionAt(perceIndex).getChannelAt(channelIndex).getBandsSize() == 0 ||
        static_cast<int>(
            sreader.haptic.getPerceptionAt(perceIndex).getChannelAt(channelIndex).getBandsSize()) <
            bandIndex) {
      sreader.haptic.getPerceptionAt(perceIndex)
          .getChannelAt(channelIndex)
          .addBand(sreader.bandStream.band);
      sreader.bandStream.index = static_cast<int>(sreader.haptic.getPerceptionAt(perceIndex)
                                                      .getChannelAt(channelIndex)
                                                      .getBandsSize()) -
                                 1;
      sreader.bandStreamsHaptic.push_back(sreader.bandStream);
    } else {
      sreader.haptic.getPerceptionAt(perceIndex)
          .getChannelAt(channelIndex)
          .replaceBandMetadataAt(bandIndex, sreader.bandStream.band);
    }
    return true;
  }
  case (MIHSPacketType::Data): {
    return readData(sreader, payload);
  }
  case (MIHSPacketType::CRC16):
  case (MIHSPacketType::CRC32):
  case (MIHSPacketType::GlobalCRC16):
  case (MIHSPacketType::GlobalCRC32): {
    return readCRC(payload, crc, mihsPacketType);
  }
  case (MIHSPacketType::InitializationTiming): {
    return readInitializationTiming(sreader, payload);
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

auto IOStream::readMIHSPacketType(std::vector<bool> &packet) -> MIHSPacketType {
  int idx = 0;
  int typeInt = IOBinaryPrimitives::readUInt(packet, idx, H_MIHS_PACKET_TYPE);

  return static_cast<MIHSPacketType>(typeInt);
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

  return true;
}

auto IOStream::readTiming(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  int index = 0;
  int timestamp = IOBinaryPrimitives::readUInt(bitstream, index, TIMING_TIME);

  auto sync = types::Sync(timestamp);
  sreader.time = timestamp;
  sreader.haptic.addSync(sync);

  return true;
}

auto IOStream::writeInitializationTiming(StreamWriter &swriter, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<TIMING_TIME> timestampBits = swriter.time;
  std::string timestampStr = timestampBits.to_string();
  IOBinaryPrimitives::writeStrBits(timestampStr, bitstream);

  std::bitset<INITTIMING_TIMESCALE> timescaleBits = swriter.timescale;
  std::string timescaleStr = timescaleBits.to_string();
  IOBinaryPrimitives::writeStrBits(timescaleStr, bitstream);

  std::bitset<INITTIMING_NOMINALDURATION> nominalDurationBits = swriter.nominalDuration;
  std::string nominalDurationStr = nominalDurationBits.to_string();
  IOBinaryPrimitives::writeStrBits(nominalDurationStr, bitstream);

  std::bitset<INITTIMING_DURATIONDEVIATION> durationDeviationBits = swriter.durationDeviation;
  std::string durationDeviationStr = durationDeviationBits.to_string();
  IOBinaryPrimitives::writeStrBits(durationDeviationStr, bitstream);

  std::bitset<INITTIMING_OVERLAPPING> overlappingBits(static_cast<int>(swriter.overlapping));
  std::string overlappingStr = overlappingBits.to_string();
  IOBinaryPrimitives::writeStrBits(overlappingStr, bitstream);

  return true;
}

auto IOStream::readInitializationTiming(StreamReader &sreader, std::vector<bool> &bitstream)
    -> bool {
  int index = 0;
  int timestamp = IOBinaryPrimitives::readUInt(bitstream, index, TIMING_TIME);
  int timescale = IOBinaryPrimitives::readUInt(bitstream, index, INITTIMING_TIMESCALE);
  int nominalDuration = IOBinaryPrimitives::readUInt(bitstream, index, INITTIMING_NOMINALDURATION);
  int durationDeviation =
      IOBinaryPrimitives::readUInt(bitstream, index, INITTIMING_DURATIONDEVIATION);
  bool overlapping = IOBinaryPrimitives::readUInt(bitstream, index, INITTIMING_OVERLAPPING) == 1;
  types::Sync sync = types::Sync(timestamp, timescale);
  sreader.haptic.addSync(sync);
  sreader.nominalDuration = nominalDuration;
  sreader.durationDeviation = durationDeviation;
  sreader.overlapping = overlapping;
  sreader.timescale = timescale;
  sreader.time = timestamp;
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

  std::bitset<MDEXP_PROFILE_SIZE> profileCountBits(haptic.getProfile().size());
  const std::string profileCountStr = profileCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(profileCountStr, bitstream);
  for (auto c : haptic.getProfile()) {
    std::bitset<BYTE_SIZE> cBits(c);
    const std::string cStr = cBits.to_string();
    IOBinaryPrimitives::writeStrBits(cStr, bitstream);
  }

  std::bitset<MDEXP_LEVEL> levelBits(haptic.getLevel());
  std::string levelStr = levelBits.to_string();
  IOBinaryPrimitives::writeStrBits(levelStr, bitstream);

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

  int profileLength = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_PROFILE_SIZE);
  std::string profile = IOBinaryPrimitives::readString(bitstream, index, profileLength);
  haptic.setProfile(profile);

  int level = IOBinaryPrimitives::readUInt(bitstream, index, MDEXP_LEVEL);
  haptic.setLevel(level);

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

  std::bitset<MDPERCE_PRIORITY> percePriorityBits(swriter.perception.getPriorityOrDefault());
  std::string percePriorityStr = percePriorityBits.to_string();
  IOBinaryPrimitives::writeStrBits(percePriorityStr, bitstream);

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

  if (swriter.perception.getEffectSemanticScheme().has_value()) {
    std::string schemeStr = swriter.perception.getEffectSemanticScheme().value();
    std::bitset<MDPERCE_FLAG_SEMANTIC> flagSemanticBits(1);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, bitstream);
    std::bitset<MDPERCE_SCHEME_LENGTH> schemeLengthBits(schemeStr.size());
    std::string shemeLengthStr = schemeLengthBits.to_string();
    IOBinaryPrimitives::writeStrBits(shemeLengthStr, bitstream);
    for (auto c : schemeStr) {
      std::bitset<MDPERCE_SCHEME_CHAR> cBits(c);
      const std::string cStr = cBits.to_string();
      IOBinaryPrimitives::writeStrBits(cStr, bitstream);
    }
  } else {
    std::bitset<MDPERCE_FLAG_SEMANTIC> flagSemanticBits(0);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, bitstream);
  }

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

  std::bitset<MDPERCE_CHANNEL_COUNT> channelCountBits(swriter.perception.getChannelsSize());
  std::string channelCountStr = channelCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(channelCountStr, bitstream);
  return true;
}
auto IOStream::readMetadataPerception(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {
  int idx = 0;

  int id = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  sreader.perception = types::Perception();

  sreader.perception.setId(id);

  int priority = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_PRIORITY);
  if (priority != 0) {
    sreader.perception.setPriority(priority);
  }

  int descLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_DESC_SIZE);

  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  sreader.perception.setDescription(desc);

  int modal = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_MODALITY);
  sreader.perception.setPerceptionModality(static_cast<types::PerceptionModality>(modal));

  int avatarId = IOBinaryPrimitives::readUInt(bitstream, idx, AVATAR_ID);
  sreader.perception.setAvatarId(avatarId);

  // read effect library size, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_LIBRARY_COUNT);

  int flagScheme = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_FLAG_SEMANTIC);
  if (flagScheme == 1) {
    int schemeLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_SCHEME_LENGTH);
    std::string schemeStr = IOBinaryPrimitives::readString(bitstream, idx, schemeLength);
    sreader.perception.setEffectSemanticScheme(schemeStr);
  }

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
  // read channel count, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_CHANNEL_COUNT);

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

  int effectType = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TYPE);
  libraryEffect.setEffectType(static_cast<types::EffectType>(effectType));

  int hasSemantic = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_FLAG_SEMANTIC);
  if (hasSemantic == 1) {
    int semanticCode = IOBinaryPrimitives::readUInt(
        bitstream, idx, EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2);
    auto semantic = std::string(
        types::effectSemanticToString.at(static_cast<types::EffectSemantic>(semanticCode)));
    libraryEffect.setSemantic(semantic);
  }
  int position = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_POSITION_STREAMING);
  libraryEffect.setPosition(position);

  if (effectType == 0) {
    float phase = IOBinaryPrimitives::readFloatNBits<EFFECT_PHASE>(bitstream, idx, 0, MAX_PHASE);
    libraryEffect.setPhase(phase);

    int baseSignal = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_BASE_SIGNAL);
    libraryEffect.setBaseSignal(static_cast<types::BaseSignal>(baseSignal));
  }

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

  return success;
}

auto IOStream::writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &bitstream)
    -> bool {
  std::bitset<EFFECT_ID> idBits(libraryEffect.getId());
  std::string idStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(idStr, bitstream);

  std::bitset<EFFECT_TYPE> typeBits(static_cast<int>(libraryEffect.getEffectType()));
  std::string typeStr = typeBits.to_string();
  IOBinaryPrimitives::writeStrBits(typeStr, bitstream);

  if (libraryEffect.getSemantic().has_value()) {
    std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(1);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, bitstream);

    types::EffectSemantic semantic =
        types::stringToEffectSemantic.at(libraryEffect.getSemantic().value());
    std::bitset<EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2> semanticBits(
        static_cast<int>(semantic));
    std::string semanticStr = semanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(semanticStr, bitstream);
  } else {
    std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(0);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, bitstream);
  }

  std::bitset<EFFECT_POSITION_STREAMING> posBits(libraryEffect.getPosition());
  std::string posStr = posBits.to_string();
  IOBinaryPrimitives::writeStrBits(posStr, bitstream);

  if (libraryEffect.getEffectType() == types::EffectType::Basis) {

    IOBinaryPrimitives::writeFloatNBits<uint32_t, EFFECT_PHASE>(libraryEffect.getPhaseOrDefault(),
                                                                bitstream, 0, MAX_PHASE);

    std::bitset<EFFECT_BASE_SIGNAL> baseBits(
        static_cast<int>(libraryEffect.getBaseSignalOrDefault()));
    std::string baseStr = baseBits.to_string();
    IOBinaryPrimitives::writeStrBits(baseStr, bitstream);
  }

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

auto IOStream::writeMetadataChannel(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDCHANNEL_ID> idBits(swriter.channel.getId());
  std::string valueStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::bitset<MDPERCE_ID> perceidBits(swriter.perception.getId());
  valueStr = perceidBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::bitset<MDCHANNEL_PRIORITY> priorityBits(swriter.channel.getPriorityOrDefault());
  std::string priorityStr = priorityBits.to_string();
  IOBinaryPrimitives::writeStrBits(priorityStr, bitstream);

  std::bitset<MDPERCE_DESC_SIZE> descSizeBits(swriter.channel.getDescription().size());
  valueStr = descSizeBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  for (char &c : swriter.channel.getDescription()) {
    std::bitset<BYTE_SIZE> descBits(c);
    valueStr = descBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<REFDEV_ID> refDevID(swriter.channel.getReferenceDeviceId().value_or(-1));
  valueStr = refDevID.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  std::vector<bool> bufBits = std::vector<bool>();
  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDCHANNEL_GAIN>(swriter.channel.getGain(), bufBits,
                                                                -MAX_FLOAT, MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  IOBinaryPrimitives::writeFloatNBits<uint32_t, MDCHANNEL_MIXING_WEIGHT>(
      swriter.channel.getMixingWeight(), bufBits, 0, MAX_FLOAT);
  bitstream.insert(bitstream.end(), bufBits.begin(), bufBits.end());
  bufBits.clear();

  auto optionalMetadataMask = (uint8_t)0b0000'0000;
  if (swriter.channel.getActuatorResolution().has_value()) {
    optionalMetadataMask |= (uint8_t)0b0000'0010;
  } else {
    optionalMetadataMask |= (uint8_t)0b0000'0001;
  }
  if (swriter.channel.getDirection().has_value()) {
    optionalMetadataMask |= (uint8_t)0b0000'0100;
  }
  std::bitset<MDCHANNEL_OPT_FIELDS> optionalMetadataMaskBits(optionalMetadataMask);
  valueStr = optionalMetadataMaskBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  if ((optionalMetadataMask & (uint8_t)0b0000'0001) != 0) {
    std::bitset<MDCHANNEL_BODY_PART_MASK> bodyPartMaskBits(swriter.channel.getBodyPartMask());
    valueStr = bodyPartMaskBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  } else if ((optionalMetadataMask & (uint8_t)0b0000'0010) != 0) {
    types::Vector channelResolution = swriter.channel.getActuatorResolution().value();
    IOBinaryPrimitives::writeVector(channelResolution, bitstream);

    std::vector<types::BodyPartTarget> bodyPartTarget =
        swriter.channel.getBodyPartTarget().value_or(std::vector<types::BodyPartTarget>{});
    auto bodyPartTargetCount = static_cast<uint8_t>(bodyPartTarget.size());
    IOBinaryPrimitives::writeNBits<uint8_t, MDCHANNEL_BODY_PART_TARGET_COUNT>(bodyPartTargetCount,
                                                                              bitstream);
    for (uint8_t i = 0; i < bodyPartTargetCount; i++) {
      IOBinaryPrimitives::writeNBits<uint8_t, MDCHANNEL_BODY_PART_TARGET>(
          static_cast<uint8_t>(bodyPartTarget[i]), bitstream);
    }

    std::vector<types::Vector> actuatorTarget =
        swriter.channel.getActuatorTarget().value_or(std::vector<types::Vector>{});
    auto actuatorTargetCount = static_cast<uint8_t>(actuatorTarget.size());
    IOBinaryPrimitives::writeNBits<uint8_t, MDCHANNEL_ACTUATOR_TARGET_COUNT>(actuatorTargetCount,
                                                                             bitstream);
    for (uint8_t i = 0; i < actuatorTargetCount; i++) {
      types::Vector target = actuatorTarget[i];
      IOBinaryPrimitives::writeVector(target, bitstream);
    }
  }

  int freqSampling = static_cast<int>(swriter.channel.getFrequencySampling().value_or(0));
  std::bitset<MDCHANNEL_FREQ_SAMPLING> maxFreqBits(freqSampling);
  valueStr = maxFreqBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  if (freqSampling > 0) {
    std::bitset<MDCHANNEL_SAMPLE_COUNT> sampleCountBits(
        swriter.channel.getSampleCount().value_or(0));
    valueStr = sampleCountBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  if (swriter.channel.getDirection().has_value()) {
    types::Vector dir = swriter.channel.getDirection().value();
    std::bitset<MDCHANNEL_DIRECTION_AXIS> axisValue(dir.X);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
    axisValue = std::bitset<MDCHANNEL_DIRECTION_AXIS>(dir.Y);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
    axisValue = std::bitset<MDCHANNEL_DIRECTION_AXIS>(dir.Z);
    valueStr = axisValue.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<MDCHANNEL_VERT_COUNT> vertCountBits(swriter.channel.getVerticesSize());
  valueStr = vertCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  for (auto i = 0; i < static_cast<int>(swriter.channel.getVerticesSize()); i++) {
    std::bitset<MDCHANNEL_VERT> vertBits(swriter.channel.getVertexAt(i));
    valueStr = vertBits.to_string();
    IOBinaryPrimitives::writeStrBits(valueStr, bitstream);
  }

  std::bitset<MDCHANNEL_BANDS_COUNT> bandsCountBits(swriter.channel.getBandsSize());
  valueStr = bandsCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(valueStr, bitstream);

  return true;
}
auto IOStream::readMetadataChannel(StreamReader &sreader, std::vector<bool> &bitstream) -> bool {

  sreader.channel = types::Channel();
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_ID);
  sreader.channel.setId(id);

  int perceId = IOBinaryPrimitives::readUInt(bitstream, idx, MDPERCE_ID);
  int perceIndex = searchPerceptionInHaptic(sreader.haptic, perceId);
  if (perceIndex == -1) {
    return false;
  }
  sreader.perception = sreader.haptic.getPerceptionAt(perceIndex);

  int priority = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_PRIORITY);
  if (priority != 0) {
    sreader.channel.setPriority(priority);
  }

  int descLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_DESC_LENGTH);
  std::string desc = IOBinaryPrimitives::readString(bitstream, idx, descLength);
  sreader.channel.setDescription(desc);

  int deviceId = IOBinaryPrimitives::readUInt(bitstream, idx, REFDEV_ID);
  if (deviceId < REFDEV_MAX_ID) {
    sreader.channel.setReferenceDeviceId(deviceId);
  }
  float gain =
      IOBinaryPrimitives::readFloatNBits<MDCHANNEL_GAIN>(bitstream, idx, -MAX_FLOAT, MAX_FLOAT);
  sreader.channel.setGain(gain);

  float mixingWeight =
      IOBinaryPrimitives::readFloatNBits<MDCHANNEL_MIXING_WEIGHT>(bitstream, idx, 0, MAX_FLOAT);

  sreader.channel.setMixingWeight(mixingWeight);
  uint8_t optionalMetadataMask = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_OPT_FIELDS);
  if ((optionalMetadataMask & 0b0000'0001) != 0) {
    uint32_t bodyPartMask = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_BODY_PART_MASK);
    sreader.channel.setBodyPartMask(bodyPartMask);
  } else if ((optionalMetadataMask & 0b0000'0010) != 0) {
    auto X = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
    auto Y = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
    auto Z = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
    sreader.channel.setActuatorResolution(haptics::types::Vector(X, Y, Z));
    auto bodyPartTargetCount = static_cast<uint8_t>(
        IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_BODY_PART_TARGET_COUNT));
    std::vector<types::BodyPartTarget> bodyPartTarget(bodyPartTargetCount,
                                                      types::BodyPartTarget::Unknown);
    for (auto &target : bodyPartTarget) {
      target = static_cast<types::BodyPartTarget>(
          IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_BODY_PART_TARGET));
    }
    sreader.channel.setBodyPartTarget(bodyPartTarget);

    auto actuatorTargetCount =
        IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_ACTUATOR_TARGET_COUNT);
    std::vector<types::Vector> actuatorTarget(actuatorTargetCount);
    for (auto &target : actuatorTarget) {
      auto X = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
      auto Y = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
      auto Z = static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, VECTOR_AXIS_SIZE));
      target = haptics::types::Vector(X, Y, Z);
    }
    sreader.channel.setActuatorTarget(actuatorTarget);
  }
  uint32_t frequencySampling =
      IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_FREQ_SAMPLING);
  if (frequencySampling > 0) {
    sreader.channel.setFrequencySampling(frequencySampling);
    uint32_t sampleCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_SAMPLE_COUNT);
    sreader.channel.setSampleCount(sampleCount);
  }

  if ((optionalMetadataMask & 0b0000'0100) != 0) {
    auto X =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_DIRECTION_AXIS));
    auto Y =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_DIRECTION_AXIS));
    auto Z =
        static_cast<int8_t>(IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_DIRECTION_AXIS));
    sreader.channel.setDirection(haptics::types::Vector(X, Y, Z));
  }

  int verticesCount = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_VERT_COUNT);
  for (int i = 0; i < verticesCount; i++) {
    int vertex = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_VERT);
    sreader.channel.addVertex(vertex);
  }

  // read band count, unused but could be used for check
  IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_BANDS_COUNT);

  return true;
}

auto IOStream::writeMetadataBand(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool {
  std::bitset<MDBAND_ID> idBits(swriter.bandStream.id);
  std::string idStr = idBits.to_string();
  IOBinaryPrimitives::writeStrBits(idStr, bitstream);
  std::bitset<MDPERCE_ID> perceidBits(swriter.perception.getId());
  std::string perceidStr = perceidBits.to_string();
  IOBinaryPrimitives::writeStrBits(perceidStr, bitstream);
  std::bitset<MDCHANNEL_ID> channelidBits(swriter.channel.getId());
  std::string channelidStr = channelidBits.to_string();
  IOBinaryPrimitives::writeStrBits(channelidStr, bitstream);
  std::bitset<MDBAND_PRIORITY> priorityBits(swriter.bandStream.band.getPriorityOrDefault());
  std::string priorityStr = priorityBits.to_string();
  IOBinaryPrimitives::writeStrBits(priorityStr, bitstream);

  std::bitset<MDBAND_BAND_TYPE> bandTypeBits(
      static_cast<int>(swriter.bandStream.band.getBandType()));
  std::string bandTypeStr = bandTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(bandTypeStr, bitstream);
  if (swriter.bandStream.band.getBandType() == types::BandType::Curve) {
    std::bitset<MDBAND_CURVE_TYPE> curveTypeBits(
        static_cast<int>(swriter.bandStream.band.getCurveTypeOrDefault()));
    std::string curveTypeStr = curveTypeBits.to_string();
    IOBinaryPrimitives::writeStrBits(curveTypeStr, bitstream);
  } else if (swriter.bandStream.band.getBandType() == types::BandType::WaveletWave) {
    std::bitset<MDBAND_BLK_LEN> winLengthBits(
        static_cast<uint8_t>(swriter.bandStream.band.getBlockLengthOrDefault()));
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

  int channelId = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_ID);
  int channelIndex = searchChannelInHaptic(sreader.haptic, channelId);
  if (channelIndex == -1) {
    return false;
  }
  sreader.channel = sreader.perception.getChannelAt(channelIndex);

  int priority = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_PRIORITY);
  if (priority != 0) {
    sreader.bandStream.band.setPriority(priority);
  }

  int bandType = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_BAND_TYPE);
  sreader.bandStream.band.setBandType(static_cast<types::BandType>(bandType));
  if (sreader.bandStream.band.getBandType() == types::BandType::Curve) {
    int curveType = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_CURVE_TYPE);
    sreader.bandStream.band.setCurveType(static_cast<types::CurveType>(curveType));
  } else if (sreader.bandStream.band.getBandType() == types::BandType::WaveletWave) {
    int windowLength = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_BLK_LEN);
    sreader.bandStream.band.setBlockLength(windowLength);
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
    if (effect.getEffectType() == types::EffectType::Composite) {
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
    if (effectTimeline.getEffectType() == types::EffectType::Composite) {
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
      swriter.auType = AUType::RAU;
      packetBits = writeEffectHeader(swriter);
      packetBits = writeWaveletPayloadPacket(bufpacket, packetBits, swriter);
      bitstreams.push_back(packetBits);
      swriter.time += static_cast<int>(swriter.bandStream.band.getBlockLengthOrDefault());
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
      swriter.time += static_cast<int>(swriter.packetDuration);
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
  if (swriter.bandStream.band.getBandType() != types::BandType::WaveletWave ||
      !swriter.bandStream.band.getBlockLength().has_value()) {
    return false;
  }
  int nbWaveBlock = static_cast<int>(swriter.packetDuration) /
                    static_cast<int>(swriter.bandStream.band.getBlockLength().value());
  for (auto i = 0; i < static_cast<int>(swriter.bandStream.band.getEffectsSize());
       i += nbWaveBlock) {
    std::vector<bool> bufbitstream = std::vector<bool>();
    types::Effect bufEffect = swriter.bandStream.band.getEffectAt(i);
    IOBinaryBands::writeWaveletEffect(bufEffect, bufbitstream);
    bitstream.push_back(bufbitstream);
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
        swriter.auType = isRAU ? AUType::RAU : AUType::DAU;
      }
      if (endEffect && i == static_cast<int>(swriter.bandStream.band.getEffectsSize()) - 1) {
        return true;
      }
      if (endPacket) {
        return false;
      }
    } else if (effect.getEffectType() == types::EffectType::Reference) {
      if (effect.getPosition() >= swriter.time &&
          effect.getPosition() < swriter.time + static_cast<int>(swriter.packetDuration)) {
        bitstream.push_back(bufEffect);
        swriter.effects.push_back(effect);
        swriter.auType = AUType::RAU;
        swriter.keyframesCount.push_back(0);
      } else if (effect.getPosition() > swriter.time + static_cast<int>(swriter.packetDuration)) {
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
  std::bitset<MDCHANNEL_ID> channelIdBits(swriter.channel.getId());
  std::string channelIdStr = channelIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(channelIdStr, packetBits);

  std::bitset<MDBAND_ID> bandIdBits(swriter.bandStream.id);
  std::string bandIdStr = bandIdBits.to_string();
  IOBinaryPrimitives::writeStrBits(bandIdStr, packetBits);

  return packetBits;
}

auto IOStream::writeWaveletPayloadPacket(std::vector<bool> bufPacketBitstream,
                                         std::vector<bool> &packetBits, StreamWriter &swriter)
    -> std::vector<bool> {
  std::bitset<DB_EFFECT_COUNT> fxCountBits(1);
  std::string fxCountStr = fxCountBits.to_string();
  IOBinaryPrimitives::writeStrBits(fxCountStr, packetBits);

  int id = getNextEffectId(swriter.effectsId);
  std::bitset<EFFECT_ID> effectIDBits(static_cast<int>(id));
  std::string effectIDStr = effectIDBits.to_string();
  IOBinaryPrimitives::writeStrBits(effectIDStr, packetBits);

  std::bitset<EFFECT_TYPE> effectTypeBits(static_cast<int>(types::EffectType::Basis));
  std::string effectTypeStr = effectTypeBits.to_string();
  IOBinaryPrimitives::writeStrBits(effectTypeStr, packetBits);

  if (swriter.bandStream.band.getEffectAt(0).getSemantic().has_value()) {
    std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(1);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, packetBits);

    types::EffectSemantic semantic = types::stringToEffectSemantic.at(
        swriter.bandStream.band.getEffectAt(0).getSemantic().value());
    std::bitset<EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2> semanticBits(
        static_cast<int>(semantic));
    std::string semanticStr = semanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(semanticStr, packetBits);

  } else {
    std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(0);
    std::string flagSemanticStr = flagSemanticBits.to_string();
    IOBinaryPrimitives::writeStrBits(flagSemanticStr, packetBits);
  }

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
    std::bitset<EFFECT_POSITION> effectPosBits(effectPos);
    std::string effectPosStr = effectPosBits.to_string();
    IOBinaryPrimitives::writeStrBits(effectPosStr, packetBits);

    if (swriter.effects[l].getEffectType() == types::EffectType::Basis) {
      if (swriter.effects[l].getSemantic().has_value()) {
        std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(1);
        std::string flagSemanticStr = flagSemanticBits.to_string();
        IOBinaryPrimitives::writeStrBits(flagSemanticStr, packetBits);

        types::EffectSemantic semantic =
            types::stringToEffectSemantic.at(swriter.effects[l].getSemantic().value());
        std::bitset<EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2> semanticBits(
            static_cast<int>(semantic));
        std::string semanticStr = semanticBits.to_string();
        IOBinaryPrimitives::writeStrBits(semanticStr, packetBits);
      } else {
        std::bitset<EFFECT_FLAG_SEMANTIC> flagSemanticBits(0);
        std::string flagSemanticStr = flagSemanticBits.to_string();
        IOBinaryPrimitives::writeStrBits(flagSemanticStr, packetBits);
      }

      std::bitset<EFFECT_KEYFRAME_COUNT> kfCountBits(swriter.keyframesCount[l]);
      std::string kfCountStr = kfCountBits.to_string();
      IOBinaryPrimitives::writeStrBits(kfCountStr, packetBits);
      if (swriter.bandStream.band.getBandType() == types::BandType::VectorialWave) {
        IOBinaryPrimitives::writeFloatNBits<uint16_t, EFFECT_PHASE>(
            swriter.effects[l].getPhaseOrDefault(), packetBits, 0, MAX_PHASE);
        std::bitset<EFFECT_BASE_SIGNAL> fxBaseBits(
            static_cast<int>(swriter.effects[l].getBaseSignalOrDefault()));
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
    for (auto j = 0; j < static_cast<int>(swriter.perception.getChannelsSize()); j++) {
      swriter.channel = swriter.perception.getChannelAt(j);
      for (auto k = 0; k < static_cast<int>(swriter.channel.getBandsSize()); k++) {
        BandStream bandStream;
        bandStream.id = bandId++;
        bandStream.band = swriter.channel.getBandAt(k);
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

  int channelId = IOBinaryPrimitives::readUInt(bitstream, idx, MDCHANNEL_ID);
  auto channelIndex = searchChannelInHaptic(sreader.haptic, channelId);
  if (channelIndex == -1) {
    return false;
  }
  sreader.channel = sreader.perception.getChannelAt(channelIndex);
  sreader.bandStream.index = -1;
  sreader.bandStream.id = -1;
  sreader.bandStream.id = IOBinaryPrimitives::readUInt(bitstream, idx, MDBAND_ID);
  sreader.bandStream.index = searchBandInHaptic(sreader, sreader.bandStream.id);
  if (sreader.bandStream.index == -1) {
    return false;
  }
  sreader.bandStream.band = sreader.channel.getBandAt(sreader.bandStream.index);

  int fxCount = IOBinaryPrimitives::readUInt(bitstream, idx, DB_EFFECT_COUNT);
  if (fxCount > 0) {
    std::vector<types::Effect> effects;
    std::vector<bool> effectsBitsList(bitstream.begin() + idx, bitstream.end());
    if (sreader.bandStream.band.getBandType() != types::BandType::WaveletWave) {
      if (!readListObject(effectsBitsList, fxCount, sreader.bandStream.band, effects, idx)) {
        return false;
      }
      addTimestampEffect(effects, static_cast<int>(sreader.time));
    } else {
      types::Effect effect;
      IOStream::readWaveletEffect(effectsBitsList, sreader.bandStream.band, effect, idx,
                                  sreader.timescale);
      effects.push_back(effect);
    }
    return addEffectToHaptic(sreader.haptic, perceptionIndex, channelIndex,
                             sreader.bandStream.index, effects);
  }
  return true;
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
auto IOStream::readCRC(std::vector<bool> &bitstream, CRC &crc, MIHSPacketType mihsPacketType)
    -> bool {
  int idx = 0;
  if (mihsPacketType == MIHSPacketType::CRC16) {
    crc.nbPackets = 1;
    crc.value16 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC16_NB_BITS);
    crc.value32 = 0;
    return true;
  }
  if (mihsPacketType == MIHSPacketType::CRC32) {
    crc.nbPackets = 1;
    crc.value32 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC32_NB_BITS);
    crc.value16 = 0;
    return true;
  }
  if (mihsPacketType == MIHSPacketType::GlobalCRC16) {
    crc.nbPackets = IOBinaryPrimitives::readUInt(bitstream, idx, GCRC_NB_PACKET);
    crc.value16 = IOBinaryPrimitives::readUInt(bitstream, idx, CRC16_NB_BITS);
    crc.value32 = 0;
    return true;
  }
  if (mihsPacketType == MIHSPacketType::GlobalCRC32) {
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
                                 types::Effect &effect, int &length, const unsigned int timescale)
    -> bool {
  int idx = 0;
  int id = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_ID);
  effect.setId(id);

  types::EffectType effectType =
      static_cast<types::EffectType>(IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_TYPE));
  effect.setEffectType(effectType);

  int hasSemantic = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_FLAG_SEMANTIC);
  if (hasSemantic == 1) {
    int semanticCode = IOBinaryPrimitives::readUInt(
        bitstream, idx, EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2);
    auto semantic = std::string(
        types::effectSemanticToString.at(static_cast<types::EffectSemantic>(semanticCode)));
    effect.setSemantic(semantic);
  }

  int effectPos = static_cast<int>(timescale) * band.getBlockLength().value() *
                  static_cast<int>(band.getEffectsSize()) / band.getUpperFrequencyLimit();
  effect.setPosition(effectPos);

  IOBinaryBands::readWaveletEffect(effect, bitstream, idx);
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

  if (effectType == types::EffectType::Basis) {
    int hasSemantic = IOBinaryPrimitives::readUInt(bitstream, idx, EFFECT_FLAG_SEMANTIC);
    if (hasSemantic == 1) {
      int semanticCode = IOBinaryPrimitives::readUInt(
          bitstream, idx, EFFECT_SEMANTIC_LAYER_1 + EFFECT_SEMANTIC_LAYER_2);
      auto semantic = std::string(
          types::effectSemanticToString.at(static_cast<types::EffectSemantic>(semanticCode)));
      effect.setSemantic(semantic);
    }
    if (!readEffectBasis(bitstream, effect, band.getBandType(), idx)) {
      return false;
    }
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
    if (currentTime < swriter.time + static_cast<int>(swriter.packetDuration) &&
        currentTime >= swriter.time) {
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
    } else if (currentTime >= swriter.time + static_cast<int>(swriter.packetDuration)) {
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

auto IOStream::searchPerceptionInHaptic(types::Haptics &haptic, int id) -> int {
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    if (id == haptic.getPerceptionAt(i).getId()) {
      return i;
    }
  }
  return -1;
}
auto IOStream::searchChannelInHaptic(types::Haptics &haptic, int id) -> int {
  for (auto i = 0; i < static_cast<int>(haptic.getPerceptionsSize()); i++) {
    types::Perception perception = haptic.getPerceptionAt(i);
    for (auto j = 0; j < static_cast<int>(perception.getChannelsSize()); j++) {
      if (id == perception.getChannelAt(j).getId()) {
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

auto IOStream::addEffectToHaptic(types::Haptics &haptic, int perceptionIndex, int channelIndex,
                                 int bandIndex, std::vector<types::Effect> &effects) -> bool {
  for (auto &effect : effects) {
    bool effectExist = false;
    types::Band &band =
        haptic.getPerceptionAt(perceptionIndex).getChannelAt(channelIndex).getBandAt(bandIndex);
    if (effect.getEffectType() == types::EffectType::Basis) {
      for (auto i = 0; i < static_cast<int>(band.getEffectsSize()); i++) {
        types::Effect &hapticEffect = haptic.getPerceptionAt(perceptionIndex)
                                          .getChannelAt(channelIndex)
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
    for (auto j = 0; j < static_cast<int>(perception.getChannelsSize()); j++) {
      types::Channel channel = perception.getChannelAt(j);
      for (auto k = 0; k < static_cast<int>(channel.getBandsSize()); k++) {
        types::Band band = channel.getBandAt(k);
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
  int missing = static_cast<int>(bitstream.size()) % BYTE_SIZE;
  if (missing != 0) {
    int byte_stuffing = BYTE_SIZE - missing;
    for (int i = 0; i < byte_stuffing; i++) {
      bitstream.push_back(false);
    }
  }
}

auto IOStream::getNextSync(types::Haptics &haptic, types::Sync &sync, int &idx) -> bool {
  if (idx >= static_cast<int>(haptic.getSyncsSize()) - 1) {
    idx = -1;
    return false;
  }
  if (idx == 0 && haptic.getSyncsAt(0).getTimestamp() == 0) {
    if (haptic.getSyncsSize() == 1) {
      idx = -1;
      return false;
    }
  }
  sync = haptic.getSyncsAt(++idx);
  return true;
}
} // namespace haptics::io