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

#ifndef IOSTREAM_H
#define IOSTREAM_H
#include <IOHaptics/include/IOBinary.h>
#include <Spiht/include/Spiht_Dec.h>
#include <Spiht/include/Spiht_Enc.h>
#include <Types/include/Haptics.h>
#include <bitset>
#include <string>
#include <tuple>
#include <vector>

namespace haptics::io {

static constexpr int TIME_TO_MS = 1000;
static constexpr int DEFAULT_PACKET_DURATION = 128;

static constexpr uint32_t CRC32_POLYNOMIAL = 2187366103;
static constexpr uint16_t CRC16_POLYNOMIAL = 49185;

enum class MIHSUnitType { Initialization, Temporal, Spatial, Silent };

enum class NALuType {
  Timing,
  MetadataHaptics,
  MetadataPerception,
  MetadataChannel,
  MetadataBand,
  Data,
  EffectLibrary,
  CRC32,
  CRC16,
  GlobalCRC16,
  GlobalCRC32
};

enum class AUType { RAU, DAU };

class IOStream {
public:
  struct BandStream {
    int id = -1;
    types::Band band = types::Band();
    int index = -1;
  };

  struct CRC {
    uint16_t polynomial16 = CRC16_POLYNOMIAL;
    uint32_t polynomial32 = CRC32_POLYNOMIAL;
    uint8_t nbPackets = 0;
    uint16_t value16 = 0;
    uint32_t value32 = 0;
  };

  struct StreamWriter {
    int time = 0;
    int timescale = TIME_TO_MS;
    int packetDuration = DEFAULT_PACKET_DURATION;
    types::Haptics haptic;
    types::Perception perception;
    types::Channel channel;
    BandStream bandStream;
    std::vector<types::Effect> effects;
    std::vector<int> keyframesCount;
    std::vector<int> effectsId;
    AUType auType = AUType::RAU;
  };

  struct StreamReader {
    types::Haptics haptic;
    types::Perception perception;
    types::Channel channel;
    types::Band band;
    BandStream bandStream;
    std::vector<BandStream> bandStreamsHaptic;
    AUType auType = AUType::RAU;
    int level = -1;
    int time = 0;
    int packetLength = 0;
    int packetDuration = 0;
    int timescale = 1;
    bool waitSync = false;
  };
  static auto readFile(const std::string &filePath, types::Haptics &haptic) -> bool;
  static auto loadFile(const std::string &filePath, std::vector<std::vector<bool>> &bitset) -> bool;
  static auto writeFile(types::Haptics &haptic, const std::string &filePath, int packetDuration)
      -> bool;
  static auto writeUnitFile(types::Haptics &haptic, const std::string &filePath, int packetDuration)
      -> bool;

