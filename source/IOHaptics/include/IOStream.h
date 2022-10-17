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

static constexpr int H_NBITS = 32;
static constexpr int H_NALU_TYPE = 4;
static constexpr int H_LEVEL = 2;
static constexpr int H_PAYLOAD_LENGTH = 16;
static constexpr int H_PAYLOAD_BYTES_LENGTH = 16;
static constexpr int H_BYTES_LENGTH = 4;

static constexpr int MDEXP_VERSION = 16;
static constexpr int MDEXP_DATE = 16;
static constexpr int MDEXP_DESC_SIZE = 16;
static constexpr int MDEXP_PERC_COUNT = 8;
static constexpr int MDEXP_AVATAR_COUNT = 8;

static constexpr int AVATAR_ID = 8;
static constexpr int AVATAR_LOD = 8;
static constexpr int AVATAR_TYPE = 3;
static constexpr int AVATAR_MESH_COUNT = 16;

static constexpr int MDPERCE_ID = 8;
static constexpr int MDPERCE_DESC_SIZE = 16;
static constexpr int MDPERCE_MODALITY = 4;
static constexpr int MDPERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_PERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_LIBRARY_COUNT = 16;
static constexpr int MDPERCE_REFDEVICE_COUNT = 8;
static constexpr int MDPERCE_TRACK_COUNT = 8;
static constexpr int MDPERCE_FXLIB_COUNT = 16;

static constexpr int REFDEV_ID = 32;
static constexpr int REFDEV_NAME_LENGTH = 16;
static constexpr int REFDEV_OPT_FIELDS = 12;
static constexpr int REFDEV_BODY_PART_MASK = 32;
static constexpr int REFDEV_MAX_FREQ = 32;
static constexpr int REFDEV_MIN_FREQ = 32;
static constexpr int REFDEV_RES_FREQ = 32;
static constexpr int REFDEV_MAX_AMP = 32;
static constexpr int REFDEV_IMPEDANCE = 32;
static constexpr int REFDEV_MAX_VOLT = 32;
static constexpr int REFDEV_MAX_CURR = 32;
static constexpr int REFDEV_MAX_DISP = 32;
static constexpr int REFDEV_WEIGHT = 32;
static constexpr int REFDEV_SIZE = 32;
static constexpr int REFDEV_CUSTOM = 32;
static constexpr int REFDEV_TYPE = 3;

static constexpr int MDTRACK_ID = 8;
static constexpr int MDTRACK_DESC_LENGTH = 16;
static constexpr int MDTRACK_GAIN = 32;
static constexpr int MDTRACK_MIXING_WEIGHT = 32;
static constexpr int MDTRACK_BODY_PART_MASK = 32;
static constexpr int MDTRACK_SAMPLING_FREQUENCY = 32;
static constexpr int MDTRACK_VERT_COUNT = 16;
static constexpr int MDTRACK_VERT = 32;
static constexpr int MDTRACK_BANDS_COUNT = 16;
static constexpr int MDTRACK_DIRECTION_MASK = 1;
static constexpr int MDTRACK_DIRECTION_AXIS = 8;
static constexpr int MDTRACK_SAMPLE_COUNT = 32;

static constexpr int MDBAND_ID = 8;
static constexpr int MDBAND_BAND_TYPE = 2;
static constexpr int MDBAND_CURVE_TYPE = 3;
static constexpr int MDBAND_WIN_LEN = 16;
static constexpr int MDBAND_LOW_FREQ = 16;
static constexpr int MDBAND_UP_FREQ = 16;
static constexpr int MDBAND_FX_COUNT = 16;

static constexpr int DB_AU_TYPE = 1;
static constexpr int DB_TIMESTAMP = 32;
static constexpr int DB_FX_COUNT = 16;

static constexpr int FX_ID = 16;
static constexpr int FX_TYPE = 2;
static constexpr int FX_POSITION = 24;
static constexpr int FX_REF_ID = 32;
static constexpr int FX_PHASE = 16;
static constexpr int FX_BASE = 4;
static constexpr int FX_KF_COUNT = 16;
static constexpr int EFFECT_WAVELET_SIZE = 16;

static constexpr int FXLIB_TIMELINESIZE = 16;

