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

#ifndef AVATAR_H
#define AVATAR_H

#include <map>
#include <string>
#include <optional>

namespace haptics::types {
enum class AvatarType {
  Vibration = 1,
  Pressure = 2,
  Temperature = 3,
  Custom = 0,
};

static const std::map<std::string, AvatarType> stringToAvatarType = {
    {"Vibration", AvatarType::Vibration},
    {"Pressure", AvatarType::Pressure},
    {"Temperature", AvatarType::Temperature},
    {"Custom", AvatarType::Custom}};
static const std::map<AvatarType, std::string> avatarTypeToString = {
    {AvatarType::Vibration, "Vibration"},
    {AvatarType::Pressure, "Pressure"},
    {AvatarType::Temperature, "Temperature"},
    {AvatarType::Custom, "Custom"}};

class Avatar {
public:
  explicit Avatar() = default;
  explicit Avatar(int newId, int newLod, AvatarType newType)
      : id(newId), lod(newLod), type(newType){};

  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getLod() const -> int;
  auto setLod(int newLod) -> void;
  [[nodiscard]] auto getType() const -> AvatarType;
  auto setType(AvatarType newType) -> void;
  [[nodiscard]] auto getMesh() const -> std::optional<std::string>;
  auto setMesh(const std::string &newMesh) -> void;

private:
  int id = -1;
  int lod = 0;
  AvatarType type = AvatarType::Custom;
  std::optional<std::string> mesh;
};
} // namespace haptics::types
#endif // AVATAR_H
