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

#ifndef HAPTICS_H
#define HAPTICS_H

#include <Tools/include/OHMData.h>
#include <Types/include/Avatar.h>
#include <Types/include/Perception.h>
#include <Types/include/Sync.h>
#include <fstream>
#include <optional>
#include <vector>

namespace haptics::types {

class Haptics {
public:
  static constexpr unsigned int DEFAULT_TIMESCALE = 1000;

  explicit Haptics() = default;
  explicit Haptics(std::string newVersion, std::string newDate, std::string newDescription)
      : version(std::move(newVersion))
      , date(std::move(newDate))
      , description(std::move(newDescription))
      , perceptions({})
      , avatars({})
      , syncs({}){};

  [[nodiscard]] auto getVersion() const -> std::string;
  auto setVersion(std::string &newVersion) -> void;

  [[nodiscard]] auto getProfile() const -> std::string;
  auto setProfile(std::string &newProfile) -> void;
  [[nodiscard]] auto getLevel() const -> uint8_t;
  auto setLevel(uint8_t newLevel) -> void;

  [[nodiscard]] auto getDate() const -> std::string;
  auto setDate(std::string &newDate) -> void;
  [[nodiscard]] auto getDescription() const -> std::string;
  auto setDescription(std::string &newDescription) -> void;
  [[nodiscard]] auto getTimescale() const -> std::optional<unsigned int>;
  [[nodiscard]] auto getTimescaleOrDefault() const -> unsigned int;
  auto setTimescale(std::optional<unsigned int> newTimescale) -> void;
  auto getPerceptionsSize() -> size_t;
  auto getPerceptionAt(int index) -> Perception &;
  auto replacePerceptionAt(int index, Perception &newPerception) -> bool;
  auto removePerceptionAt(int index) -> bool;
  auto addPerception(Perception &newPerception) -> void;
  auto getAvatarsSize() -> size_t;
  auto getAvatarAt(int index) -> Avatar &;
  auto addAvatar(Avatar &newAvatar) -> void;
  [[nodiscard]] auto getTimescaleOrDefault() const -> unsigned int;
  [[nodiscard]] auto getTimescale() const -> std::optional<unsigned int>;
  auto setTimescale(std::optional<unsigned int> newTimescale) -> void;
  auto getSyncsSize() -> size_t;
  auto getSyncsAt(int index) -> Sync &;
  auto addSync(Sync &newSync) -> void;
  auto loadMetadataFromOHM(haptics::tools::OHMData data) -> void;
  auto extractMetadataToOHM(std::string &filename) -> haptics::tools::OHMData;
  auto linearize() -> void;
  auto refactor() -> void;

private:
  static constexpr unsigned int DEFAULT_TIMESCALE = 1000;

  std::string version;
  std::string profile = "Main";
  uint8_t level = 1;
  std::string date;
  std::string description;
  std::optional<unsigned int> timescale;
  std::vector<Perception> perceptions = {};
  std::vector<Avatar> avatars = {};
  std::optional<unsigned int> timescale = DEFAULT_TIMESCALE;
  std::vector<Sync> syncs = {};
};
} // namespace haptics::types
#endif // HAPTICS_H