#ifndef IOCOMPATIBILITY_H
#define IOCOMPATIBILITY_H

//#include <IOHaptics/include/IOStream.h>
#include <Types/include/Haptics.h>
//#include <climits>
#include <map>
#include <string>
#include <vector>

namespace haptics::io {

using namespace haptics::types;

static constexpr size_t MAX_8_BITS = 255;
static constexpr size_t MAX_31_BITS = 4294967295;

enum class hjifErrorCode {
  Experience_Description_Size_OutOfRange,
  Experience_Timescale_OutOfRange,
  Experience_Avatars_Size_OutOfRange,
  Experience_Perceptions_Size_OutOfRange
};

static const std::map<hjifErrorCode, std::string> hjifErrorCodeToString = {
    // Init_Experience_*
    {hjifErrorCode::Experience_Description_Size_OutOfRange,
     "Experience Error: description too long. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Description_Size_OutOfRange))},
    {hjifErrorCode::Experience_Timescale_OutOfRange,
     "Experience Error: timescale out of bounds. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Timescale_OutOfRange))},
    {hjifErrorCode::Experience_Avatars_Size_OutOfRange,
     "Experience Error: avatars vector size out of bounds. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Avatars_Size_OutOfRange))},
    {hjifErrorCode::Experience_Perceptions_Size_OutOfRange,
     "Experience Error: perceptions vector size out of bounds. Error code: " +
         std::to_string(static_cast<int>(hjifErrorCode::Experience_Perceptions_Size_OutOfRange))}};

class IOCompatibility {
public:
  static auto checkExperience(types::Haptics &haptic, std::vector<std::string> &logs) -> void {

    if (haptic.getDescription().size() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Experience_Description_Size_OutOfRange));
    }

    if (haptic.getTimescaleOrDefault() > MAX_31_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Experience_Timescale_OutOfRange));
    }

    if (haptic.getAvatarsSize() > MAX_8_BITS) {
      logs.push_back(hjifErrorCodeToString.at(hjifErrorCode::Experience_Avatars_Size_OutOfRange));
    }

    if (haptic.getPerceptionsSize() > MAX_8_BITS) {
      logs.push_back(
          hjifErrorCodeToString.at(hjifErrorCode::Experience_Perceptions_Size_OutOfRange));
    }
  };
};
} // namespace haptics::io
#endif