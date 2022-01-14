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

#ifndef IOJSON_H
#define IOJSON_H

#include <string>
#include <nlohmann/json.hpp>
#include <Types/include/Haptics.h>
#include <map>

namespace haptics::encoder {
  static const std::map<std::string, types::EncodingModality> stringToModality = {
    {"Quantized", types::EncodingModality::Quantized},
    {"Vectorial", types::EncodingModality::Vectorial},
    {"Wavelet", types::EncodingModality::Wavelet}};
  static const std::map<types::EncodingModality, std::string> modalityToString = {
    {types::EncodingModality::Quantized, "Quantized"},
    {types::EncodingModality::Vectorial, "Vectorial"},
    {types::EncodingModality::Wavelet, "Wavelet"}};

  
  static const std::map<std::string, types::BandType> stringToBandType = {
      {"Wave", types::BandType::Wave},
      {"Curve", types::BandType::Curve},
      {"Transient", types::BandType::Transient}};
  static const std::map<types::BandType, std::string> bandTypeToString = {
      {types::BandType::Wave, "Wave"},
      {types::BandType::Curve, "Curve"},
      {types::BandType::Transient, "Transient"}};

  
  static const std::map<std::string, types::BaseSignal> stringToBaseSignal = {
      {"Sine", types::BaseSignal::Sine},
      {"Square", types::BaseSignal::Square},
      {"Triangle", types::BaseSignal::Triangle},
      {"SawToothUp", types::BaseSignal::SawToothUp},
      {"SawToothDown", types::BaseSignal::SawToothDown}};
  static const std::map<types::BaseSignal, std::string> baseSignalToString = {
      {types::BaseSignal::Sine, "Sine"},
      {types::BaseSignal::Square, "Square"},
      {types::BaseSignal::Triangle, "Triangle"},
      {types::BaseSignal::SawToothUp, "SawToothUp"},
      {types::BaseSignal::SawToothDown, "SawToothDown"}};


  static const std::map<std::string, types::PerceptionModality> stringToPerceptionModality = {
      {"Other", types::PerceptionModality::Other},
      {"Pressure", types::PerceptionModality::Pressure},
      {"Acceleration", types::PerceptionModality::Acceleration},
      {"Velocity", types::PerceptionModality::Velocity},
      {"Position", types::PerceptionModality::Position},
      {"Temperature", types::PerceptionModality::Temperature},
      {"Vibration", types::PerceptionModality::Vibration},
      {"Water", types::PerceptionModality::Water},
      {"Wind", types::PerceptionModality::Wind}};
  static const std::map<types::PerceptionModality, std::string> perceptionModalityToString = {
      {types::PerceptionModality::Other, "Other"},
      {types::PerceptionModality::Pressure, "Pressure"},
      {types::PerceptionModality::Acceleration, "Acceleration"},
      {types::PerceptionModality::Velocity, "Velocity"},
      {types::PerceptionModality::Position, "Position"},
      {types::PerceptionModality::Temperature, "Temperature"},
      {types::PerceptionModality::Vibration, "Vibration"},
      {types::PerceptionModality::Water, "Water"},
      {types::PerceptionModality::Wind, "Wind"}};

  
  static const std::map<std::string, types::AvatarType> stringToAvatarType = {
      {"Vibration", types::AvatarType::Vibration},
      {"Pressure", types::AvatarType::Pressure},
      {"Temperature", types::AvatarType::Temperature},
      {"Custom", types::AvatarType::Custom}};
  static const std::map<types::AvatarType, std::string> avatarTypeToString = {
      {types::AvatarType::Vibration, "Vibration"},
      {types::AvatarType::Pressure, "Pressure"},
      {types::AvatarType::Temperature, "Temperature"},
      {types::AvatarType::Custom, "Custom"}};

  class IOJson {
public:
  static auto loadFile(const std::string &filePath) -> types::Haptics;
  static auto loadPerceptions(const nlohmann::json &jsonPerceptions, types::Haptics &haptic)
      -> void;
  static auto loadAvatars(const nlohmann::json &jsonAvatars, types::Haptics &haptic) -> void;
  static auto loadTracks(const nlohmann::json &jsonTracks, types::Perception &perception) -> void;
  static auto loadReferenceDevices(const nlohmann::json &jsonReferenceDevices,
                                   types::Perception &perception) -> void;
  static auto loadBands(const nlohmann::json &jsonBands, types::Track &track) -> void;
  static auto loadEffects(const nlohmann::json &jsonEffects, types::Band &band) -> void;
  static auto loadKeyframes(const nlohmann::json &jsonKeyframes, types::Effect &effect) -> void;

  static auto extractPerceptions(types::Haptics &haptic, nlohmann::json &jsonPerceptions)
      -> void;
  static auto extractAvatars(types::Haptics &haptic, nlohmann::json &jsonAvatars)
      -> void;
  static auto extractTracks(types::Perception &perception, nlohmann::json &jsonTracks)
      -> void;
  static auto
  extractReferenceDevices(types::Perception &perception, nlohmann::json &jsonReferenceDevices) -> void;

  static auto writeFile(types::Haptics &haptic, const std::string &filePath) -> void;
};
} // namespace encoder
#endif //IOJSON_H
