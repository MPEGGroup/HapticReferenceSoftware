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

#include <iostream>
#include <pugixml.hpp>
#include <vector>

#include <Types/include/Channel.h>
#include <Types/include/Effect.h>
#include <Types/include/Keyframe.h>
#include <Types/include/Perception.h>

namespace haptics::encoder {

class IvsEncoder {
public:
  auto static encode(const std::string &filename, types::Perception &out) -> int;
  [[nodiscard]] auto static getLastModified(const pugi::xml_document *doc) -> std::string;
  [[nodiscard]] auto static getBasisEffects(const pugi::xml_document *doc)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getTimelineEffects(const pugi::xml_document *doc)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getLaunchEvents(const pugi::xml_node *timeline)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getRepeatEvents(const pugi::xml_node *timeline)
      -> pugi::xml_object_range<pugi::xml_named_node_iterator>;
  [[nodiscard]] auto static getLaunchedEffect(
      const pugi::xml_object_range<pugi::xml_named_node_iterator> *basisEffects,
      const pugi::xml_node *launchEvent, pugi::xml_node &out) -> bool;

  [[nodiscard]] auto static convertToEffect(const pugi::xml_node *basisEffect,
                                            const pugi::xml_node *launchEvent,
                                            haptics::types::Effect *out) -> bool;

  [[nodiscard]] auto static getName(const pugi::xml_node *node) -> std::string;
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
  auto static isRepeatNested(pugi::xml_node *parent, pugi::xml_node *child) -> bool;
  auto static isRepeatNested(int parent_start, int parent_end, int child_start) -> bool;
  auto static injectIntoBands(types::Effect &effect, types::Channel &channel) -> void;
  static constexpr float MAGNITUDE_2_AMPLITUDE = .0001F;
  static const int MIN_FREQUENCY = 0;
  static const int MAX_FREQUENCY = 1000;
  static const int MAGSWEEP_FREQUENCY = 170;

  struct RepeatNode {
    pugi::xml_node *value;
    std::vector<types::Effect> myEffects;
    std::vector<RepeatNode> children;

    auto pushEffect(types::Effect &myEffect) -> bool {
      if (value != nullptr) {
        int start = IvsEncoder::getTime(value);
        int end = start + IvsEncoder::getDuration(value);
        int position = myEffect.getPosition();
        if (start > position || position >= end) {
          return false;
        }
      }

      for (auto it = children.begin(); it < children.end(); it++) {
        if (it->pushEffect(myEffect)) {
          return true;
        }
      }

      auto it = myEffects.end();
      auto it_effects = myEffects.begin();
      int effectPosition = myEffect.getPosition();
      for (; it_effects < myEffects.end(); it_effects++) {
        if (it_effects->getPosition() < effectPosition) {
          it = it_effects + 1;
        }
      }
      myEffects.insert(it, myEffect);
      return true;
    }

    auto linearize(std::vector<types::Effect> &effectLibrary, int &delay) -> int {
      int count = 0;
      int duration = 0;
      if (value != nullptr) {
        count = IvsEncoder::getCount(value);
        duration = IvsEncoder::getDuration(value);
      }
      auto myEffects_it = myEffects.begin();
      auto children_it = children.begin();
      int delay_tmp = 0;
      std::vector<types::Effect> linearized_effects;
      for (; myEffects_it < myEffects.end() && children_it < children.end();) {
        int repeat_position =
            children_it->value == nullptr ? 0 : IvsEncoder::getTime(children_it->value);

        if (myEffects_it->getPosition() < repeat_position) {
          types::Effect e = *myEffects_it;
          int position = e.getPosition();
          e.setPosition(delay + position);
          linearized_effects.push_back(e);
          myEffects_it++;
        } else {
          delay_tmp = children_it->linearize(linearized_effects, delay);
          duration += delay_tmp;
          delay += delay_tmp;
          children_it++;
        }
      }
      for (; myEffects_it < myEffects.end(); myEffects_it++) {
        types::Effect e = *myEffects_it;
        int position = e.getPosition();
        e.setPosition(delay + position);
        linearized_effects.push_back(e);
      }
      for (; children_it < children.end(); children_it++) {
        delay_tmp = children_it->linearize(linearized_effects, delay);
        duration += delay_tmp;
        delay += delay_tmp;
      }

      for (int i = 0; i <= count; i++) {
        for (types::Effect e : linearized_effects) {
          e.setPosition(e.getPosition() + i * duration);
          effectLibrary.push_back(e);
        }
      }

      return count * duration;
    }
  };
};
} // namespace haptics::encoder
#endif // IVSENCODER_H
