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

#ifndef HAPSENCODER_H
#define HAPSENCODER_H

#include <Types/include/Band.h>
#include <Types/include/Effect.h>
#include <Types/include/Keyframe.h>
#include <Types/include/Perception.h>
#include <iostream>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996 26451 26495 26812 33010)
#endif
#include <rapidjson/document.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::encoder {

class HapsEncoder {
public:
  enum class Waveform : int {
    Unknown = -2,
    Constant = -1,
    Sine = haptics::types::BaseSignal::Sine,
    Square = haptics::types::BaseSignal::Square,
    Triangle = haptics::types::BaseSignal::Triangle,
    SawToothUp = haptics::types::BaseSignal::SawToothUp,
    SawToothDown = haptics::types::BaseSignal::SawToothDown
  };

  [[nodiscard]] auto static encode(std::string &filename, types::Perception &out,
                                   unsigned int timescale) -> int;
  [[nodiscard]] auto static extractVibration(const rapidjson::Value::Object &vibrationTrack,
                                             types::Channel &channel, const unsigned int timescale)
      -> int;
  [[nodiscard]] auto static extractTransients(const rapidjson::Value::Array &transients,
                                              types::Band *transientBand,
                                              const unsigned int timescale) -> int;
  [[nodiscard]] auto static extractMelodies(const rapidjson::Value::Array &melodies,
                                            types::Channel &channel, int lowerFrequencyLimit,
                                            int upperFrequencyLimit, const unsigned int timescale)
      -> int;
  [[nodiscard]] auto static extractNote(const rapidjson::Value::Object &note, types::Band *band,
                                        const double amplitudeMultiplier,
                                        const unsigned int timescale) -> int;
  [[nodiscard]] auto static extractFrequencyRange(const rapidjson::Value::Object &vibrationTrack,
                                                  int &lowerFrequencyLimit,
                                                  int &upperFrequencyLimit) -> int;
  [[nodiscard]] auto static getWaveform(const std::string &waveform, Waveform &out) -> int;

private:
  enum class ModulationType : uint8_t { Constant = 0, PeriodicSignal = 1, InterpolationCurve = 2 };

  [[nodiscard]] auto static extractPitchInNote(const rapidjson::Value::Object &note,
                                               types::Effect &effect, const unsigned int timescale,
                                               const int lowerFrequencyLimit,
                                               const int upperFrequencyLimit,
                                               int &lastFrequencyValue,
                                               int &lastFrequencyTimestamp,
                                               ModulationType &modulationType) -> int;
  [[nodiscard]] auto static extractAmplitudeInNote(const rapidjson::Value::Object &note,
                                                   types::Effect &effect,
                                                   const unsigned int timescale,
                                                   std::optional<int> timescaledLength,
                                                   const double amplitudeMultiplier,
                                                   ModulationType &modulationType) -> int;
  [[nodiscard]] auto static extractAmplitudeAsPeriodicSignal(
      const rapidjson::Value::Object &amplitudeModulation, types::Effect &effect,
      const unsigned int timescale, std::optional<int> timescaledLength,
      const double amplitudeMultiplier, ModulationType &modulationType) -> int;
  auto static storeAmplitudeAsPeriodicSignal(types::Effect &effect, const unsigned timescale,
                                             const double amplitudeMultiplier,
                                             std::optional<int> timescaledLength,
                                             const Waveform waveform, const double amplitude,
                                             const double verticalOffset, const double periodLength,
                                             const double phase) -> void;
  auto static storeApproximatedAmplitudeKeyframe(
      types::Effect &effect, const unsigned int timescale, const double amplitudeMultiplier,
      const int t, const Waveform waveform, const double amplitude, const double verticalOffset,
      const double periodLength, const double phase) -> void;
  auto static storeAmplitudeAsConstant(const double amplitude, types::Effect &effect,
                                       std::optional<int> timescaledLength) -> void;
  auto static storeAmplitudeAsConstant(const double amplitude, types::Effect &effect,
                                       std::optional<int> timescaledLength,
                                       ModulationType &modulationType) -> void;
  [[nodiscard]] auto static extractFrequencyCurve(const rapidjson::Value::Object &curve,
                                                  types::Effect &effect,
                                                  const unsigned int timescale,
                                                  const int lowerFrequencyLimit,
                                                  const int upperFrequencyLimit, int &lastValue,
                                                  int &lastPosition) -> int;
  [[nodiscard]] auto static extractFrequencyCurve(const rapidjson::Value::Object &curve,
                                                  types::Effect &effect,
                                                  const unsigned int timescale,
                                                  const int lowerFrequencyLimit,
                                                  const int upperFrequencyLimit) -> int;
  [[nodiscard]] auto static extractAmplitudeCurve(const rapidjson::Value::Object &curve,
                                                  types::Effect &effect,
                                                  const unsigned int timescale,
                                                  const double amplitudeModifier) -> int;
  [[nodiscard]] auto static extractAmplitudeCurve(const rapidjson::Value::Object &curve,
                                                  types::Effect &effect,
                                                  const unsigned int timescale,
                                                  const double amplitudeModifier, double &lastValue,
                                                  int &lastPosition) -> int;
  [[nodiscard]] auto static extractCurve(const rapidjson::Value::Object &curve,
                                         types::Effect &effect, const unsigned int timescale,
                                         const bool isAmplitude, const double amplitudeModifier,
                                         const int lowerFrequencyLimit,
                                         const int upperFrequencyLimit) -> int;
  [[nodiscard]] auto static extractCurve(const rapidjson::Value::Object &curve,
                                         types::Effect &effect, const unsigned int timescale,
                                         const bool isAmplitude, const double amplitudeModifier,
                                         const int lowerFrequencyLimit,
                                         const int upperFrequencyLimit, double &lastValue,
                                         int &lastPosition) -> int;
  auto static secondsToTimeScale(const double seconds, const unsigned int timescale) -> int;
  auto static millisecondsToTimeScale(const double milliseconds, const unsigned int timescale) -> int;
  auto static computeAbsoluteFreq(int lowerFrequencyLimit, int upperFrequencyLimit, double freq)
      -> int;

  static const inline int MIN_HAPS_FREQUENCY = 0;
  static const inline int MAX_HAPS_FREQUENCY = 1000;
};
} // namespace haptics::encoder
#endif // HAPSENCODER_H