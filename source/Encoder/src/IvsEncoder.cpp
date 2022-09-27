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

#include <Encoder/include/IvsEncoder.h>
#include <Tools/include/Tools.h>
#include <fstream>
#include <limits>

namespace haptics::encoder {

auto IvsEncoder::encode(const std::string &filename, types::Perception &out) -> int {
  if (filename.empty()) {
    return EXIT_FAILURE;
  }

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(filename.c_str());
  if (!result) {
    return EXIT_FAILURE;
  }

  std::string date = IvsEncoder::getLastModified(&doc);
  int trackId = 0;
  for (const pugi::xml_node timeline : IvsEncoder::getTimelineEffects(&doc)) {
    haptics::types::Track myTrack;
    if (out.getTracksSize() <= static_cast<size_t>(trackId)) {
      std::string timelineName = IvsEncoder::getName(&timeline);
      myTrack = haptics::types::Track(0, timelineName.append(" - ").append(date), 1, 1, 0);
      out.addTrack(myTrack);
    };
    myTrack = out.getTrackAt(trackId);

    pugi::xml_object_range<pugi::xml_named_node_iterator> repeatEvents =
        IvsEncoder::getRepeatEvents(&timeline);
    std::vector<pugi::xml_node> sortedRepeatEvents;
    for (pugi::xml_node &repeatEvent : repeatEvents) {
      auto it = std::lower_bound(sortedRepeatEvents.begin(), sortedRepeatEvents.end(), repeatEvent,
                                 [](pugi::xml_node a, pugi::xml_node b) -> bool {
                                   int time_a = IvsEncoder::getTime(&a);
                                   int time_b = IvsEncoder::getTime(&b);
                                   return time_a == time_b ? IvsEncoder::getDuration(&a) >=
                                                                 IvsEncoder::getDuration(&b)
                                                           : time_a <= time_b;
                                 });
      sortedRepeatEvents.insert(it, repeatEvent);
    }

    auto *repeatTree = new RepeatNode{nullptr, nullptr, {}, {}};
    for (pugi::xml_node &repeatEvent : sortedRepeatEvents) {
      auto *currentNode = new RepeatNode{&repeatEvent, repeatTree, {}, {}};
      RepeatNode *researchNode = repeatTree;
      bool continue_search = false;
      do {
        if (researchNode->children.empty()) {
          break;
        }

        auto *lastChild = researchNode->children.back();
        continue_search = isRepeatNested(lastChild->value, currentNode->value);
        if (continue_search) {
          currentNode->father = researchNode = lastChild;
        }
      } while (continue_search);
      researchNode->children.push_back(currentNode);
    }

    pugi::xml_node basisEffect = {};
    std::vector<haptics::types::Effect> myEffects;
    pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects =
        IvsEncoder::getBasisEffects(&doc);
    for (pugi::xml_node launchEvent : IvsEncoder::getLaunchEvents(&timeline)) {
      if (!IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, basisEffect)) {
        continue;
      }

      haptics::types::Effect myEffect{};
      if (IvsEncoder::convertToEffect(&basisEffect, &launchEvent, &myEffect)) {
        repeatTree->pushEffect(myEffect);
      }
    }

    std::vector<types::Effect> repeatedEffectsSet;
    int delay = 0;
    repeatTree->linearize(repeatedEffectsSet, delay);

    for (haptics::types::Effect &myEffect : repeatedEffectsSet) {
      IvsEncoder::injectIntoBands(myEffect, myTrack);
    }
    out.replaceTrackAt(trackId, myTrack);
    trackId++;
  }

  return EXIT_SUCCESS;
}

auto IvsEncoder::isRepeatNested(pugi::xml_node *parent, pugi::xml_node *child) -> bool {
  auto parent_start = IvsEncoder::getTime(parent);
  auto parent_end = parent_start + IvsEncoder::getDuration(parent);
  auto child_start = IvsEncoder::getTime(child);
  return IvsEncoder::isRepeatNested(parent_start, parent_end, child_start);
}

auto IvsEncoder::isRepeatNested(int parent_start, int parent_end, int child_start) -> bool {
  return parent_start <= child_start && child_start < parent_end;
}

#pragma region TMP
auto IvsEncoder::injectIntoBands(types::Effect &effect, types::Track &track) -> void {
  haptics::types::Band *myBand =
      track.findBandAvailable(effect.getPosition(),
                              effect.getKeyframeAt(static_cast<int>(effect.getKeyframesSize()) - 1)
                                  .getRelativePosition()
                                  .value(),
                              types::BandType::VectorialWave);
  if (myBand == nullptr) {
    myBand = track.generateBand(haptics::types::BandType::VectorialWave,
                                haptics::types::CurveType::Unknown, 0, IvsEncoder::MIN_FREQUENCY,
                                IvsEncoder::MAX_FREQUENCY);
  }
  myBand->addEffect(effect);
}

