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

#ifndef PCMENCODER_H
#define PCMENCODER_H

#include <FilterBank/include/Filterbank.h>
#include <Tools/include/WavParser.h>
#include <Types/include/Band.h>
#include <Types/include/Effect.h>
#include <Types/include/Keyframe.h>
#include <Types/include/Perception.h>
#include <Types/include/Track.h>
#include <iostream>
#include <utility>
#include <vector>

namespace haptics::encoder {

static constexpr int BITBUDGET_2KBS = 4;
static constexpr int BITBUDGET_16KBS = 13;
static constexpr int BITBUDGET_64KBS = 88;

struct EncodingConfig {
  double curveFrequencyLimit = 0;
  int wavelet_windowLength = 0;
  int wavelet_bitbudget = 0;

  explicit EncodingConfig() = default;
  explicit EncodingConfig(double _curveFrequencyLimit, int _wavelet_windowLength,
                          int _wavelet_bitbudget)
      : curveFrequencyLimit(_curveFrequencyLimit)
      , wavelet_windowLength(_wavelet_windowLength)
      , wavelet_bitbudget(_wavelet_bitbudget){};

  auto static generateConfig(int bitrate = 2) -> EncodingConfig {

    const double curveFrequencyLimit = 72.5;
    const int wavelet_windowLength = 512;
    int wavelet_bitbudget = 0;
    switch (bitrate) {
    case 2:
      wavelet_bitbudget = BITBUDGET_2KBS;
      break;
    case 16:
      wavelet_bitbudget = BITBUDGET_16KBS;
      break;
    case 64:
      wavelet_bitbudget = BITBUDGET_64KBS;
      break;
    default:
      std::cout << "bitrate not supported, switching to 2 kb/s" << std::endl;
      wavelet_bitbudget = BITBUDGET_2KBS;
      break;
    }

    return EncodingConfig(curveFrequencyLimit, wavelet_windowLength, wavelet_bitbudget);
  }
};

class PcmEncoder {
public:
  auto static encode(std::string &filename, EncodingConfig &config, types::Perception &out) -> int;
  [[nodiscard]] auto static convertToCurveBand(std::vector<std::pair<int, double>> &points,
                                               double samplerate, double curveFrequencyLimit,
                                               haptics::types::Band *out) -> bool;
  [[nodiscard]] auto static localExtrema(std::vector<double> signal, bool includeBorder)
      -> std::vector<std::pair<int, double>>;
};
} // namespace haptics::encoder
#endif // PCMENCODER_H
