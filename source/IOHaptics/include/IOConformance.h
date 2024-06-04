#ifndef IOCONFORMANCE_H
#define IOCONFORMANCE_H

#include <IOHaptics/include/IOStream.h>
#include <Types/include/EffectSemantic.h>
#include <climits>
#include <string>

namespace haptics::io {

constexpr int LOWER_RANGE_UNIT_VECTOR = 15692;
constexpr int UPPER_RANGE_UNIT_VECTOR = 16572;
class IOStream;
class StreamReader;
enum class MIHSUnitType;

enum class hmpgErrorCode {
  Init_Experience_UnitType_Invalid,
  Init_Experience_FirstUnitType_Invalid,
  Init_Experience_UnitSync_Invalid,
  Init_Experience_InitUnitSync_Invalid,
  Init_Experience_InitUnitTiming_Invalid,
  Init_Experience_Profile_Invalid,
  Init_Experience_Level_Invalid,
  Init_Experience_Version_Invalid,
  Init_Experience_Date_Invalid,
  Init_Experience_Avatar_ID_OutOfRange,
  Init_Experience_Avatar_MeshURI_Invalid,

  Init_Packet_Type_Invalid,

  Init_Perception_ID_NotUnique,
  Init_Perception_Modality_OutOfRange,
  Init_Perception_Modality_NotSupportedByLevelProfile,
  Init_Perception_AvatarID_Unkown,
  Init_Perception_SchemeURN_Invalid,
  Init_Perception_ReferenceDevice_ID_OutOfRange,
  Init_Perception_ReferenceDevice_ID_NotUnique,
  Init_Perception_ReferenceDevice_Type_OutOfRange,

  Init_Channel_ID_NotUnique,
  Init_Channel_OutOfRange,
  Init_Channel_PerceptionID_Unknown,
  Init_Channel_ReferenceDeviceID_Unknown,
  Init_Channel_ActuatorResolution_OutOfRange,
  Init_Channel_BodyPartTarget_OutOfRange,
  Init_Channel_ActuatorTarget_OutOfRange,
  Init_Channel_Direction_Invalid,

  Init_Band_ID_NotUnique,
  Init_Band_PerceptionID_Unknown,
  Init_Band_ChannelID_Unknown,
  Init_Band_OutOfRange,
  Init_Band_BandType_OutOfRange,
  Init_Band_BandType_NotSupportedByLevelProfile,
  Init_Band_CurveType_OutOfRange,
  Init_Band_TimescaleInvalidWithLevelProfile,

  Init_EffectLibrary_ID_NotUnique,
  Init_EffectLibrary_PerceptionID_Unknown,
  Init_EffectLibrary_EffectType_OutOfRange,
  Init_EffectLibrary_SemanticKeyword_Invalid,
  Init_EffectLibrary_BaseSignal_OutOfRange,

  NonTempSpat_Data_InvalidNumber,

  NonInit_Timing_InvalidNumber,
  NonInit_Experience_InvalidNumber,
  NonInit_Perception_InvalidNumber,
  NonInit_Channel_InvalidNumber,
  NonInit_Band_InvalidNumber,
  NonInit_EffectLibrary_InvalidNumber,

  TempSpat_Data_InvalidNumber,
  TempSpat_Data_PerceptionID_Unknown,
  TempSpat_Data_ChannelID_Unknown,
  TempSpat_Data_BandID_Unknown,
  TempSpat_Data_EffectID_Unknown,
  TempSpat_Data_EffectType_Unknown,
  TempSpat_Data_EffectType_OutOfRange,
  TempSpat_Data_Semantic_Unknown,
  TempSpat_Data_EffectType_CompositeNotSupportedByProfile,

  Temp_Timing_Not_Ascending,

  Temp_Data_MIHSPacket_Invalid,
  Temp_Data_PerceptionModality_Invalid,

  Temp_InitDuration_Invalid,
  Temp_Duration_Invalid,
  Temp_Position_Invalid,
  Temp_BaseSignal_Invalid,

  Spat_Data_PerceptionModality_Invalid,
  Spat_Duration_Invalid,
  Spat_No_Packets,
  Spat_UnitSync_Invalid,
  Spat_Data_MIHSPacket_Invalid,
  Spat_Data_PerceptionModality_Invalid,