  static auto writePacket(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writePacket(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream,
                          int packetDuration) -> bool;
  static auto writeUnits(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream,
                         int packetDuration) -> bool;

  static auto writeMIHSUnit(MIHSUnitType unitType, std::vector<std::vector<bool>> &listPackets,
                            std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool;
  static auto readMIHSUnit(std::vector<bool> &mihsunit, StreamReader &sreader, CRC &crc) -> bool;

  static auto writeNALu(NALuType naluType, StreamWriter &swriter, int level,
                        std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto writeAllBands(StreamWriter &swriter, NALuType naluType, int level,
                            std::vector<bool> &naluHeader,
                            std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto readNALu(std::vector<bool> packet, StreamReader &sreader, CRC &crc) -> bool;
  static auto initializeStream() -> StreamReader;

private:
  struct StartTimeIdx {
    int time = 0;
    int idx = 0;
  };

  static auto writeMIHSUnitInitialization(std::vector<std::vector<bool>> &listPackets,
                                          std::vector<bool> &mihsunit, StreamWriter &swriter)
      -> bool;

  static auto writeMIHSUnitTemporal(std::vector<std::vector<bool>> &listPackets,
                                    std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool;
  static auto writeMIHSUnitSpatial(std::vector<std::vector<bool>> &listPackets,
                                   std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool;
  static auto writeMIHSUnitSilent(std::vector<std::vector<bool>> &listPackets,
                                  std::vector<bool> &mihsunit, StreamWriter &swriter) -> bool;

  static auto readMIHSUnitInitialization(std::vector<bool> &mihsunit, StreamReader &sreader)
      -> bool;

  static auto writeNALuHeader(NALuType naluType, int level, int payloadSize,
                              std::vector<bool> &bitstream) -> bool;
  static auto writeNALuPayload(NALuType naluType, types::Haptics &haptic,
                               std::vector<bool> &bitstream) -> bool;
  static auto writeTiming(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto writeAvatar(types::Avatar &avatar, std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataPerception(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool;

  static auto writeLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool;
  static auto writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &bitstream)
      -> bool;

  static auto writeReferenceDevice(types::ReferenceDevice &refDevice, std::vector<bool> &bitstream)
      -> bool;
  static auto generateReferenceDeviceInformationMask(types::ReferenceDevice &referenceDevice,
                                                     std::vector<bool> &informationMask) -> bool;
  static auto writeMetadataChannel(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataBand(StreamWriter &swriter, std::vector<bool> &bitstream) -> bool;
  static auto writeData(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto writeSpatialData(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream)
      -> bool;
  static auto packetizeBand(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstreams)
      -> bool;

  static auto createWaveletPayload(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream)
      -> bool;

  static auto createPayloadPacket(StreamWriter &swriter, std::vector<std::vector<bool>> &bitstream)
      -> bool;
  static auto writePayloadPacket(StreamWriter &swriter,
                                 std::vector<std::vector<bool>> bufPacketBitstream,
                                 std::vector<bool> &packetBits) -> std::vector<bool>;
  static auto writeWaveletPayloadPacket(std::vector<bool> bufPacketBitstream,
                                        std::vector<bool> &packetBits, std::vector<int> &effectsId)
      -> std::vector<bool>;
  static auto readWaveletEffect(std::vector<bool> &bitstream, types::Band &band,
                                types::Effect &effect, int &length) -> bool;
  static auto writeEffectHeader(StreamWriter &swriter) -> std::vector<bool>;
  static auto writeEffectBasis(types::Effect effect, StreamWriter &swriter, int &kfCount, bool &rau,
                               std::vector<bool> &bitstream) -> bool;

  static auto writeKeyframe(types::BandType bandType, types::Keyframe &keyframe,
                            std::vector<bool> &bitstream) -> bool;
  static auto writeTransient(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;
  static auto writeCurve(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;
  static auto writeVectorial(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;

  static auto writeCRC(std::vector<std::vector<bool>> &bitstream, std::vector<bool> &packetCRC,
                       int crcLevel) -> bool;

  static auto computeCRC(std::vector<bool> &bitstream, std::vector<bool> &polynomial) -> bool;

  static auto sortPacket(std::vector<std::vector<std::vector<bool>>> &bandPacket,
                         std::vector<std::vector<bool>> &output) -> bool;

  static auto readPacketTS(std::vector<bool> bitstream) -> int;
  static auto readPacketLength(std::vector<bool> &bitstream) -> int;

  static auto readNALuType(std::vector<bool> &packet) -> NALuType;
  static auto readNALuHeader(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto readMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto readAvatar(std::vector<bool> &bitstream, types::Avatar &avatar, int &length) -> bool;
  static auto readMetadataPerception(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readEffectsLibrary(std::vector<bool> &bitstream, std::vector<types::Effect> &effects)
      -> bool;
  static auto readReferenceDevice(std::vector<bool> &bitstream, types::ReferenceDevice &refDevice,
                                  int &length) -> bool;
  static auto readLibrary(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readLibraryEffect(types::Effect &libraryEffect, int &idx,
                                std::vector<bool> &bitstream) -> bool;
  static auto readMetadataChannel(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readMetadataBand(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readSpatialData(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readData(StreamReader &sreader, std::vector<bool> &bitstream) -> bool;
  static auto readEffect(types::Effect &effect, std::vector<bool> &bitstream) -> bool;
  static auto readCRC(std::vector<bool> &bitstream, CRC &crc, NALuType naluType) -> bool;

  static auto getEffectsId(types::Haptics &haptic) -> std::vector<int>;

  static auto readListObject(std::vector<bool> &bitstream, int avatarCount,
                             std::vector<types::Avatar> &avatarList) -> bool;
  static auto readListObject(std::vector<bool> &bitstream, int refDevCount,
                             std::vector<types::ReferenceDevice> &refDevList, int &length) -> bool;

  static auto addEffectToHaptic(types::Haptics &haptic, int perceptionIndex, int channelIndex,
                                int bandIndex, std::vector<types::Effect> &effects) -> bool;
  template <class T> static auto searchInList(std::vector<T> &list, T &item, int id) -> bool;
  static auto searchInList(std::vector<BandStream> &list, BandStream &item, int id) -> bool;
  static auto searchPerceptionInHaptic(types::Haptics &haptic, int id) -> int;
  static auto searchChannelInHaptic(types::Haptics &haptic, int id) -> int;
  static auto searchBandInHaptic(StreamReader &sreader, int id) -> int;

  static auto readListObject(std::vector<bool> &bitstream, int fxCount, types::Band &band,
                             std::vector<types::Effect> &fxList, int &length) -> bool;

  static auto readEffect(std::vector<bool> &bitstream, types::Effect &effect, types::Band &band,
                         int &length) -> bool;

  static auto readEffectBasis(std::vector<bool> &bitstream, types::Effect &effect,
                              types::BandType bandType, int &idx) -> bool;

  static auto readListObject(std::vector<bool> &bitstream, int kfCount, types::BandType &bandType,
                             std::vector<types::Keyframe> &kfList, int &length) -> bool;

  static auto readKeyframe(std::vector<bool> &bitstream, types::Keyframe &keyframe,
                           types::BandType &bandType, int &length) -> bool;

  static auto readTransient(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
      -> bool;
  static auto readCurve(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
      -> bool;
  static auto readVectorial(std::vector<bool> &bitstream, types::Keyframe &keyframe, int &length)
      -> bool;

  static auto readTimelineEffect(std::vector<types::Effect> &timeline, std::vector<bool> &bitstream)
      -> bool;
  static auto linearizeTimeline(types::Band &band) -> void;
  static auto linearizeTimelineEffect(types::Effect &effect, std::vector<types::Effect> &effects)
      -> void;
  static auto checkCRC(std::vector<std::vector<bool>> &bitstream, CRC &crc) -> bool;
  static auto checkHapticComponent(types::Haptics &haptic) -> void;

  static auto padToByteBoundary(std::vector<bool> &bitstream) -> void;
  static auto setNextEffectId(std::vector<int> &effectsId, types::Effect &effect) -> bool;
  static auto getNextEffectId(std::vector<int> &effectsId) -> int;
  static auto addTimestampEffect(std::vector<types::Effect> &effects, int timestamp) -> bool;
  static auto silentUnitSyncFlag(std::vector<std::vector<bool>> &bitstream) -> void;
};
} // namespace haptics::io
#endif // IOSTREAM_H