static constexpr int KF_AMPLITUDE = 8;
static constexpr int KF_POSITION = 16;
static constexpr int KF_FREQUENCY = 16;
static constexpr int KF_INFORMATION_MASK = 2;
static constexpr int KF_MASK = 3;

static constexpr int PACKET_DURATION = 128;
static constexpr int PACKET_HEADER_SIZE = 32;

enum class NALuType {
  MetadataHaptics = 1,
  MetadataPerception,
  MetadataTrack,
  MetadataBand,
  EffectLibrary,
  Data,
  CRC,
  ByteStuffing
};

enum class PacketType {
  Metadata = 1,
  Data,
  ProtectedMetadata,
  ProtectedData,
};
static int BANDID = 0;

class IOStream {
public:
  struct BandStream {
    int id = -1;
    types::Band band = types::Band();
    int index = -1;
  };

  struct Buffer {
    std::vector<types::Perception> perceptionsBuffer;
    std::vector<types::Track> tracksBuffer;
    std::vector<BandStream> bandStreamsBuffer;
    std::vector<BandStream> bandStreamsHaptic;
  };
  static auto readFile(const std::string &filePath, types::Haptics &haptic) -> bool;
  static auto loadFile(const std::string &filePath, std::vector<std::vector<bool>> &bitset) -> bool;
  static auto writeFile(types::Haptics &haptic, const std::string &filePath) -> bool;

