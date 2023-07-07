#ifndef IOCONFORMANCE_H
#define IOCONFORMANCE_H

#include <IOHaptics/include/IOStream.h>
#include <string>

namespace haptics::io {

class IOStream;
class StreamReader;
enum class MIHSUnitType;

enum class hmpgErrorCode {
  Init_MIHSDataPacket_UnvalidNumber,

  Init_Band_PerceptionID_Unknown,
  Init_Band_ChannelID_Unknown,
  Init_Band_BandType_OutofRange,
  Init_Band_CurveType_OutofRange,

  Init_EffectLibrary_PerceptionID_Unknown,
  Init_EffectLibrary_ID_NotUnique,
  Init_EffectLibrary_EffectType_OutofRange,
  Init_EffectLibrary_SemanticKeyword_Unvalid,
  Init_EffectLibrary_BaseSignal_OutofRange,

  TempSpat_MIHSDataPacket_UnvalidNumber,
  Temp_Duration_Unvalid,

  Temp_DataPacket_PerceptionModality_Unvalid,
  TempSpat_DataPacket_PerceptionID_Unvalid,
  TempSpat_DataPacket_ChannelID_Unvalid,
  TempSpat_DataPacket_BandID_Unvalid,
  TempSpat_DataPacket_EffectID_Unvalid,
  TempSpat_DataPacket_EffectType_OutofRange,

