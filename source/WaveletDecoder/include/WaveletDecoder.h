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

#ifndef WAVELETDECODER_H
#define WAVELETDECODER_H

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

#include "FilterBank/include/Wavelet.h"
#include "Spiht/include/Spiht_Dec.h"
#include "Types/include/Band.h"
#include "Types/include/Effect.h"
#include "Types/include/Keyframe.h"

using haptics::filterbank::Wavelet;
using haptics::spiht::Spiht_Dec;
using haptics::types::Band;
using haptics::types::BandType;
using haptics::types::Effect;

namespace haptics::waveletdecoder {

constexpr double MS_2_S_WAVELET = 0.001;

class WaveletDecoder {
public:
  auto decodeBand(Band &band, int timescale) -> std::vector<double>;
  void transformBand(Band &band, unsigned int timescale);
  void static decodeBlock(std::vector<int> &block_dwt, std::vector<double> &block_time,
                          double scalar, int dwtl);

private:
  std::vector<double> sig_rec;
  Spiht_Dec spihtDec = Spiht_Dec();
};
} // namespace haptics::waveletdecoder
#endif // WAVELETDECODER_H