  static auto writePacket(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writePacket(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream)
      -> bool;

  static auto writeNALu(NALuType naluType, types::Haptics &haptic, int level,
                        std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto writeAllBands(types::Haptics &haptic, NALuType naluType, int level,
                            std::vector<bool> &naluHeader,
                            std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto readNALu(types::Haptics &haptic, std::vector<bool> packet, Buffer &buffer) -> bool;
  static auto initializeStream(types::Haptics &haptic) -> Buffer;

private:
  struct StartTimeIdx {
    int time = 0;
    int idx = 0;
  };

  static auto writeNALuHeader(NALuType naluType, int level, int payloadSize,
                              std::vector<bool> &bitstream) -> bool;
  static auto writeNALuPayload(NALuType naluType, types::Haptics &haptic,
                               std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto writeAvatar(types::Avatar &avatar, std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataPerception(types::Perception &perception, std::vector<bool> &bitstream)
      -> bool;

  static auto writeLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool;
  static auto writeLibraryEffect(types::Effect &libraryEffect, std::vector<bool> &bitstream)
      -> bool;

  static auto writeReferenceDevice(types::ReferenceDevice &refDevice, std::vector<bool> &bitstream)
      -> bool;
  static auto generateReferenceDeviceInformationMask(types::ReferenceDevice &referenceDevice,
                                                     std::vector<bool> &informationMask) -> bool;
  static auto writeMetadataTrack(types::Track &track, std::vector<bool> &bitstream) -> bool;
  static auto writeMetadataBand(types::Band &band, std::vector<bool> &bitstream, int id) -> bool;
  static auto writeData(types::Haptics &haptic, std::vector<std::vector<bool>> &bitstream) -> bool;
  static auto packetizeBand(int perceID, int trackID, BandStream &bandStream,
                            std::vector<std::vector<bool>> &bitstreams, std::vector<int> &effectsId)
      -> bool;
  static auto createPayloadPacket(types::Band &band, StartTimeIdx &startTI,
                                  std::vector<types::Effect> &vecEffect, std::vector<int> &kfCount,
                                  bool &rau, std::vector<std::vector<bool>> &bitstream,
                                  std::vector<int> &effectsId) -> bool;
  static auto writePayloadPacket(std::vector<types::Effect> &vecEffect, BandStream &bandStream,
                                 std::vector<int> kfCount,
                                 std::vector<std::vector<bool>> bufPacketBitstream,
                                 std::vector<bool> &packetBits, int time) -> std::vector<bool>;
  static auto writeEffectHeader(StartTimeIdx point, StartTimeIdx percetrackID, bool &rau,
                                BandStream &bandStream) -> std::vector<bool>;
  static auto writeEffectBasis(types::Effect effect, types::BandType bandType,
                               StartTimeIdx &startTI, int &kfcount, bool &rau,
                               std::vector<bool> &bitstream) -> bool;
  static auto writeEffectTimeline(types::Effect &effect, std::vector<bool> &bitstream) -> bool;
  static auto writeEffectWavelet(types::Effect &effect, StartTimeIdx startTI,
                                 std::vector<bool> &bitstream) -> bool;
  static auto writeKeyframe(types::BandType bandType, types::Keyframe &keyframe,
                            std::vector<bool> &bitstream) -> bool;
  static auto writeTransient(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;
  static auto writeCurve(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;
  static auto writeVectorial(types::Keyframe &keyframe, std::vector<bool> &bitstream) -> bool;
  static auto writeTimeline(std::vector<types::Effect> &timeline, std::vector<bool> &bitstream)
      -> bool;
  /*static auto writeCRC(std::vector<bool> &bitstream) -> bool;
  static auto writeByteStuffing(int bits, std::vector<bool> &bitstream) -> bool;*/

  static auto sortPacket(std::vector<std::vector<std::vector<bool>>> &bandPacket,
                         std::vector<std::vector<bool>> &output) -> bool;

  static auto readPacketTS(std::vector<bool> bitstream) -> int;

  static auto readPacketLength(std::vector<bool> &bitstream) -> int;

  static auto readNALuType(std::vector<bool> &packet) -> NALuType;
  static auto readNALuHeader(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto readMetadataHaptics(types::Haptics &haptic, std::vector<bool> &bitstream) -> bool;
  static auto readAvatar(std::vector<bool> &bitstream, types::Avatar &avatar, int &length) -> bool;
  static auto readMetadataPerception(types::Perception &perception, std::vector<bool> &bitstream)
      -> bool;
  static auto readEffectsLibrary(std::vector<bool> &bitstream, std::vector<types::Effect> &effects)
      -> bool;
  static auto readReferenceDevice(std::vector<bool> &bitstream, types::ReferenceDevice &refDevice,
                                  int &length) -> bool;
  static auto readLibrary(types::Perception &perception, std::vector<bool> &bitstream) -> bool;
  static auto readLibraryEffect(types::Effect &libraryEffect, int &idx,
                                std::vector<bool> &bitstream) -> bool;
  static auto readMetadataTrack(types::Track &track, std::vector<bool> &bitstream) -> bool;
  static auto readMetadataBand(BandStream &bandStream, std::vector<bool> &bitstream) -> bool;
  static auto readData(types::Haptics &haptic, Buffer &buffer, std::vector<bool> &bitstream)
      -> bool;
  static auto readEffect(types::Effect &effect, std::vector<bool> &bitstream) -> bool;
  static auto readCRC(std::vector<bool> &crc, std::vector<bool> &bitstream) -> bool;

  static auto getEffectsId(types::Haptics &haptic) -> std::vector<int>;

  static auto readListObject(std::vector<bool> &bitstream, int avatarCount,
                             std::vector<types::Avatar> &avatarList) -> bool;
  static auto readListObject(std::vector<bool> &bitstream, int refDevCount,
                             std::vector<types::ReferenceDevice> &refDevList) -> bool;

  static auto addEffectToHaptic(types::Haptics &haptic, int perceptionIndex, int trackIndex,
                                int bandIndex, std::vector<types::Effect> &effects) -> bool;
  template <class T> static auto searchInList(std::vector<T> &list, T &item, int id) -> bool;
  static auto searchInList(std::vector<BandStream> &list, BandStream &item, int id) -> bool;
  static auto searchPerceptionInHaptic(types::Haptics &haptic, int id) -> int;
  static auto searchTrackInHaptic(types::Haptics &haptic, int id) -> int;
  static auto searchBandInHaptic(Buffer &buffer, int id) -> int;

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

  static auto readTimeline(std::vector<types::Effect> &timeline, std::vector<bool> &bitstream)
      -> bool;
  static auto checkHapticComponent(types::Haptics &haptic) -> void;

  static auto padToByteBoundary(std::vector<bool> &bitstream) -> void;

  static auto setNextEffectId(std::vector<int> &effectsId, types::Effect &effect) -> bool;

  static auto IOStream::addTimestampEffect(std::vector<types::Effect> &effects, int timestamp)
      -> bool;
};
} // namespace haptics::io
#endif // IOSTREAM_H