  Spat_DataPacket_PerceptionModality_Unvalid
};

static const std::map<hmpgErrorCode, std::string> hmpgErrorCodeToString = {
    {hmpgErrorCode::Init_Band_PerceptionID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Band error: Perception id does not exists. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_PerceptionID_Unknown))},
    {hmpgErrorCode::Init_Band_ChannelID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Band error: Perception id does not exists. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_ChannelID_Unknown))},
    {hmpgErrorCode::Init_Band_BandType_OutofRange,
     "MIHSUnit_Initialization MIHSPacket_Band error: band type is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_BandType_OutofRange))},
    {hmpgErrorCode::Init_Band_CurveType_OutofRange,
     "MIHSUnit_Initialization MIHSPacket_Band error: curve type is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_CurveType_OutofRange))},
    {hmpgErrorCode::Init_EffectLibrary_PerceptionID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: effect library id is unknown. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_PerceptionID_Unknown))},
    {hmpgErrorCode::Init_EffectLibrary_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: effect library id is not unique. "
     "Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_ID_NotUnique))},
    {hmpgErrorCode::Init_EffectLibrary_EffectType_OutofRange,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: effect type in effect library is not "
     "valid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_EffectType_OutofRange))},
    {hmpgErrorCode::Init_EffectLibrary_SemanticKeyword_Unvalid,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: effect semantic keyword in effect "
     "library is not valid. Error "
     "code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_EffectLibrary_SemanticKeyword_Unvalid))},
    {hmpgErrorCode::Init_EffectLibrary_BaseSignal_OutofRange,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: effect base signal in effect library "
     "is not valid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_BaseSignal_OutofRange))},
    {hmpgErrorCode::Init_MIHSDataPacket_UnvalidNumber,
     "MIHSUnit_Initialization MIHSPacket_Data error: No MIHSPacket_Data are allowed in "
     "MIHSUnit_Initialization. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_MIHSDataPacket_UnvalidNumber))},
    {hmpgErrorCode::TempSpat_MIHSDataPacket_UnvalidNumber,
     "MIHSUnit_Temporal MIHSPacket_Data error: Incorrect number of MIHSData packets, one or more "
     "MIHSPacket_Data expected. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_MIHSDataPacket_UnvalidNumber))},
    {hmpgErrorCode::Temp_Duration_Unvalid,
     "MIHSUnit_Temporal error: Incorrect MIHSUnit duration. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Duration_Unvalid))},
    {hmpgErrorCode::Temp_DataPacket_PerceptionModality_Unvalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Perception modality unvalid for temporal MIHSUnit. "
     "Error "
     "code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Temp_DataPacket_PerceptionModality_Unvalid))},
    {hmpgErrorCode::TempSpat_DataPacket_PerceptionID_Unvalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Perception id is not valid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_DataPacket_PerceptionID_Unvalid))},
    {hmpgErrorCode::TempSpat_DataPacket_ChannelID_Unvalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Channel id is not valid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_DataPacket_ChannelID_Unvalid))},
    {hmpgErrorCode::TempSpat_DataPacket_BandID_Unvalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Band id is not valid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_DataPacket_BandID_Unvalid))},
    {hmpgErrorCode::TempSpat_DataPacket_EffectID_Unvalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Band id is not valid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_DataPacket_EffectID_Unvalid))},
    {hmpgErrorCode::TempSpat_DataPacket_EffectType_OutofRange,
     "MIHSUnit_Temporal MIHSPacket_Data error: effect type is is out of range. Error "
     "code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::TempSpat_DataPacket_EffectType_OutofRange))},
    {hmpgErrorCode::Spat_DataPacket_PerceptionModality_Unvalid,
     "MIHSUnit_Spatial MIHSPacket_Data error: Perception modality unvalid for spatial MIHSUnit. "
     "Error "
     "code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Spat_DataPacket_PerceptionModality_Unvalid))}};

static class IOConformance {
public:
  static auto checkBandTypeRange(IOStream::StreamReader &sreader) -> void {
    BandType btype = sreader.bandStream.band.getBandType();
    if (btype < BandType::Transient && btype > BandType::WaveletWave) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_BandType_OutofRange));
    }
  };

  static auto checkCurveTypeRange(IOStream::StreamReader &sreader) -> void {
    types::CurveType ctype = sreader.bandStream.band.getCurveType();
    if (ctype < types::CurveType::Unknown && ctype > types::CurveType::Bspline) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_CurveType_OutofRange));
    }
  };

  static auto checkEffectLibrary(IOStream::StreamReader &sreader, types::Perception &perce)
      -> void {
    std::vector<int> effectsID = std::vector<int>();
    for (int i = 0; i < perce.getEffectLibrarySize(); i++) {
      Effect e = perce.getBasisEffectAt(i);
      int eId = e.getId();
      if (std::find(effectsID.begin(), effectsID.end(), eId) != effectsID.end()) {
        sreader.logs.push_back(
            hmpgErrorCodeToString.at(hmpgErrorCode::Init_EffectLibrary_ID_NotUnique));
      }
      effectsID.push_back(eId);
    }
  };

  static auto checkMIHSUnitDataPacket(IOStream::StreamReader &sreader) -> void {
    if (sreader.currentUnitType == MIHSUnitType::Spatial ||
        sreader.currentUnitType == MIHSUnitType::Temporal) {
      if (!sreader.MIHSData) {
        sreader.logs.push_back(
            hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_MIHSDataPacket_UnvalidNumber));
      }
    }
  }

  static auto checkMIHSUnitTemporalDuraction(IOStream::StreamReader &sreader) -> void {
    if (sreader.packetDuration <= 0) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Duration_Unvalid));
    }
  }

  static auto checkMIHSUnitTemporalPerceptionModality(IOStream::StreamReader &sreader) -> void {
    types::PerceptionModality modality = sreader.perception.getPerceptionModality();
    if (modality == types::PerceptionModality::VibrotactileTexture ||
        modality == types::PerceptionModality::Stiffness ||
        modality == types::PerceptionModality::Friction) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Temp_DataPacket_PerceptionModality_Unvalid));
    }
  }

  static auto checkMIHSUnitSpatialPerceptionModality(IOStream::StreamReader &sreader) -> void {
    types::PerceptionModality modality = sreader.perception.getPerceptionModality();
    if (modality == types::PerceptionModality::Pressure ||
        modality == types::PerceptionModality::Acceleration ||
        modality == types::PerceptionModality::Velocity ||
        modality == types::PerceptionModality::Position ||
        modality == types::PerceptionModality::Temperature ||
        modality == types::PerceptionModality::Vibrotactile ||
        modality == types::PerceptionModality::Water ||
        modality == types::PerceptionModality::Wind ||
        modality == types::PerceptionModality::Force ||
        modality == types::PerceptionModality::Electrotactile ||
        modality == types::PerceptionModality::Other) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Spat_DataPacket_PerceptionModality_Unvalid));
    }
  }

  static auto checkEffectIDExists(IOStream::StreamReader &sreader, int effectId) -> void {
    for (int i = 0; i < sreader.perception.getEffectLibrarySize(); i++) {
      if (sreader.perception.getBasisEffectAt(i).getId() == effectId) {
        return;
      }
    }
    sreader.logs.push_back(
        hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_DataPacket_EffectID_Unvalid));
  }
};
} // namespace haptics::io
#endif