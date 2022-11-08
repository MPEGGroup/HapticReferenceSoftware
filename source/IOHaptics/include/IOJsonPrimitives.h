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

#ifndef IOJSONPRIMITIVES_H
#define IOJSONPRIMITIVES_H

#include <Types/include/Track.h>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996 26451 26495 26812 33010)
#endif
#include <rapidjson/document.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::io {

class IOJsonPrimitives {
public:
  [[nodiscard]] static auto
  hasInt(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject, const char *valueKey)
      -> bool;
  [[nodiscard]] static auto
  hasUint(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject, const char *valueKey)
      -> bool;
  [[nodiscard]] static auto
  hasString(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
            const char *valueKey) -> bool;
  [[nodiscard]] static auto
  hasNumber(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
            const char *valueKey) -> bool;
  [[nodiscard]] static auto
  hasArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject, const char *valueKey)
      -> bool;

  [[nodiscard]] static auto
  getStringArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
                 const char *valueKey, std::vector<std::string> &output) -> bool;
  [[nodiscard]] static auto
  getIntArray(const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject,
              const char *valueKey, std::vector<int> &output) -> bool;
  [[nodiscard]] static auto IOJsonPrimitives::getVectorArray(
      const rapidjson::GenericObject<true, rapidjson::Value> &jsonObject, const char *valueKey,
      std::vector<haptics::types::Vector> &output) -> bool;
};
} // namespace haptics::io
#endif // IOJSONPRIMITIVES_H
