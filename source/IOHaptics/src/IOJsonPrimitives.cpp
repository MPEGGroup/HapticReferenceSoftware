/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2022, ISO/IEC
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

#include <IOHaptics/include/IOJsonPrimitives.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::io {

[[nodiscard]] auto
IOJsonPrimitives::hasInt(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                         const char *valueKey) -> bool {
  return jsonObject.HasMember(valueKey) && jsonObject[valueKey].IsInt();
}

[[nodiscard]] auto
IOJsonPrimitives::hasUint(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                          const char *valueKey) -> bool {
  return jsonObject.HasMember(valueKey) && jsonObject[valueKey].IsUint();
}

[[nodiscard]] auto
IOJsonPrimitives::hasString(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                            const char *valueKey) -> bool {
  return jsonObject.HasMember(valueKey) && jsonObject[valueKey].IsString();
}

[[nodiscard]] auto
IOJsonPrimitives::hasNumber(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                            const char *valueKey) -> bool {
  return jsonObject.HasMember(valueKey) && jsonObject[valueKey].IsNumber();
}

[[nodiscard]] auto
IOJsonPrimitives::hasArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                           const char *valueKey) -> bool {
  return jsonObject.HasMember(valueKey) && jsonObject[valueKey].IsArray();
}

[[nodiscard]] auto IOJsonPrimitives::getVector(const rapidjson::Value &jsonValue,
                                               haptics::types::Vector &output) -> bool {
  if (!(jsonValue.IsObject() && jsonValue.HasMember("X") && jsonValue["X"].IsInt() &&
        jsonValue.HasMember("Y") && jsonValue["Y"].IsInt() && jsonValue.HasMember("Z") &&
        jsonValue["Z"].IsInt())) {
    return false;
  }

  output.X = static_cast<int8_t>(jsonValue["X"].GetInt());
  output.Y = static_cast<int8_t>(jsonValue["Y"].GetInt());
  output.Z = static_cast<int8_t>(jsonValue["Z"].GetInt());
  return true;
}

[[nodiscard]] auto
IOJsonPrimitives::getIntArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                              const char *valueKey, std::vector<int> &output) -> bool {
  if (!IOJsonPrimitives::hasArray(jsonObject, valueKey)) {
    return false;
  }
  for (const auto &value : jsonObject[valueKey].GetArray()) {
    if (value.IsInt()) {
      output.push_back(value.GetInt());
    }
  }
  return true;
}

[[nodiscard]] auto
IOJsonPrimitives::getStringArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                                 const char *valueKey, std::vector<std::string> &output) -> bool {
  if (!IOJsonPrimitives::hasArray(jsonObject, valueKey)) {
    return false;
  }
  for (const auto &value : jsonObject[valueKey].GetArray()) {
    if (value.IsString()) {
      output.emplace_back(value.GetString());
    }
  }
  return true;
}

[[nodiscard]] auto
IOJsonPrimitives::getVectorArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                                 const char *valueKey, std::vector<haptics::types::Vector> &output)
    -> bool {
  if (!IOJsonPrimitives::hasArray(jsonObject, valueKey)) {
    return false;
  }
  for (const auto &value : jsonObject[valueKey].GetArray()) {
    types::Vector vec{};
    if (IOJsonPrimitives::getVector(value, vec)) {
      output.push_back(vec);
    }
  }
  return true;
}

} // namespace haptics::io
