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
#include <Encoder/include/WaveletEncoder.h>
#include <Tools/include/Tools.h>

using haptics::encoder::WaveletEncoder;
using haptics::filterbank::Filterbank;
using haptics::filterbank::FourierTools;
using haptics::tools::WavParser;
using haptics::types::Band;
using haptics::types::BandType;
using haptics::types::BaseSignal;
using haptics::types::Effect;
using haptics::types::EncodingModality;
using haptics::types::Keyframe;
using haptics::types::Perception;
using haptics::types::Track;

namespace haptics::encoder {

auto PcmEncoder::encode(std::string &filename, EncodingConfig &config, Perception &out) -> int {
  WavParser wavParser;
  wavParser.loadFile(filename);
  size_t numChannels = wavParser.getNumChannels();
  Track myTrack;
  if (out.getTracksSize() == 0) {
    for (uint32_t channelIndex = 0; channelIndex < numChannels; channelIndex++) {
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
    filteredSignal = filterbank.LP(signal, config.curveFrequencyLimit);
    points = PcmEncoder::localExtrema(filteredSignal, true);
    myBand = Band();
    if (PcmEncoder::convertToCurveBand(points, wavParser.getSamplerate(),
                                       config.curveFrequencyLimit, &myBand)) {
      myTrack.addBand(myBand);
    }
    out.replaceTrackAt((int)channelIndex, myTrack);

    // wavelet processing
    signal_wavelet = wavParser.getSamplesChannel(channelIndex);
    Filterbank filterbank2(static_cast<double>(wavParser.getSamplerate()));
    signal_wavelet = filterbank2.HP(signal_wavelet, config.curveFrequencyLimit);
    waveletBand = Band();
    if (waveletEnc.encodeSignal(signal_wavelet, config.wavelet_bitbudget,
                                config.curveFrequencyLimit, waveletBand)) {
      myTrack.addBand(waveletBand);
    }
    // WAVE BANDS
    /*for (std::pair<double, double> frequencyLimits : config.frequencyBandLimits) {
        myBand = Band();
        if (PcmEncoder::encodeIntoWaveBand(signal, filterbank, wavParser.getSamplerate(),
            frequencyLimits, config, &myBand)) {
            myTrack.addBand(myBand);
        }
    }*/
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
  out->setEncodingModality(EncodingModality::Quantized);
  out->setWindowLength(0);
  out->setLowerFrequencyLimit(0);
  out->setUpperFrequencyLimit((int)curveFrequencyLimit);
  Effect myEffect(0, 0, BaseSignal::Sine);
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
[[nodiscard]] auto
PcmEncoder::encodeIntoWaveBand(std::vector<double> &signal, Filterbank &filterbank,
                               const double samplerate,
                               const std::pair<double, double> frequencyBandLimits,
                               EncodingConfig &config, Band *out) -> bool {
  if (out == nullptr) {
    return false;
  }

  int windowSampleCount = (int)std::round(config.windowLength * MS_2_S * samplerate);
  if (windowSampleCount == 0) {
    return false;
  }

  int upperFrequency = (int)std::round(frequencyBandLimits.second);
  int LowerFrequency = (int)std::round(frequencyBandLimits.first);
  std::vector<double> filteredSignal = filterbank.HP(signal, LowerFrequency);
  filteredSignal = filterbank.LP(filteredSignal, upperFrequency);
  if (filteredSignal.empty()) {
    return false;
  }

  out->setBandType(BandType::Wave);
  out->setEncodingModality(EncodingModality::Quantized);
  out->setLowerFrequencyLimit(LowerFrequency);
  out->setUpperFrequencyLimit(upperFrequency);
  out->setWindowLength(config.windowLength);

  Effect myEffect;
  bool generateNewEffect = true;
  std::vector<double> windowedSignal;
  for (uint32_t startingWindowIndex = 0; startingWindowIndex < filteredSignal.size();
       startingWindowIndex += windowSampleCount) {
    if (startingWindowIndex + windowSampleCount < filteredSignal.size()) {
      windowedSignal =
          std::vector<double>(filteredSignal.begin() + startingWindowIndex,
                              filteredSignal.begin() + startingWindowIndex + windowSampleCount);
    } else {
      windowedSignal =
          std::vector<double>(filteredSignal.begin() + startingWindowIndex, filteredSignal.end());
    }
    std::valarray<std::complex<double>> fftValue;
    if (!FourierTools::FFT(windowedSignal, fftValue)) {
      continue;
    }

    auto *maxIt =
        std::max_element(std::begin(fftValue), std::end(fftValue),
                         [](std::complex<double> e1, std::complex<double> e2) {
                           return FourierTools::GetAmplitude(e1) < FourierTools::GetAmplitude(e2);
                         });
    int maxIndex = static_cast<int>(maxIt - std::begin(fftValue));
    if (tools::is_eq(FourierTools::GetAmplitude(*maxIt), 0)) {
      generateNewEffect = true;
      out->addEffect(myEffect);
    } else {
      if (generateNewEffect) {
        myEffect = Effect((int)(std::round(S_2_MS * startingWindowIndex / samplerate)),
                          (float)FourierTools::GetPhase(*maxIt), BaseSignal::Sine);
        generateNewEffect = false;
      }
      int f = (int)std::round(
          FourierTools::GetFrequency(maxIndex, static_cast<int>(fftValue.size()), samplerate));
      if (LowerFrequency < f && f <= upperFrequency) {
        myEffect.addKeyframe(std::nullopt, (float)FourierTools::GetAmplitude(*maxIt), f);
      } else {
        generateNewEffect = true;
        if (myEffect.getKeyframesSize() != 0) {
          out->addEffect(myEffect);
        }
      }
    }
  }
  if (!generateNewEffect) {
    out->addEffect(myEffect);
  }

  return out->getEffectsSize() != 0;
}

} // namespace haptics::encoder