  Sile_InitDuration_Invalid,
  Sile_Data_Invalid,
  Silent_Duration_Invalid
};

static const std::map<hmpgErrorCode, std::string> hmpgErrorCodeToString = {
    // Init_Experience_*
    {hmpgErrorCode::Init_Experience_UnitType_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Invalid MIHS unit type. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_UnitType_Invalid))},
    {hmpgErrorCode::Init_Experience_FirstUnitType_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: The first MIHS unit in a haptic stream "
     "shall be an initialization unit. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_FirstUnitType_Invalid))},
    {hmpgErrorCode::Init_Experience_UnitSync_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Invalid MIHS unit sync value. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_UnitSync_Invalid))},
    {hmpgErrorCode::Init_Experience_InitUnitSync_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: An initialization unit is a sync unit. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_InitUnitSync_Invalid))},
    {hmpgErrorCode::Init_Experience_InitUnitTiming_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: An initialization unit shall contain "
     "one timing MIHS packet. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_InitUnitTiming_Invalid))},
    {hmpgErrorCode::Init_Experience_Profile_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Profile not defined in the "
     "specifications. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Profile_Invalid))},
    {hmpgErrorCode::Init_Experience_Level_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Level not defined in the "
     "specifications. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Level_Invalid))},
    {hmpgErrorCode::Init_Experience_Date_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Date shall follow the ISO 8601 "
     "standard. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Date_Invalid))},
    {hmpgErrorCode::Init_Experience_Version_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Version should follow the format: XXXX "
     "or XXXX-Y. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Version_Invalid))},
    {hmpgErrorCode::Init_Experience_Avatar_ID_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Avatar id is out of range. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Avatar_ID_OutOfRange))},
    {hmpgErrorCode::Init_Experience_Avatar_MeshURI_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Experience error: Mesh URI is invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Experience_Avatar_MeshURI_Invalid))},

    // Init_Packet_*
    {hmpgErrorCode::Init_Packet_Type_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Metadata error: Packet type is invalid. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Packet_Type_Invalid))},

    // Init_Perception_*
    {hmpgErrorCode::Init_Perception_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Perception ID is not unique. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Perception_ID_NotUnique))},
    {hmpgErrorCode::Init_Perception_Modality_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Modality is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Perception_Modality_OutOfRange))},
    {hmpgErrorCode::Init_Perception_Modality_NotSupportedByLevelProfile,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Modality is not supported by the "
     "level/profile used. Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Perception_Modality_NotSupportedByLevelProfile))},
    {hmpgErrorCode::Init_Perception_AvatarID_Unkown,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Avatar ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Perception_AvatarID_Unkown))},
    {hmpgErrorCode::Init_Perception_SchemeURN_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Scheme URN is invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Perception_SchemeURN_Invalid))},
    {hmpgErrorCode::Init_Perception_ReferenceDevice_ID_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Reference device ID is out of range. "
     "Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Perception_ReferenceDevice_ID_OutOfRange))},
    {hmpgErrorCode::Init_Perception_ReferenceDevice_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Reference device ID is not unique. "
     "Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Perception_ReferenceDevice_ID_NotUnique))},
    {hmpgErrorCode::Init_Perception_ReferenceDevice_Type_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Perception error: Reference device type is out of range. "
     "Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Perception_ReferenceDevice_Type_OutOfRange))},

    // Init_Channel_*
    {hmpgErrorCode::Init_Channel_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Channel ID is not unique. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_ID_NotUnique))},
    {hmpgErrorCode::Init_Channel_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Channel error: The Channel count is out of range. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_OutOfRange))},
    {hmpgErrorCode::Init_Channel_PerceptionID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Perception ID does not exist. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_PerceptionID_Unknown))},
    {hmpgErrorCode::Init_Channel_ReferenceDeviceID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Reference device ID does not exist. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_ReferenceDeviceID_Unknown))},
    {hmpgErrorCode::Init_Channel_ActuatorResolution_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Actuator resolution is out of range, "
     "positive value expected. Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Channel_ActuatorResolution_OutOfRange))},
    {hmpgErrorCode::Init_Channel_BodyPartTarget_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Body part target component is out of "
     "range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_BodyPartTarget_OutOfRange))},
    {hmpgErrorCode::Init_Channel_ActuatorTarget_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Actuator target component is out of range. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_ActuatorTarget_OutOfRange))},
    {hmpgErrorCode::Init_Channel_Direction_Invalid,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Direction is invalid, unit vector "
     "expected. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Channel_Direction_Invalid))},

    // Init_Band_*
    {hmpgErrorCode::Init_Band_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_Channel error: Band ID is not unique. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_ID_NotUnique))},
    {hmpgErrorCode::Init_Band_PerceptionID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Band error: Perception ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_PerceptionID_Unknown))},
    {hmpgErrorCode::Init_Band_ChannelID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_Band error: Channel ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_ChannelID_Unknown))},
    {hmpgErrorCode::Init_Band_BandType_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Band error: Band type is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_BandType_OutOfRange))},
    {hmpgErrorCode::Init_Band_BandType_NotSupportedByLevelProfile,
     "MIHSUnit_Initialization MIHSPacket_Band error: Band type is not supported by the "
     "level/profile used. Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Band_BandType_NotSupportedByLevelProfile))},
    {hmpgErrorCode::Init_Band_CurveType_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_Band error: Curve type is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_Band_CurveType_OutOfRange))},
    {hmpgErrorCode::Init_Band_TimescaleInvalidWithLevelProfile,
     "MIHSUnit_Initialization MIHSPacket_Band error: Timescale is not supported by the bandtype "
     "and level/profile used. Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_Band_TimescaleInvalidWithLevelProfile))},

    // Init_EffectLibrary_*
    {hmpgErrorCode::Init_EffectLibrary_PerceptionID_Unknown,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: Perception ID is unknown. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_PerceptionID_Unknown))},
    {hmpgErrorCode::Init_EffectLibrary_ID_NotUnique,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: Effect ID is not unique. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_ID_NotUnique))},
    {hmpgErrorCode::Init_EffectLibrary_EffectType_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: Effect type is out of range. Error "
     "code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_EffectType_OutOfRange))},
    {hmpgErrorCode::Init_EffectLibrary_SemanticKeyword_Invalid,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: Effect semantic keyword is invalid. "
     "Error code: " +
         std::to_string(
             static_cast<int>(hmpgErrorCode::Init_EffectLibrary_SemanticKeyword_Invalid))},
    {hmpgErrorCode::Init_EffectLibrary_BaseSignal_OutOfRange,
     "MIHSUnit_Initialization MIHSPacket_EffectLibrary error: Effect base signal is out of range. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Init_EffectLibrary_BaseSignal_OutOfRange))},

    // NonInit_*
    {hmpgErrorCode::NonInit_Timing_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Timing error: MIHSPacket_Timing allowed only in "
     "MIHSUnit_Initialization. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_Timing_InvalidNumber))},
    {hmpgErrorCode::NonInit_Experience_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Experience error: MIHSPacket_Experience allowed only in "
     "MIHSUnit_Initialization. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_Experience_InvalidNumber))},
    {hmpgErrorCode::NonInit_Perception_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Perception error: MIHSPacket_Perception allowed only in "
     "MIHSUnit_Initialization. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_Perception_InvalidNumber))},
    {hmpgErrorCode::NonInit_Channel_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Channel error: No MIHSPacket_Channel allowed only in "
     "MIHSUnit_Initialization. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_Channel_InvalidNumber))},
    {hmpgErrorCode::NonInit_Band_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Band error: MIHSPacket_Band allowed only in MIHSUnit_Initialization. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_Band_InvalidNumber))},
    {hmpgErrorCode::NonInit_EffectLibrary_InvalidNumber,
     "MIHSUnit_* MIHSPacket_EffectLibrary error: MIHSPacket_EffectLibrary allowed only in "
     "MIHSUnit_Initialization. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonInit_EffectLibrary_InvalidNumber))},

    // TempSpat_Data_*
    {hmpgErrorCode::TempSpat_Data_InvalidNumber,
     "MIHSUnit_Temporal MIHSPacket_Data error: Incorrect number of MIHSData packets, one or more "
     "MIHSPacket_Data expected. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_InvalidNumber))},
    {hmpgErrorCode::TempSpat_Data_PerceptionID_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Perception ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_PerceptionID_Unknown))},
    {hmpgErrorCode::TempSpat_Data_ChannelID_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Channel ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_ChannelID_Unknown))},
    {hmpgErrorCode::TempSpat_Data_BandID_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Band ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_BandID_Unknown))},
    {hmpgErrorCode::TempSpat_Data_EffectID_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Effect ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_EffectID_Unknown))},
    {hmpgErrorCode::TempSpat_Data_EffectType_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Effect Type does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_EffectType_Unknown))},
    {hmpgErrorCode::TempSpat_Data_EffectType_OutOfRange,
     "MIHSUnit_Temporal MIHSPacket_Data error: Effect type is out of range. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_EffectType_OutOfRange))},
    {hmpgErrorCode::TempSpat_Data_EffectType_CompositeNotSupportedByProfile,
     "MIHSUnit_Temporal MIHSPacket_Data error: Composite effects are not supported with the "
     "profile used. Error code: " +
         std::to_string(static_cast<int>(
             hmpgErrorCode::TempSpat_Data_EffectType_CompositeNotSupportedByProfile))},
    {hmpgErrorCode::TempSpat_Data_Semantic_Unknown,
     "MIHSUnit_Temporal MIHSPacket_Data error: Semantic does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::TempSpat_Data_Semantic_Unknown))},

    // NonTempSpat_*
    {hmpgErrorCode::NonTempSpat_Data_InvalidNumber,
     "MIHSUnit_* MIHSPacket_Data error: MIHSPacket_Data allowed only in MIHSUnit_Temporal or "
     "MIHSUnit_Spatial. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::NonTempSpat_Data_InvalidNumber))},

    // Temp_Data_*
    {hmpgErrorCode::Temp_Data_MIHSPacket_Invalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: A spatial unit shall contain one or more MIHS "
     "packets. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Data_MIHSPacket_Invalid))},
    {hmpgErrorCode::Temp_Timing_Not_Ascending,
     "MIHSUnit_Temporal MIHSPacket_Data error: Effect ID does not exist. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Timing_Not_Ascending))},
    {hmpgErrorCode::Temp_Data_PerceptionModality_Invalid,
     "MIHSUnit_Temporal MIHSPacket_Data error: Perception modality invalid for temporal MIHSUnit. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Data_PerceptionModality_Invalid))},

    // Other Temp_*
    {hmpgErrorCode::Temp_InitDuration_Invalid,
     "MIHSUnit_Temporal error: The duration of an initialization unit duration shall be zero. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_InitDuration_Invalid))},
    {hmpgErrorCode::Temp_Duration_Invalid,
     "MIHSUnit_Temporal error: Duration invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Duration_Invalid))},
    {hmpgErrorCode::Temp_Position_Invalid,
     "MIHSUnit_Temporal error: Position invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_Position_Invalid))},
    {hmpgErrorCode::Temp_BaseSignal_Invalid,
     "MIHSUnit_Temporal error: baseSignal invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Temp_BaseSignal_Invalid))},

    // Spat_Data_*
    {hmpgErrorCode::Spat_Data_MIHSPacket_Invalid,
     "MIHSUnit_Spatial MIHSPacket_Data error: A spatial unit shall contain one or more MIHS "
     "packets. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Spat_Data_MIHSPacket_Invalid))},
    {hmpgErrorCode::Spat_Data_PerceptionModality_Invalid,
     "MIHSUnit_Spatial MIHSPacket_Data error: Perception modality invalid for spatial MIHSUnit. "
     "Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Spat_Data_PerceptionModality_Invalid))},
    {hmpgErrorCode::Spat_Duration_Invalid,
     "MIHSUnit_Spatial error: Duration invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Spat_Duration_Invalid))},
    {hmpgErrorCode::Spat_No_Packets,
     "MIHSUnit_Spatial error: No packets in unit. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Spat_No_Packets))},
    {hmpgErrorCode::Spat_UnitSync_Invalid,
     "MIHSUnit_Spatial error: UnitSync value invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Spat_UnitSync_Invalid))},

    // Sile_Data_*
    {hmpgErrorCode::Sile_InitDuration_Invalid,
     "MIHSUnit_Silent MIHSPacket_Data error: The duration of a silent MIHS unit shall be a "
     "positive number. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Sile_InitDuration_Invalid))},
    {hmpgErrorCode::Sile_Data_Invalid,
     "MIHSUnit_Silent MIHSPacket_Data error: A Silent MIHS unit shall not include any MIHS packets "
     "other than MIHS packets of type PACTYPE_TIMING. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Sile_InitDuration_Invalid))},
    {hmpgErrorCode::Silent_Duration_Invalid,
     "MIHSUnit_Silent error: Duration invalid. Error code: " +
         std::to_string(static_cast<int>(hmpgErrorCode::Silent_Duration_Invalid))}};

class IOConformance {
public:
  static auto checkBandTypeRange(IOStream::StreamReader &sreader) -> void {
    auto btype = sreader.bandStream.band.getBandType();
    if (btype < types::BandType::Transient || btype > types::BandType::WaveletWave) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_BandType_OutOfRange));
    }

    if (strcmp(sreader.haptic.getProfile().c_str(), SIMPLE_PARAMETRIC_PROFILE) == 0) {
      if (btype != types::BandType::Transient && btype != types::BandType::Curve &&
          btype != types::BandType::VectorialWave) {
        sreader.logs.push_back(
            hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_BandType_NotSupportedByLevelProfile));
      }
    }
  };

  static auto checkCurveTypeRange(IOStream::StreamReader &sreader) -> void {
    auto ctype = sreader.bandStream.band.getCurveTypeOrDefault();
    if (ctype < types::CurveType::Unknown || ctype > types::CurveType::Bspline) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_CurveType_OutOfRange));
    }
  };

  static auto checkTimescaleForBandType(IOStream::StreamReader &sreader) -> void {
    auto btype = sreader.bandStream.band.getBandType();
    if (strcmp(sreader.haptic.getProfile().c_str(), MAIN_PROFILE) == 0 &&
        btype == types::BandType::WaveletWave) {
      return;
    }

    if (sreader.timescale != haptics::types::Haptics::DEFAULT_TIMESCALE) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Band_TimescaleInvalidWithLevelProfile));
    }
  };

  static auto checkEffectLibrary(IOStream::StreamReader &sreader, types::Perception &perce)
      -> void {
    std::vector<int> effectsID = std::vector<int>();
    for (unsigned int i = 0; i < perce.getEffectLibrarySize(); i++) {
      auto e = perce.getBasisEffectAt(static_cast<int>(i));
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
            hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_Data_InvalidNumber));
      }
    } else if (sreader.currentUnitType == MIHSUnitType::Silent) {
      if (sreader.MIHSData) {
        sreader.logs.push_back(
            hmpgErrorCodeToString.at(hmpgErrorCode::NonTempSpat_Data_InvalidNumber));
      }
    }
  }

  static auto checkFirstMIHSUnitType(IOStream::StreamReader &sreader, std::vector<bool> &mihsunit)
      -> void {
    int index = 0;
    int unitType = IOBinaryPrimitives::readUInt(mihsunit, index, UNIT_TYPE);
    if (unitType == static_cast<int>(MIHSUnitType::Initialization)) {
      return;
    }

    sreader.logs.push_back(
        hmpgErrorCodeToString.at(hmpgErrorCode::Init_Experience_FirstUnitType_Invalid));
  }

  static auto checkMIHSUnitType(IOStream::StreamReader &sreader, const int unitType) -> bool {
    if (unitType == static_cast<int>(MIHSUnitType::Initialization) ||
        unitType == static_cast<int>(MIHSUnitType::Temporal) ||
        unitType == static_cast<int>(MIHSUnitType::Spatial) ||
        unitType == static_cast<int>(MIHSUnitType::Silent)) {
      return true;
    }

    if (sreader.conformance) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Experience_UnitType_Invalid));
    }
    return false;
  }

  static auto checkNALuType(IOStream::StreamReader &sreader, const int naluType) -> void {
    if (naluType == static_cast<int>(MIHSPacketType::Timing) ||
        naluType == static_cast<int>(MIHSPacketType::InitializationTiming) ||
        naluType == static_cast<int>(MIHSPacketType::MetadataHaptics) ||
        naluType == static_cast<int>(MIHSPacketType::MetadataPerception) ||
        naluType == static_cast<int>(MIHSPacketType::MetadataChannel) ||
        naluType == static_cast<int>(MIHSPacketType::MetadataBand) ||
        naluType == static_cast<int>(MIHSPacketType::Data) ||
        naluType == static_cast<int>(MIHSPacketType::EffectLibrary) ||
        naluType == static_cast<int>(MIHSPacketType::CRC32) ||
        naluType == static_cast<int>(MIHSPacketType::CRC16) ||
        naluType == static_cast<int>(MIHSPacketType::GlobalCRC32) ||
        naluType == static_cast<int>(MIHSPacketType::GlobalCRC16)) {
      return;
    }

    sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Init_Packet_Type_Invalid));
  }

  static auto checkMIHSUnitSync(IOStream::StreamReader &sreader, const int unitSync) -> void {
    if (unitSync == 0 || unitSync == 1) {
      return;
    }

    sreader.logs.push_back(
        hmpgErrorCodeToString.at(hmpgErrorCode::Init_Experience_UnitSync_Invalid));
  }

  static auto checkMIHSUnitSyncWhenInit(IOStream::StreamReader &sreader, const bool unitSync)
      -> void {
    if (sreader.currentUnitType == MIHSUnitType::Initialization && !unitSync) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Init_Experience_InitUnitSync_Invalid));
    }
  }

  static auto checkMIHSUnitInitializationDuraction(IOStream::StreamReader &sreader) -> void {
    if (sreader.packetDuration != 0) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_InitDuration_Invalid));
    }
  }

  static auto checkMIHSUnitDuration(IOStream::StreamReader &sreader) -> void {
    if (sreader.packetDuration <= 0) {
      if (sreader.currentUnitType == MIHSUnitType::Spatial) {
        if (sreader.packetDuration != 0) {
          sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Spat_Duration_Invalid));
        }
      } else if (sreader.currentUnitType == MIHSUnitType::Silent) {
        sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Silent_Duration_Invalid));
      } else {
        sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Duration_Invalid));
      }
    }
  }

  static auto checkEffectPosition(IOStream::StreamReader &sreader, int effectPos) -> void {
    if (effectPos < 0 && (!sreader.waitSync || !(sreader.auType == AUType::DAU))) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Position_Invalid));
    }
    if (effectPos >= static_cast<int>(sreader.packetDuration)) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Position_Invalid));
    }
  }

  static auto checkBaseSignal(IOStream::StreamReader &sreader, int baseSignal) -> void {
    if (!(baseSignal >= 0 && baseSignal <= static_cast<int>(types::BaseSignal::SawToothDown))) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_BaseSignal_Invalid));
    }
  }

  static auto checkMIHSUnitTemporalPerceptionModality(IOStream::StreamReader &sreader) -> void {
    types::PerceptionModality modality = sreader.perception.getPerceptionModality();
    if (modality == types::PerceptionModality::VibrotactileTexture ||
        modality == types::PerceptionModality::Stiffness ||
        modality == types::PerceptionModality::Friction) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Data_PerceptionModality_Invalid));
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
          hmpgErrorCodeToString.at(hmpgErrorCode::Spat_Data_PerceptionModality_Invalid));
    }
  }

  static auto checkEffectIDExists(IOStream::StreamReader &sreader, int effectId) -> void {
    for (unsigned int i = 0; i < sreader.perception.getEffectLibrarySize(); i++) {
      if (sreader.perception.getBasisEffectAt(static_cast<int>(i)).getId() == effectId) {
        return;
      }
    }
    sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_Data_EffectID_Unknown));
  }

  static auto checkEffectTypeUnknown(IOStream::StreamReader &sreader, int effectType) -> void {
    if (effectType < 0 || effectType > 1) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_Data_EffectType_Unknown));
    }
  }

  static auto checkSemanticUnknown(IOStream::StreamReader &sreader, const std::string &semantic)
      -> void {
    if (types::stringToEffectSemantic.find(semantic) == types::stringToEffectSemantic.end()) {
      sreader.logs.push_back(
          hmpgErrorCodeToString.at(hmpgErrorCode::TempSpat_Data_Semantic_Unknown));
    }
  }

  static auto checkEffectOrder(IOStream::StreamReader &sreader, std::vector<types::Effect> &effects)
      -> void {
    int pos = INT_MIN;
    for (const auto &effect : effects) {
      if (effect.getPosition() < pos) {
        sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Temp_Timing_Not_Ascending));
      }
      pos = effect.getPosition();
    }
  }

  static auto checkMIHSUnitSpatialPackets(IOStream::StreamReader &sreader,
                                          const std::vector<bool> &packets) -> void {
    if (packets.empty()) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Spat_No_Packets));
    }

    if (sreader.waitSync) {
      sreader.logs.push_back(hmpgErrorCodeToString.at(hmpgErrorCode::Spat_UnitSync_Invalid));
    }
  }
};
} // namespace haptics::io
#endif