[[nodiscard]] auto IvsEncoder::convertToEffect(const pugi::xml_node *basisEffect,
                                               const pugi::xml_node *launchEvent,
                                               haptics::types::Effect *out) -> bool {
  int periodLength = IvsEncoder::getPeriod(basisEffect, launchEvent);
  int freq = 0;
  std::string effectType = basisEffect->attribute("type").as_string();
  if (effectType == "periodic") {
    out->setBaseSignal(IvsEncoder::getWaveform(basisEffect));
    freq = static_cast<int>(1.0 / (static_cast<float>(periodLength) * MS_2_S));
  } else if (effectType == "magsweep") {
    out->setBaseSignal(types::BaseSignal::Sine);
    freq = IvsEncoder::MAGSWEEP_FREQUENCY;
  } else {
    return false;
  }

  out->setPosition(IvsEncoder::getTime(launchEvent));
  out->setPhase(0);

  int duration = IvsEncoder::getDuration(basisEffect, launchEvent);
  int magnitude = IvsEncoder::getMagnitude(basisEffect, launchEvent);

  float amplitude = static_cast<float>(magnitude) * IvsEncoder::MAGNITUDE_2_AMPLITUDE;
  types::Keyframe k(0, amplitude, freq);
  out->addKeyframe(k);
  k = types::Keyframe(duration, amplitude, freq);
  out->addKeyframe(k);

  int attackTime = IvsEncoder::getAttackTime(basisEffect);
  if (attackTime != -1) {
    int attackLevel = IvsEncoder::getAttackLevel(basisEffect);
    float attackAmplitude = static_cast<float>(attackLevel) * IvsEncoder::MAGNITUDE_2_AMPLITUDE;
    out->addAmplitudeAt(attackAmplitude, 0);
    if (attackTime >= duration) {
      std::pair<int, double> attackStart(0, attackAmplitude);
      std::pair<int, double> attackEnd(attackTime, amplitude);
      out->addAmplitudeAt(
          static_cast<float>(tools::linearInterpolation(attackStart, attackEnd, duration)),
          duration);
      return true;
    }
    out->addAmplitudeAt(amplitude, attackTime);
  }

  int fadeDuration = IvsEncoder::getFadeTime(basisEffect);
  if (fadeDuration != -1) {
    int fadeTime = duration - fadeDuration;

    int fadeLevel = IvsEncoder::getFadeLevel(basisEffect);
    float fadeAmplitude = static_cast<float>(fadeLevel) * IvsEncoder::MAGNITUDE_2_AMPLITUDE;
    auto actualAttackTime = std::max(attackTime, 0);
    if (fadeTime < actualAttackTime) {
      std::pair<int, double> fadeStart(fadeTime, amplitude);
      std::pair<int, double> fadeEnd(duration, fadeAmplitude);
      out->addAmplitudeAt(
          static_cast<float>(tools::linearInterpolation(fadeStart, fadeEnd, actualAttackTime)),
          actualAttackTime, false);
    } else if (fadeTime > actualAttackTime) {
      out->addAmplitudeAt(amplitude, fadeTime);
    }
    out->addAmplitudeAt(fadeAmplitude, duration);
  }

  return true;
}

[[nodiscard]] auto IvsEncoder::getLastModified(const pugi::xml_document *doc) -> std::string {
  std::string res = std::string(doc->child("ivs-file").attribute("last-modified").value());
  return res;
}

[[nodiscard]] auto IvsEncoder::getBasisEffects(const pugi::xml_document *doc)
    -> pugi::xml_object_range<pugi::xml_named_node_iterator> {
  pugi::xml_object_range<pugi::xml_named_node_iterator> res =
      doc->child("ivs-file").child("effects").children("basis-effect");
  return res;
}

[[nodiscard]] auto IvsEncoder::getTimelineEffects(const pugi::xml_document *doc)
    -> pugi::xml_object_range<pugi::xml_named_node_iterator> {
  pugi::xml_object_range<pugi::xml_named_node_iterator> res =
      doc->child("ivs-file").child("effects").children("timeline-effect");
  return res;
}

[[nodiscard]] auto IvsEncoder::getLaunchEvents(const pugi::xml_node *timeline)
    -> pugi::xml_object_range<pugi::xml_named_node_iterator> {
  pugi::xml_object_range<pugi::xml_named_node_iterator> res = timeline->children("launch-event");
  return res;
}

[[nodiscard]] auto IvsEncoder::getRepeatEvents(const pugi::xml_node *timeline)
    -> pugi::xml_object_range<pugi::xml_named_node_iterator> {
  pugi::xml_object_range<pugi::xml_named_node_iterator> res = timeline->children("repeat-event");
  return res;
}

[[nodiscard]] auto IvsEncoder::getLaunchedEffect(
    const pugi::xml_object_range<pugi::xml_named_node_iterator> *basisEffects,
    const pugi::xml_node *launchEvent, pugi::xml_node &out) -> bool {
  auto it = (*basisEffects).begin();
  while (it != (*basisEffects).end()) {
    std::string launchEventStr(launchEvent->attribute("effect").as_string());
    std::string basisEffectStr((*it).attribute("name").as_string());
    if (launchEventStr == basisEffectStr) {
      out = *it;
      return true;
    }
    it++;
  }

  return false;
}

