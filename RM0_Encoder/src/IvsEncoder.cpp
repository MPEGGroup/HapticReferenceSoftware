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

#include "../include/IvsEncoder.h"
#include "Track.h"
#include <fstream>

namespace haptics::encoder {

[[nodiscard]] auto IvsEncoder::encode(const std::string &filename) -> int {
  if (filename.empty()) {
    return EXIT_FAILURE;
  }

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(filename.c_str());
  if (!result) {
    return EXIT_FAILURE;
  }

  std::string &date = IvsEncoder::getLastModified(&doc);
  haptics::types::Track myTrack(0, date, 1.0f, 1.0f, 0);

  pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects = IvsEncoder::getBasisEffects(&doc);
  pugi::xml_node basisEffect = {};
  int bandIndex = -1;
  haptics::types::Effect *effect = nullptr;
  for (pugi::xml_node launchEvent : IvsEncoder::getLaunchEvents(&doc)) {
    if (!IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, basisEffect)) {
      continue;
    }

    effect = new haptics::types::Effect();
    if (!convert(&basisEffect, &launchEvent, effect)) {
      continue;
    }

    // TODO : add bandIndex = myTrack.findBandAvailable(startTime, duration)


    std::cout << "\tEFFECT ";
    std::cout << effect->getPosition() << " | ";
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto IvsEncoder::convert(const pugi::xml_node *basisEffect,
                                       const pugi::xml_node *launchEvent,
                                       haptics::types::Effect *out) -> bool {
  // TODO : to finish
  out->setPosition(launchEvent->attribute("time").as_int());
  out->setPhase(0.0f);
  out->setBaseSignal(IvsEncoder::getWaveform(basisEffect));

  int duration = IvsEncoder::getDuration(basisEffect, launchEvent);  

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

[[nodiscard]] auto IvsEncoder::getLaunchEvents(const pugi::xml_document *doc)
    -> pugi::xml_object_range<pugi::xml_named_node_iterator> {
  pugi::xml_object_range<pugi::xml_named_node_iterator> res =
      doc->child("ivs-file").child("effects").child("timeline-effect").children("launch-event");
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

[[nodiscard]] auto IvsEncoder::getDuration(const pugi::xml_node *basisEffect,
                                           const pugi::xml_node *launchEvent) -> int {
  pugi::xml_attribute durationAttribute = launchEvent->attribute("duration-override");
  const pugi::char_t *n = durationAttribute.name();
  if (std::string(durationAttribute.name()) != "") {
    return durationAttribute.as_int();
  }

  durationAttribute = basisEffect->attribute("duration");
  if (std::string(durationAttribute.name()) != "") {
    return durationAttribute.as_int();
  }

  return -1;
}

[[nodiscard]] auto IvsEncoder::getMagnitude(const pugi::xml_node *basisEffect,
                                           const pugi::xml_node *launchEvent) -> int {
  pugi::xml_attribute magnitudeAttribute = launchEvent->attribute("magnitude-override");
  const pugi::char_t *n = magnitudeAttribute.name();
  if (std::string(magnitudeAttribute.name()) != "") {
    return magnitudeAttribute.as_int();
  }

  magnitudeAttribute = basisEffect->attribute("magnitude");
  if (std::string(magnitudeAttribute.name()) != "") {
    return magnitudeAttribute.as_int();
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

} // namespace haptics::encoder