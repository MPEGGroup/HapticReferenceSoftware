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

using haptics::tools::WavParser;

namespace haptics::encoder {

auto PcmEncoder::encode(std::string &filename) -> int {
  WavParser wavParser;
  wavParser.loadFile(filename);

  std::cout << std::endl << std::endl;
  std::cout << wavParser.getSamplerate() << std::endl;
  std::cout << wavParser.getNumSamples() << std::endl;
  std::cout << wavParser.getNumChannels() << std::endl;

  std::vector<std::vector<std::pair<int16_t, double>>> extractedKeyframes;
  size_t channelIndex = 0;
  for (; channelIndex < wavParser.getNumChannels(); channelIndex++) {
    extractedKeyframes.push_back(PcmEncoder::localExtrema(wavParser.getSamplesChannel(channelIndex), true));
  }

  return EXIT_SUCCESS;
}


[[nodiscard]] auto PcmEncoder::localExtrema(std::vector<double> signal, bool includeBorder)
    -> std::vector<std::pair<int16_t, double>> {
  std::vector<std::pair<int16_t, double>> extremaIndexes;
  
  auto it = signal.begin();
  if (it == signal.end()) {
    return {};
  }

  double lastValue = *it;
  ++it;
  if (it == signal.end()) {
    if (includeBorder) {
      std::pair<int16_t, double> p1(0, signal[0]);
      std::pair<int16_t, double> p2(0, signal[0]);
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
      std::pair<int16_t, double> p1(0, signal[0]);
      std::pair<int16_t, double> p2(1, signal[1]);
      extremaIndexes.push_back(p1);
      extremaIndexes.push_back(p2);
      return extremaIndexes;
    }
    return {};
  }

  std::pair<int16_t, double> p;
  int16_t i = 1;
  double nextValue = 0;
  if (includeBorder) {
    p = std::pair<int16_t, double>(0, signal[0]);
    extremaIndexes.push_back(p);
  }
  do {
    nextValue = *it;
    if (((value >= lastValue && value >= nextValue) ||
         (value <= lastValue && value <= nextValue)) &&
        (value != lastValue || value != nextValue)) {

      p = std::pair<int16_t, double>(i, value);
      extremaIndexes.push_back(p);
    }

    lastValue = value;
    value = nextValue;
    ++it;
    i++;
  }
  while (it != signal.end());

  if (includeBorder) {
    p = std::pair<int16_t, double>(i, value);
    extremaIndexes.push_back(p);
  }

  return extremaIndexes;
}

} // namespace haptics::encoder