[[nodiscard]] auto IvsEncoder::getName(const pugi::xml_node *node) -> std::string {
  pugi::xml_attribute timeAttribute = node->attribute("name");
  if (!std::string(timeAttribute.name()).empty()) {
    return timeAttribute.as_string();
  }

  return "";
}

[[nodiscard]] auto IvsEncoder::getCount(const pugi::xml_node *node) -> int {
  pugi::xml_attribute timeAttribute = node->attribute("count");
  if (!std::string(timeAttribute.name()).empty()) {
    return timeAttribute.as_int();
  }

  return 0;
}

[[nodiscard]] auto IvsEncoder::getTime(const pugi::xml_node *node) -> int {
  pugi::xml_attribute timeAttribute = node->attribute("time");
  if (!std::string(timeAttribute.name()).empty()) {
    return timeAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getDuration(const pugi::xml_node *node) -> int {
  pugi::xml_attribute durationAttribute = node->attribute("duration");
  if (!std::string(durationAttribute.name()).empty()) {
    return durationAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getDuration(const pugi::xml_node *basisEffect,
                                           const pugi::xml_node *launchEvent) -> int {
  pugi::xml_attribute durationAttribute = launchEvent->attribute("duration-override");
  if (!std::string(durationAttribute.name()).empty()) {
    return durationAttribute.as_int();
  }

  return IvsEncoder::getDuration(basisEffect);
}

[[nodiscard]] auto IvsEncoder::getMagnitude(const pugi::xml_node *basisEffect,
                                            const pugi::xml_node *launchEvent) -> int {
  pugi::xml_attribute magnitudeAttribute = launchEvent->attribute("magnitude-override");
  if (!std::string(magnitudeAttribute.name()).empty()) {
    return magnitudeAttribute.as_int();
  }

  magnitudeAttribute = basisEffect->attribute("magnitude");
  if (!std::string(magnitudeAttribute.name()).empty()) {
    return magnitudeAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getPeriod(const pugi::xml_node *basisEffect,
                                         const pugi::xml_node *launchEvent) -> int {
  pugi::xml_attribute periodAttribute = launchEvent->attribute("period-override");

  if (!std::string(periodAttribute.name()).empty()) {
    return floatToInt(periodAttribute.as_int());
  }

  periodAttribute = basisEffect->attribute("period");
  if (!std::string(periodAttribute.name()).empty()) {
    return floatToInt(periodAttribute.as_int());
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getWaveform(const pugi::xml_node *basisEffect)
    -> haptics::types::BaseSignal {
  std::string waveform = std::string(basisEffect->attribute("waveform").as_string());
  if (waveform == "sine") {
    return haptics::types::BaseSignal::Sine;
  }
  if (waveform == "square") {
    return haptics::types::BaseSignal::Square;
  }
  if (waveform == "triangle") {
    return haptics::types::BaseSignal::Triangle;
  }
  if (waveform == "sawtooth-up") {
    return haptics::types::BaseSignal::SawToothUp;
  }
  if (waveform == "sawtooth-down") {
    return haptics::types::BaseSignal::SawToothDown;
  }

  return haptics::types::BaseSignal(-1);
}

[[nodiscard]] auto IvsEncoder::getAttackTime(const pugi::xml_node *basisEffect) -> int {
  pugi::xml_attribute attackTimeAttribute = basisEffect->attribute("attack-time");
  if (!std::string(attackTimeAttribute.name()).empty()) {
    return attackTimeAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getAttackLevel(const pugi::xml_node *basisEffect) -> int {
  pugi::xml_attribute attackLevelAttribute = basisEffect->attribute("attack-level");
  if (!std::string(attackLevelAttribute.name()).empty()) {
    return attackLevelAttribute.as_int();
  }

  return 0;
}

[[nodiscard]] auto IvsEncoder::getFadeTime(const pugi::xml_node *basisEffect) -> int {
  pugi::xml_attribute fadeTimeAttribute = basisEffect->attribute("fade-time");
  if (!std::string(fadeTimeAttribute.name()).empty()) {
    return fadeTimeAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getFadeLevel(const pugi::xml_node *basisEffect) -> int {
  pugi::xml_attribute fadeLevelAttribute = basisEffect->attribute("fade-level");
  if (!std::string(fadeLevelAttribute.name()).empty()) {
    return fadeLevelAttribute.as_int();
  }

  return 0;
}

[[nodiscard]] auto IvsEncoder::floatToInt(const int f) -> int {

  if (f < 0) {
    int res = ((f + std::numeric_limits<int>::max()) + 1) / MS_2_MICROSECONDS;
    return res;
  }

  return f;
}
#pragma endregion
} // namespace haptics::encoder
