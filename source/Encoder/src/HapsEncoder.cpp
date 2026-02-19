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

#include <Encoder/include/HapsEncoder.h>
#include <Tools/include/Tools.h>
#include <algorithm>
#include <cmath>
#include <fstream>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
#include <rapidjson/istreamwrapper.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace haptics::encoder {

[[nodiscard]] auto HapsEncoder::encode(std::string &filename, haptics::types::Perception &out,
                                       const unsigned int timescale) -> int {
  if (out.getChannelsSize() > 1) {
    return EXIT_FAILURE;
  }

  std::ifstream ifs(filename);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json;
  if (json.ParseStream<rapidjson::kParseTrailingCommasFlag>(isw).HasParseError()) {
    std::cerr << "Invalid HAPS input file: JSON parsing error" << std::endl;
    return EXIT_FAILURE;
  }
  if (!json.IsObject()) {
    std::cerr << "Invalid HAPS input file: not a JSON object" << std::endl;
    return EXIT_FAILURE;
  }
  if (!json.HasMember("version") || !json["version"].IsString()) {
    std::cerr << "Invalid HAPS input file: missing or invalid version" << std::endl;
    return EXIT_FAILURE;
  }
  std::string version = json["version"].GetString();
  if (version != "6") {
    std::cerr << "Invalid HAPS input file: invalid version" << std::endl;
    return EXIT_FAILURE;
  }

  float gain = 1;
  if (json.HasMember("gain")) {
    if (!json["gain"].IsFloat()) {
      std::cerr << "Invalid HAPS input file: invalid gain" << std::endl;
      return EXIT_FAILURE;
    }
    gain = json["gain"].GetFloat();
  }

  std::string description;
  if (json.HasMember("description")) {
    if (!json["description"].IsString()) {
      std::cerr << "Invalid HAPS input file: invalid description" << std::endl;
      return EXIT_FAILURE;
    }
    description = json["description"].GetString();
  }

  haptics::types::Channel myChannel(0, description, gain, 1, 0);
  if (out.getChannelsSize() == 0) {
    out.addChannel(myChannel);
  }
  myChannel = out.getChannelAt(0);

  if (json.HasMember("vibration")) {
    if (!json["vibration"].IsObject()) {
      std::cerr << "Invalid HAPS input file: invalid vibration" << std::endl;
      return EXIT_FAILURE;
    }

    if (extractVibration(json["vibration"].GetObject(), myChannel, timescale) == EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: impossible to encode the vibration" << std::endl;
      return EXIT_FAILURE;
    }
  }

  out.replaceChannelAt(0, myChannel);
  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::extractVibration(const rapidjson::Value::Object &vibrationTrack,
                                                 types::Channel &channel,
                                                 const unsigned int timescale) -> int {
  if (vibrationTrack.HasMember("mute")) {
    if (!vibrationTrack["mute"].IsBool()) {
      std::cerr << "Invalid HAPS input file: invalid mute" << std::endl;
      return EXIT_FAILURE;
    }

    if (vibrationTrack["mute"].GetBool()) {
      channel.setGain(0.0F);
    }
  }

  int lowerFrequencyLimit = MIN_HAPS_FREQUENCY;
  int upperFrequencyLimit = MAX_HAPS_FREQUENCY;
  if (extractFrequencyRange(vibrationTrack, lowerFrequencyLimit, upperFrequencyLimit) ==
      EXIT_FAILURE) {
    std::cerr << "Invalid HAPS input file: impossible to read the frequency range" << std::endl;
    return EXIT_FAILURE;
  }

  if (vibrationTrack.HasMember("transients")) {
    if (!vibrationTrack["transients"].IsArray()) {
      std::cerr << "Invalid HAPS input file: invalid transients" << std::endl;
      return EXIT_FAILURE;
    }

    auto transients = vibrationTrack["transients"].GetArray();
    haptics::types::Band *transientBand = channel.generateBand(
        haptics::types::BandType::Transient, lowerFrequencyLimit, upperFrequencyLimit);
    if (extractTransients(transients, transientBand, timescale) == EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: impossible to encode the transients" << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (vibrationTrack.HasMember("melodies")) {
    if (!vibrationTrack["melodies"].IsArray()) {
      std::cerr << "Invalid HAPS input file: invalid melodies" << std::endl;
      return EXIT_FAILURE;
    }

    auto melodies = vibrationTrack["melodies"].GetArray();
    if (extractMelodies(melodies, channel, lowerFrequencyLimit, upperFrequencyLimit, timescale) ==
        EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: impossible to encode the melodies" << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::extractTransients(const rapidjson::Value::Array &transients,
                                                  types::Band *transientBand,
                                                  const unsigned int timescale) -> int {
  types::Effect transientEffect;
  for (auto &t : transients) {
    if (!t.HasMember("position") || !t["position"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid transient position" << std::endl;
      return EXIT_FAILURE;
    }

    double position = t["position"].GetDouble();
    if (position < 0.0) {
      std::cerr << "Invalid HAPS input file: transient position is negative" << std::endl;
      return EXIT_FAILURE;
    }
    int timeScalledPosition = secondsToTimeScale(position, timescale);

    double amplitude = 1.0;
    if (t.HasMember("amplitude")) {
      if (!t["amplitude"].IsDouble()) {
        std::cerr << "Invalid HAPS input file: invalid transient amplitude" << std::endl;
        return EXIT_FAILURE;
      }

      amplitude = t["amplitude"].GetDouble();
      if (amplitude < 0.0 || amplitude > 1.0) {
        std::cerr << "Invalid HAPS input file: transient amplitude is not normalized" << std::endl;
        return EXIT_FAILURE;
      }
    }

    double frequency = 0.0;
    if (t.HasMember("pitch")) {
      if (!t["pitch"].IsDouble()) {
        std::cerr << "Invalid HAPS input file: invalid transient pitch" << std::endl;
        return EXIT_FAILURE;
      }

      frequency = t["pitch"].GetDouble();
      if (frequency < 0.0 || frequency > 1.0) {
        std::cerr << "Invalid HAPS input file: transient pitch is not normalized" << std::endl;
        return EXIT_FAILURE;
      }
    }
    int absoluteFreq = computeAbsoluteFreq(transientBand->getLowerFrequencyLimit(),
                                           transientBand->getUpperFrequencyLimit(), frequency);
    transientEffect.addKeyframe(timeScalledPosition, amplitude, absoluteFreq);
  }

  transientBand->addEffect(transientEffect);
  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::extractMelodies(const rapidjson::Value::Array &melodies,
                                                types::Channel &channel, int lowerFrequencyLimit,
                                                int upperFrequencyLimit,
                                                const unsigned int timescale) -> int {
  for (auto &m : melodies) {
    double amplitudeMultiplier = 1.0;
    if (m.HasMember("gain")) {
      if (!m["gain"].IsDouble()) {
        std::cerr << "Invalid HAPS input file: invalid melody gain" << std::endl;
        return EXIT_FAILURE;
      }

      amplitudeMultiplier = m["gain"].GetDouble();
      if (amplitudeMultiplier < 0.0) {
        std::cerr << "Invalid HAPS input file: melody gain is negative" << std::endl;
        return EXIT_FAILURE;
      }
    }

    if (m.HasMember("mute")) {
      if (!m["mute"].IsBool()) {
        std::cerr << "Invalid HAPS input file: invalid melody mute" << std::endl;
        return EXIT_FAILURE;
      }

      if (m["mute"].GetBool()) {
        amplitudeMultiplier = 0.0;
      }
    }

    haptics::types::Band *band = channel.generateBand(haptics::types::BandType::VectorialWave,
                                                      lowerFrequencyLimit, upperFrequencyLimit);
    if (!m.HasMember("notes")) {
      continue;
    }

    if (!m["notes"].IsArray()) {
      std::cerr << "Invalid HAPS input file: invalid melody.notes" << std::endl;
      return EXIT_FAILURE;
    }

    for (auto &n : m["notes"].GetArray()) {
      if (!n.IsObject()) {
        std::cerr << "Invalid HAPS input file: invalid note" << std::endl;
        return EXIT_FAILURE;
      }

      if (extractNote(n.GetObject(), band, amplitudeMultiplier, timescale) == EXIT_FAILURE) {
        std::cerr << "Invalid HAPS input file: impossible to encode a note" << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::extractNote(const rapidjson::Value::Object &note, types::Band *band,
                                            const double amplitudeMultiplier,
                                            const unsigned int timescale) -> int {
  if (!note.HasMember("position") || !note["position"].IsDouble()) {
    std::cerr << "Invalid HAPS input file: invalid note.position" << std::endl;
    return EXIT_FAILURE;
  }

  double position = note["position"].GetDouble();
  if (position < 0.0) {
    std::cerr << "Invalid HAPS input file: note.position is negative" << std::endl;
    return EXIT_FAILURE;
  }

  double amplitudeMultiplierWithGain = amplitudeMultiplier;
  if (note.HasMember("gain")) {
    if (!note["gain"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid note.gain" << std::endl;
      return EXIT_FAILURE;
    }

    double gain = note["gain"].GetDouble();
    if (gain < 0.0) {
      std::cerr << "Invalid HAPS input file: note.gain is negative" << std::endl;
      return EXIT_FAILURE;
    }

    amplitudeMultiplierWithGain *= gain;
  }

  if (note.HasMember("mute")) {
    if (!note["mute"].IsBool()) {
      std::cerr << "Invalid HAPS input file: invalid note.mute" << std::endl;
      return EXIT_FAILURE;
    }

    if (note["gain"].GetBool()) {
      amplitudeMultiplierWithGain = 0;
    }
  }

  float phase = 0;
  if (note.HasMember("phase")) {
    if (!note["phase"].IsFloat()) {
      std::cerr << "Invalid HAPS input file: invalid note.phase" << std::endl;
      return EXIT_FAILURE;
    }

    phase = note["phase"].GetFloat();
    if (phase < -1.0F || phase > 1.0F) {
      std::cerr << "Invalid HAPS input file: note.phase is not normalized" << std::endl;
      return EXIT_FAILURE;
    }

    phase = phase * static_cast<float>(2 * M_PI);
  }

  Waveform baseSignal = Waveform::Sine;
  if (note.HasMember("waveform")) {
    if (!note["waveform"].IsString()) {
      std::cerr << "Invalid HAPS input file: invalid note.waveform" << std::endl;
      return EXIT_FAILURE;
    }

    if (getWaveform(note["waveform"].GetString(), baseSignal) == EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: invalid note.waveform value" << std::endl;
      return EXIT_FAILURE;
    }
  }

  int timescaledPosition = secondsToTimeScale(note["position"].GetDouble(), timescale);
  haptics::types::Effect effect(timescaledPosition, phase,
                                haptics::types::BaseSignal(static_cast<int>(baseSignal)),
                                haptics::types::EffectType::Basis);
  std::optional<int> timescaledLength = std::nullopt;
  if (note.HasMember("length")) {
    if (!note["length"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid note.length" << std::endl;
      return EXIT_FAILURE;
    }

    timescaledLength = secondsToTimeScale(note["length"].GetDouble(), timescale);
  }

  int lastFrequencyValue = 0;
  int lastFrequencyTimestamp = 0;
  ModulationType pitchModulation;
  if (extractPitchInNote(note, effect, timescale, band->getLowerFrequencyLimit(),
                         band->getUpperFrequencyLimit(), lastFrequencyValue, lastFrequencyTimestamp,
                         pitchModulation) == EXIT_FAILURE) {
    std::cerr << "Invalid HAPS input file: invalid note.pitch" << std::endl;
    return EXIT_FAILURE;
  }

  ModulationType amplitudeModulation;
  if (extractAmplitudeInNote(
          note, effect, timescale,
          // use the pitch length if pitch is an interpolation curve, otherwise length sho
          pitchModulation == ModulationType::InterpolationCurve ? lastFrequencyTimestamp
                                                                : timescaledLength,
          amplitudeMultiplierWithGain, amplitudeModulation) == EXIT_FAILURE) {
    std::cerr << "Invalid HAPS input file: invalid note.amplitude" << std::endl;
    return EXIT_FAILURE;
  }

  // Check if both pitch and amplitude needed the length
  if (pitchModulation != ModulationType::InterpolationCurve &&
      amplitudeModulation != ModulationType::InterpolationCurve) {
    if (!timescaledLength.has_value()) {
      std::cerr << "Invalid HAPS input file: missing note.length for modulation different than "
                   "InterpolationCurve"
                << std::endl;
      return EXIT_FAILURE;
    }

    effect.addFrequencyAt(lastFrequencyValue, timescaledLength.value());
  }

  band->addEffect(effect);
  return EXIT_SUCCESS;
}

[[nodiscard]] auto
HapsEncoder::extractFrequencyRange(const rapidjson::Value::Object &vibrationTrack,
                                   int &lowerFrequencyLimit, int &upperFrequencyLimit) -> int {
  if (!vibrationTrack.HasMember("frequency_range")) {
    return EXIT_SUCCESS;
  }

  if (!vibrationTrack["frequency_range"].IsObject()) {
    std::cerr << "Invalid HAPS input file: invalid frequency_range" << std::endl;
    return EXIT_FAILURE;
  }

  rapidjson::Value::Object frequencyRange = vibrationTrack["frequency_range"].GetObject();
  if (!frequencyRange.HasMember("min") || !frequencyRange["min"].IsDouble()) {
    std::cerr << "Invalid HAPS input file: invalid frequency_range.min" << std::endl;
    return EXIT_FAILURE;
  }
  lowerFrequencyLimit = static_cast<int>(std::floor(frequencyRange["min"].GetDouble()));
  if (lowerFrequencyLimit < 0) {
    std::cerr << "Invalid HAPS input file: frequency_range.min is negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (!frequencyRange.HasMember("max") || !frequencyRange["max"].IsDouble()) {
    std::cerr << "Invalid HAPS input file: invalid frequency_range.max" << std::endl;
    return EXIT_FAILURE;
  }
  upperFrequencyLimit = static_cast<int>(std::ceil(frequencyRange["max"].GetDouble()));
  if (upperFrequencyLimit < lowerFrequencyLimit) {
    std::cerr << "Invalid HAPS input file: frequency_range.max is lower than frequency_range.min"
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::getWaveform(const std::string &waveform, Waveform &out) -> int {
  if (waveform == "Sine") {
    out = Waveform::Sine;
    return EXIT_SUCCESS;
  }
  if (waveform == "Square") {
    out = Waveform::Square;
    return EXIT_SUCCESS;
  }
  if (waveform == "Triangle") {
    out = Waveform::Triangle;
    return EXIT_SUCCESS;
  }
  if (waveform == "SawToothUp") {
    out = Waveform::SawToothUp;
    return EXIT_SUCCESS;
  }
  if (waveform == "SawToothDown") {
    out = Waveform::SawToothDown;
    return EXIT_SUCCESS;
  }
  if (waveform == "Constant") {
    out = Waveform::Constant;
    return EXIT_SUCCESS;
  }
  if (waveform == "Unknown") {
    out = Waveform::Unknown;
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

[[nodiscard]] auto HapsEncoder::extractPitchInNote(
    const rapidjson::Value::Object &note, types::Effect &effect, const unsigned int timescale,
    const int lowerFrequencyLimit, const int upperFrequencyLimit, int &lastFrequencyValue,
    int &lastFrequencyTimestamp, ModulationType &modulationType) -> int {
  if (!note.HasMember("pitch")) {
    modulationType = ModulationType::Constant;
    effect.addFrequencyAt(lowerFrequencyLimit, 0);
    lastFrequencyValue = lowerFrequencyLimit;
    lastFrequencyTimestamp = 0;
    return EXIT_SUCCESS;
  }

  if (note["pitch"].IsDouble()) {
    modulationType = ModulationType::Constant;
    int freq =
        computeAbsoluteFreq(lowerFrequencyLimit, upperFrequencyLimit, note["pitch"].GetDouble());
    effect.addFrequencyAt(freq, 0);
    lastFrequencyValue = freq;
    lastFrequencyTimestamp = 0;
    return EXIT_SUCCESS;
  }

  if (note["pitch"].IsObject()) {
    modulationType = ModulationType::InterpolationCurve;
    auto curve = note["pitch"].GetObject();
    if (extractFrequencyCurve(curve, effect, timescale, lowerFrequencyLimit, upperFrequencyLimit,
                              lastFrequencyValue, lastFrequencyTimestamp) == EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: invalid note.pitch curve" << std::endl;
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  modulationType = ModulationType(-1);
  std::cerr << "Invalid HAPS input file: invalid note.pitch" << std::endl;
  return EXIT_FAILURE;
}

[[nodiscard]] auto HapsEncoder::extractAmplitudeInNote(const rapidjson::Value::Object &note,
                                                       types::Effect &effect,
                                                       const unsigned int timescale,
                                                       std::optional<int> timescaledLength,
                                                       const double amplitudeMultiplier,
                                                       ModulationType &modulationType) -> int {
  if (!note.HasMember("amplitude")) {
    storeAmplitudeAsConstant(amplitudeMultiplier, effect, timescaledLength, modulationType);
    return EXIT_SUCCESS;
  }
  if (note["amplitude"].IsDouble()) {
    storeAmplitudeAsConstant(note["amplitude"].GetDouble() * amplitudeMultiplier, effect,
                             timescaledLength, modulationType);
    return EXIT_SUCCESS;
  }
  if (!note["amplitude"].IsObject()) {
    modulationType = ModulationType(-1);
    std::cerr << "Invalid HAPS input file: invalid note.amplitude" << std::endl;
    return EXIT_FAILURE;
  }

  auto amplitudeModulation = note["amplitude"].GetObject();
  bool amplitudeMemberPresent = amplitudeModulation.HasMember("amplitude");
  bool periodLengthMemberPresent = amplitudeModulation.HasMember("period_length");
  if (!amplitudeMemberPresent && !periodLengthMemberPresent) {
    modulationType = ModulationType::InterpolationCurve;
    auto curve = note["amplitude"].GetObject();
    if (extractAmplitudeCurve(amplitudeModulation, effect, timescale, amplitudeMultiplier) ==
        EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: invalid note.amplitude curve" << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  modulationType = ModulationType::PeriodicSignal;
  if (!amplitudeMemberPresent || !periodLengthMemberPresent) {
    std::cerr << "Invalid HAPS input file: amplitude and period_length are mandatory" << std::endl;
    return EXIT_FAILURE;
  }

  if (extractAmplitudeAsPeriodicSignal(amplitudeModulation, effect, timescale, timescaledLength,
                                       amplitudeMultiplier, modulationType) == EXIT_FAILURE) {
    std::cerr << "Invalid HAPS input file: invalid note.amplitude periodic signal" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto HapsEncoder::extractAmplitudeAsPeriodicSignal(
    const rapidjson::Value::Object &amplitudeModulation, types::Effect &effect,
    const unsigned int timescale, std::optional<int> timescaledLength,
    const double amplitudeMultiplier, ModulationType &modulationType) -> int {
  if (!amplitudeModulation["amplitude"].IsDouble()) {
    std::cerr << "Invalid HAPS input file: invalid note.amplitude.amplitude" << std::endl;
    return EXIT_FAILURE;
  }

  if (!amplitudeModulation["period_length"].IsDouble()) {
    std::cerr << "Invalid HAPS input file: invalid note.amplitude.period_length" << std::endl;
    return EXIT_FAILURE;
  }

  double amplitude = amplitudeModulation["amplitude"].GetDouble();
  if (amplitude < 0 || amplitude > 1) {
    std::cerr << "Invalid HAPS input file: note.amplitude.amplitude is not normalized" << std::endl;
    return EXIT_FAILURE;
  }

  double periodLength = amplitudeModulation["period_length"].GetDouble();
  if (periodLength <= 0) {
    std::cerr << "Invalid HAPS input file: note.amplitude.period_length lower than or equal 0"
              << std::endl;
    return EXIT_FAILURE;
  }

  double phase = 0.0;
  if (amplitudeModulation.HasMember("phase")) {
    if (!amplitudeModulation["phase"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid note.amplitude.phase" << std::endl;
      return EXIT_FAILURE;
    }

    double phase = amplitudeModulation["phase"].GetDouble();
    if (phase < -1.0 || phase > 1.0) {
      std::cerr << "Invalid HAPS input file: note.amplitude.phase is not normalized" << std::endl;
      return EXIT_FAILURE;
    }
  }

  double verticalOffset = 0.0;
  if (amplitudeModulation.HasMember("vertical_offset")) {
    if (!amplitudeModulation["vertical_offset"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid note.amplitude.vertical_offset" << std::endl;
      return EXIT_FAILURE;
    }

    double verticalOffset = amplitudeModulation["vertical_offset"].GetDouble();
    if (verticalOffset < -0.5 || verticalOffset > 0.5) {
      std::cerr << "Invalid HAPS input file: note.amplitude.vertical_offset is not normalized"
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  Waveform waveform = Waveform::Sine;
  if (amplitudeModulation.HasMember("waveform")) {
    if (!amplitudeModulation["waveform"].IsString()) {
      std::cerr << "Invalid HAPS input file: invalid note.amplitude.waveform" << std::endl;
      return EXIT_FAILURE;
    }

    if (getWaveform(amplitudeModulation["waveform"].GetString(), waveform) == EXIT_FAILURE) {
      std::cerr << "Invalid HAPS input file: invalid note.amplitude.waveform value" << std::endl;
      return EXIT_FAILURE;
    }
  }

  storeAmplitudeAsPeriodicSignal(effect, timescale, amplitudeMultiplier, timescaledLength, waveform,
                                 amplitude, verticalOffset, periodLength, phase);
  return EXIT_SUCCESS;
}

auto HapsEncoder::storeAmplitudeAsPeriodicSignal(
    types::Effect &effect, const unsigned int timescale, const double amplitudeMultiplier,
    std::optional<int> timescaledLength, const Waveform waveform, const double amplitude,
    const double verticalOffset, const double periodLength, const double phase) -> void {
  // Since the format doesn't allow a parametric representation of the modulation, an appromixation
  // of the amplitude modulation through a periodic signal is generated and keyframes are stored
  int duration = timescaledLength.value_or(0);
  if (duration < 0) {
    return;
  }

  if (waveform == Waveform::Constant) {
    storeAmplitudeAsConstant(amplitude + verticalOffset, effect, timescaledLength);
    return;
  }

  int step = std::max(1, millisecondsToTimeScale(10, timescale));
  if (step > duration) {
    step = duration;
  }

  for (int t = 0; t <= duration; t += step) {
    storeApproximatedAmplitudeKeyframe(effect, timescale, amplitudeMultiplier, t, waveform,
                                       amplitude, verticalOffset, periodLength, phase);
  }

  // Ensure the last keyframe is at the end
  if (duration % step != 0) {
    storeApproximatedAmplitudeKeyframe(effect, timescale, amplitudeMultiplier, duration, waveform,
                                       amplitude, verticalOffset, periodLength, phase);
  }
}

auto HapsEncoder::storeApproximatedAmplitudeKeyframe(
    types::Effect &effect, const unsigned int timescale, const double amplitudeMultiplier,
    const int t, const Waveform waveform, const double amplitude, const double verticalOffset,
    const double periodLength, const double phase) -> void {
  double normalizedTime = (static_cast<double>(t) / periodLength);
  double angle = 2 * M_PI * normalizedTime + phase;
  double value = 0.0;
  switch (waveform) {
  case Waveform::Sine:
    value = std::sin(angle);
    break;
  case Waveform::Square:
    value = std::sin(angle) >= 0.0 ? 1.0 : -1.0;
    break;
  case Waveform::Triangle:
    value = 2.0 * std::abs(2.0 * (normalizedTime - std::floor(normalizedTime + 0.5))) - 1.0;
    break;
  case Waveform::SawToothUp:
    value = 2.0 * (normalizedTime - std::floor(normalizedTime + 0.5));
    break;
  case Waveform::SawToothDown:
    value = 2.0 * (std::floor(normalizedTime + 0.5) - normalizedTime);
    break;
  default:
    // Impossible to approximate the waveform
    return;
  }

  float amplitudeValue =
      static_cast<float>(amplitudeMultiplier * (amplitude * value + verticalOffset));
  amplitudeValue = std::max(-1.0F, std::min(1.0F, amplitudeValue));
  effect.addAmplitudeAt(amplitudeValue, t);
}

auto HapsEncoder::storeAmplitudeAsConstant(const double amplitude, types::Effect &effect,
                                           std::optional<int> timescaledLength) -> void {
  ModulationType modulationType = ModulationType::Constant;
  storeAmplitudeAsConstant(amplitude, effect, timescaledLength, modulationType);
}

auto HapsEncoder::storeAmplitudeAsConstant(const double amplitude, types::Effect &effect,
                                           std::optional<int> timescaledLength,
                                           ModulationType &modulationType) -> void {
  modulationType = ModulationType::Constant;
  effect.addAmplitudeAt(static_cast<float>(amplitude), 0);
  if (timescaledLength.has_value()) {
    effect.addAmplitudeAt(static_cast<float>(amplitude), timescaledLength.value());
  }
}

[[nodiscard]] auto
HapsEncoder::extractFrequencyCurve(const rapidjson::Value::Object &curve, types::Effect &effect,
                                   const unsigned int timescale, const int lowerFrequencyLimit,
                                   const int upperFrequencyLimit, int &lastValue, int &lastPosition)
    -> int {
  double lastFreq = 0;
  int result = extractCurve(curve, effect, timescale, false, 1, lowerFrequencyLimit,
                            upperFrequencyLimit, lastFreq, lastPosition);
  if (result == EXIT_SUCCESS) {
    lastValue = computeAbsoluteFreq(lowerFrequencyLimit, upperFrequencyLimit, lastFreq);
  }
  return result;
}

[[nodiscard]] auto HapsEncoder::extractFrequencyCurve(const rapidjson::Value::Object &curve,
                                                      types::Effect &effect,
                                                      const unsigned int timescale,
                                                      const int lowerFrequencyLimit,
                                                      const int upperFrequencyLimit) -> int {
  return extractCurve(curve, effect, timescale, false, 1, lowerFrequencyLimit, upperFrequencyLimit);
}

[[nodiscard]] auto HapsEncoder::extractAmplitudeCurve(const rapidjson::Value::Object &curve,
                                                      types::Effect &effect,
                                                      const unsigned int timescale,
                                                      const double amplitudeModifier) -> int {
  return extractCurve(curve, effect, timescale, true, amplitudeModifier, 0, 0);
}

[[nodiscard]] auto HapsEncoder::extractAmplitudeCurve(const rapidjson::Value::Object &curve,
                                                      types::Effect &effect,
                                                      const unsigned int timescale,
                                                      const double amplitudeModifier,
                                                      double &lastValue, int &lastPosition) -> int {

  return extractCurve(curve, effect, timescale, true, amplitudeModifier, 0, 0, lastValue,
                      lastPosition);
}

[[nodiscard]] auto HapsEncoder::extractCurve(const rapidjson::Value::Object &curve,
                                             types::Effect &effect, const unsigned int timescale,
                                             const bool isAmplitude, const double amplitudeModifier,
                                             const int lowerFrequencyLimit,
                                             const int upperFrequencyLimit) -> int {
  double lastValue;
  int lastPosition;
  return extractCurve(curve, effect, timescale, isAmplitude, amplitudeModifier, lowerFrequencyLimit,
                      upperFrequencyLimit, lastValue, lastPosition);
}

[[nodiscard]] auto HapsEncoder::extractCurve(const rapidjson::Value::Object &curve,
                                             types::Effect &effect, const unsigned int timescale,
                                             const bool isAmplitude, const double amplitudeModifier,
                                             const int lowerFrequencyLimit,
                                             const int upperFrequencyLimit, double &lastValue,
                                             int &lastPosition) -> int {
  if (!curve.HasMember("keyframes")) {
    return EXIT_SUCCESS;
  }

  if (!curve["keyframes"].IsArray()) {
    std::cerr << "Invalid HAPS input file: invalid curve.keyframes" << std::endl;
    return EXIT_FAILURE;
  }

  for (auto &k : curve["keyframes"].GetArray()) {
    if (!k.IsObject()) {
      std::cerr << "Invalid HAPS input file: invalid keyframe" << std::endl;
      return EXIT_FAILURE;
    }

    if (!k.HasMember("position") || !k["position"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid keyframe.position" << std::endl;
      return EXIT_FAILURE;
    }

    double position = k["position"].GetDouble();
    if (!k.HasMember("value") || !k["value"].IsDouble()) {
      std::cerr << "Invalid HAPS input file: invalid keyframe.value" << std::endl;
      return EXIT_FAILURE;
    }

    lastPosition = secondsToTimeScale(position, timescale);
    lastValue = k["value"].GetDouble();
    if (isAmplitude) {
      effect.addAmplitudeAt(static_cast<float>(lastValue * amplitudeModifier), lastPosition);
    } else {
      int absoluteFreq = computeAbsoluteFreq(lowerFrequencyLimit, upperFrequencyLimit, lastValue);
      effect.addFrequencyAt(absoluteFreq, lastPosition);
    }
  }

  return EXIT_SUCCESS;
}

auto HapsEncoder::secondsToTimeScale(const double seconds, const unsigned int timescale) -> int {
  return static_cast<int>(std::round(seconds * static_cast<double>(timescale)));
}

auto HapsEncoder::millisecondsToTimeScale(const double milliseconds, const unsigned int timescale)
    -> int {
  return secondsToTimeScale(milliseconds * MS_2_S, timescale);
}

auto HapsEncoder::computeAbsoluteFreq(int lowerFrequencyLimit, int upperFrequencyLimit, double freq)
    -> int {
  return static_cast<int>(std::round(
      haptics::tools::genericNormalization(0.0, 1.0, static_cast<double>(lowerFrequencyLimit),
                                           static_cast<double>(upperFrequencyLimit), freq)));
}

} // namespace haptics::encoder
