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

#include <Encoder/include/PcmEncoder.h>
#include <Tools/include/Tools.h>
#include <Types/include/CurveType.h>

using haptics::encoder::WaveletEncoder;
using haptics::filterbank::Filterbank;
using haptics::tools::akimaInterpolation;
using haptics::tools::bezierInterpolation;
using haptics::tools::bsplineInterpolation;
using haptics::tools::cubicInterpolation;
using haptics::tools::linearInterpolation2;
using haptics::tools::WavParser;
using haptics::types::Band;
using haptics::types::BandType;
using haptics::types::BaseSignal;
using haptics::types::CurveType;
using haptics::types::Effect;
using haptics::types::EffectType;
using haptics::types::Keyframe;
using haptics::types::Perception;
using haptics::types::Track;

namespace haptics::encoder {

auto PcmEncoder::encode(std::string &filename, EncodingConfig &config, Perception &out) -> int {
  WavParser wavParser;
  wavParser.loadFile(filename);
  size_t numChannels = wavParser.getNumChannels();
  Track myTrack;
  auto tracksSize = out.getTracksSize();
  if (tracksSize < numChannels) {
    for (uint32_t channelIndex = tracksSize; channelIndex < numChannels; channelIndex++) {
      myTrack = Track((int)channelIndex, "I'm a placeholder", 1, 1, ~uint32_t(0));
      out.addTrack(myTrack);
    }
  } else if (out.getTracksSize() != numChannels) {
    return EXIT_FAILURE;
  }

  Band myBand;
  std::vector<double> signal;
  std::vector<double> filteredSignal;
  std::vector<std::pair<int, double>> points;
  std::vector<double> interpolationSignal;
  Filterbank filterbank(static_cast<double>(wavParser.getSamplerate()));
  // init of wavelet encoding
  Band waveletBand;
  WaveletEncoder waveletEnc(config.wavelet_windowLength,
                            static_cast<int>(wavParser.getSamplerate()));
  std::vector<double> signal_wavelet;
  for (uint32_t channelIndex = 0; channelIndex < numChannels; channelIndex++) {
    myTrack = out.getTrackAt((int)channelIndex);
    signal = wavParser.getSamplesChannel(channelIndex);

    // CURVE BAND
    if (config.curveFrequencyLimit > 0) {
      if (out.getPerceptionModality() == types::PerceptionModality::VibrotactileTexture ||
          out.getPerceptionModality() == types::PerceptionModality::Stiffness) {
        filteredSignal = signal;
      } else {
        filteredSignal = filterbank.LP(signal, config.curveFrequencyLimit);
      }

      points = PcmEncoder::localExtrema(filteredSignal, true);
      myBand = Band();
      if (PcmEncoder::convertToCurveBand(points, wavParser.getSamplerate(),
                                         config.curveFrequencyLimit, &myBand)) {
        if (out.getPerceptionModality() == types::PerceptionModality::Force ||
            out.getPerceptionModality() == types::PerceptionModality::Stiffness) {
          myBand.setCurveType(CurveType::Linear);
        } else if (out.getPerceptionModality() == types::PerceptionModality::Vibrotactile ||
                   out.getPerceptionModality() == types::PerceptionModality::VibrotactileTexture) {
          myBand.setCurveType(CurveType::Cubic);
        } else {
          myBand.setCurveType(CurveType::Unknown);
        }
        myTrack.addBand(myBand);
      }
    }
    myTrack.setFrequencySampling(wavParser.getSamplerate());
    myTrack.setSampleCount(
        static_cast<uint32_t>(wavParser.getNumSamples() / wavParser.getNumChannels()));

    // wavelet processing
    if (out.getPerceptionModality() == types::PerceptionModality::Vibrotactile ||
        out.getPerceptionModality() == types::PerceptionModality::Other) {

      signal_wavelet = wavParser.getSamplesChannel(channelIndex);

      if (config.curveFrequencyLimit > 0) {
        switch (myBand.getCurveType()) {
        case CurveType::Linear:
          interpolationSignal = linearInterpolation2(points);
          break;
        case CurveType::Cubic:
          interpolationSignal = cubicInterpolation(points);
          break;
        case CurveType::Akima:
          interpolationSignal = akimaInterpolation(points);
          break;
        case CurveType::Bezier:
          interpolationSignal = bezierInterpolation(points);
          break;
        case CurveType::Bspline:
          interpolationSignal = bsplineInterpolation(points);
          break;
        default:
          interpolationSignal = cubicInterpolation(points);
          break;
        } // supposing only one effect with initial value 0

        std::vector<double> differenceSignal;
        for (size_t i = 0; i < filteredSignal.size(); i++) {
          double difference = filteredSignal[i] - interpolationSignal[i];
          differenceSignal.push_back(difference);
        }

        Filterbank filterbank2(static_cast<double>(wavParser.getSamplerate()));
        signal_wavelet = filterbank2.HP(signal_wavelet, config.curveFrequencyLimit);

        for (size_t i = 0; i < signal_wavelet.size(); i++) {
          signal_wavelet[i] += differenceSignal[i];
        }
      }

      waveletBand = Band();
      if (waveletEnc.encodeSignal(signal_wavelet, config.wavelet_bitbudget,
                                  config.curveFrequencyLimit, waveletBand)) {
        myTrack.addBand(waveletBand);
      }
      myTrack.setFrequencySampling(wavParser.getSamplerate());
      myTrack.setSampleCount(
          static_cast<uint32_t>(wavParser.getNumSamples() / wavParser.getNumChannels()));
    }

    out.replaceTrackAt((int)channelIndex, myTrack);
  }

  return EXIT_SUCCESS;
}

[[nodiscard]] auto PcmEncoder::convertToCurveBand(std::vector<std::pair<int, double>> &points,
                                                  const double samplerate,
                                                  const double curveFrequencyLimit, Band *out)
    -> bool {
  if (out == nullptr || curveFrequencyLimit <= 0) {
    return false;
  }

  out->setBandType(BandType::Curve);
  out->setCurveType(CurveType::Cubic);
  // out->setEncodingModality(EncodingModality::Wavelet);
  out->setWindowLength(0);
  out->setLowerFrequencyLimit(0);
  out->setUpperFrequencyLimit((int)curveFrequencyLimit);
  Effect myEffect(0, 0, BaseSignal::Sine, EffectType::Basis);
  for (std::pair<int, double> p : points) {
    std::optional<int> f;
    myEffect.addKeyframe(static_cast<int>(S_2_MS * p.first / samplerate), p.second, f);
  }
  out->addEffect(myEffect);

  return true;
}

[[nodiscard]] auto PcmEncoder::localExtrema(std::vector<double> signal, bool includeBorder)
    -> std::vector<std::pair<int, double>> {
  std::vector<std::pair<int, double>> extremaIndexes;

  auto it = signal.begin();
  if (it == signal.end()) {
    return {};
  }

  double lastValue = *it;
  ++it;
  if (it == signal.end()) {
    if (includeBorder) {
      std::pair<int, double> p1(0, signal[0]);
      std::pair<int, double> p2(0, signal[0]);
      extremaIndexes.push_back(p1);
      extremaIndexes.push_back(p2);
      return extremaIndexes;
    }
    return {};
  }

  double value = *it;
  ++it;
  if (it == signal.end()) {
    if (includeBorder) {
      std::pair<int, double> p1(0, signal[0]);
      std::pair<int, double> p2(1, signal[1]);
      extremaIndexes.push_back(p1);
      extremaIndexes.push_back(p2);
      return extremaIndexes;
    }
    return {};
  }

  std::pair<int, double> p;
  int i = 1;
  double nextValue = 0;
  if (includeBorder) {
    p = std::pair<int, double>(0, signal[0]);
    extremaIndexes.push_back(p);
  }
  do {
    nextValue = *it;
    if (((value >= lastValue && value >= nextValue) ||
         (value <= lastValue && value <= nextValue)) &&
        !(haptics::tools::is_eq(value, lastValue) && haptics::tools::is_eq(value, nextValue))) {
      p = std::pair<int, double>(i, value);
      extremaIndexes.push_back(p);
    }

    lastValue = value;
    value = nextValue;
    ++it;
    i++;
  } while (it != signal.end());

  if (includeBorder) {
    p = std::pair<int, double>(i, value);
    extremaIndexes.push_back(p);
  }

  return extremaIndexes;
}
} // namespace haptics::encoder
