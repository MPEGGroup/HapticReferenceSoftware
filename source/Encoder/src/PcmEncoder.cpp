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
#include <Encoder/include/WaveletEncoder.h>

using haptics::filterbank::Filterbank;
using haptics::types::EncodingModality;
using haptics::types::BaseSignal;
using haptics::tools::WavParser;
using haptics::types::BandType;
using haptics::types::Perception;
using haptics::types::Keyframe;
using haptics::types::Effect;
using haptics::types::Track;
using haptics::types::Band;
using haptics::encoder::WaveletEncoder;

namespace haptics::encoder {

auto PcmEncoder::encode(std::string &filename, const double curveFrequencyLimit, Perception &out) -> int {
  WavParser wavParser;
  wavParser.loadFile(filename);
  size_t numChannels = wavParser.getNumChannels();
  Track myTrack;
  if (out.getTracksSize() == 0) {
    for (int channelIndex = 0; channelIndex < numChannels; channelIndex++) {
      myTrack = Track(channelIndex, "I'm a placeholder", 1, 1, ~uint32_t(0));
      out.addTrack(myTrack);
    }
  } else if (out.getTracksSize() != numChannels) {
    return EXIT_FAILURE;
  }
  Band myBand;
  std::vector<double> signal;
  std::vector<std::pair<int, double>> points;
  Filterbank filterbank(static_cast<double>(wavParser.getSamplerate()));
  // init of wavelet encoding
  Band waveletBand;
  WaveletEncoder waveletEnc(BL_WAVELET_2KB, static_cast<int>(wavParser.getSamplerate()));
  std::vector<double> signal_wavelet;
  for (int channelIndex = 0; channelIndex < numChannels; channelIndex++) {
    myTrack = out.getTrackAt(channelIndex);
    signal = wavParser.getSamplesChannel(channelIndex);
    signal = filterbank.LP(signal, curveFrequencyLimit);
    points = PcmEncoder::localExtrema(signal, true);
    myBand = Band();
    if (PcmEncoder::convertToCurveBand(points, wavParser.getSamplerate(), curveFrequencyLimit,
                                       &myBand)) {
      myTrack.addBand(myBand);
    }
    //wavelet processing
    signal_wavelet = wavParser.getSamplesChannel(channelIndex);
    Filterbank filterbank2(static_cast<double>(wavParser.getSamplerate()));
    signal_wavelet = filterbank2.HP(signal_wavelet, curveFrequencyLimit);
    waveletBand = Band();
    if (waveletEnc.encodeSignal(signal_wavelet, BITBUDGET_WAVELET_2KB, curveFrequencyLimit,
                            waveletBand)) {
      myTrack.addBand(waveletBand);
    }
    out.replaceTrackAt(channelIndex, myTrack);
  }
  return EXIT_SUCCESS;
}

[[nodiscard]] auto PcmEncoder::convertToCurveBand(std::vector<std::pair<int, double>> &points,
                                                  const double samplerate,
                                                  const double curveFrequencyLimit, Band *out)
    -> bool {
  if (out == nullptr) {
    return false;
  }

  out->setBandType(BandType::Curve);
  out->setEncodingModality(EncodingModality::Quantized);
  out->setWindowLength(0);
  out->setLowerFrequencyLimit(0);
  out->setUpperFrequencyLimit((int)curveFrequencyLimit);
  Effect myEffect(0, 0, BaseSignal::Sine);
  Keyframe myKeyframe;
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
  }
  while (it != signal.end());

  if (includeBorder) {
    p = std::pair<int, double>(i, value);
    extremaIndexes.push_back(p);
  }

  return extremaIndexes;
}

} // namespace haptics::encoder
