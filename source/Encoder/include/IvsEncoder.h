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

#ifndef IVSENCODER_H
#define IVSENCODER_H

#include <Types/include/Perception.h>
#include <iostream>
#include <pugixml.hpp>
#include <vector>

#include <Types/include/Effect.h>
#include <Types/include/Keyframe.h>
#include <Types/include/Track.h>

namespace haptics::encoder {

class IvsEncoder {
public:
  auto static encode(const std::string &filename, types::Perception &out) -> int;
  [[nodiscard]] auto static getLastModified(const pugi::xml_document *doc) -> std::string;
  [[nodiscard]] auto static getBasisEffects(const pugi::xml_document *doc)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getLaunchEvents(const pugi::xml_document *doc)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getRepeatEvents(const pugi::xml_document *doc)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getLaunchedEffect(
      const pugi::xml_object_range<pugi::xml_named_node_iterator> *basisEffects,
      const pugi::xml_node *launchEvent, pugi::xml_node &out) -> bool;

  [[nodiscard]] auto static convertToEffect(const pugi::xml_node *basisEffect,
                                            const pugi::xml_node *launchEvent,
                                            haptics::types::Effect *out) -> bool;

  [[nodiscard]] auto static getTime(const pugi::xml_node *node) -> int;
  [[nodiscard]] auto static getCount(const pugi::xml_node *node) -> int;
  [[nodiscard]] auto static getDuration(const pugi::xml_node *node) -> int;
  [[nodiscard]] auto static getDuration(const pugi::xml_node *basisEffect,
                                        const pugi::xml_node *launchEvent) -> int;
  [[nodiscard]] auto static getMagnitude(const pugi::xml_node *basisEffect,
                                         const pugi::xml_node *launchEvent) -> int;
  [[nodiscard]] auto static getPeriod(const pugi::xml_node *basisEffect,
                                      const pugi::xml_node *launchEvent) -> int;
  [[nodiscard]] auto static getWaveform(const pugi::xml_node *basisEffect)
      -> haptics::types::BaseSignal;
  [[nodiscard]] auto static getAttackTime(const pugi::xml_node *basisEffect) -> int;
  [[nodiscard]] auto static getAttackLevel(const pugi::xml_node *basisEffect) -> int;
  [[nodiscard]] auto static getFadeTime(const pugi::xml_node *basisEffect) -> int;
  [[nodiscard]] auto static getFadeLevel(const pugi::xml_node *basisEffect) -> int;
  [[nodiscard]] auto static floatToInt(int f) -> int;

private:
  static constexpr float MAGNITUDE_2_AMPLITUDE = .0001F;
  static const int MIN_FREQUENCY = 0;
  static const int MAX_FREQUENCY = 1000;
  static const int MAGSWEEP_FREQUENCY = 170;
};

} // namespace haptics::encoder
#endif // IVSENCODER_H