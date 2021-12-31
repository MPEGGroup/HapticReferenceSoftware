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

#include "../include/WaveletDecoder.h"

namespace haptics::waveletdecoder {

WaveletDecoder::WaveletDecoder(Band band)
    : fs(band.getUpperFrequencyLimit())
    , bl(band.getWindowLength())
    , dwtlevel((int)log2((double)band.getWindowLength() / 4)) {

  Wavelet wavelet;
  size_t numBlocks = band.getEffectsSize();
  sig_rec.resize(numBlocks * bl, 0);

  for (int b = 0; b < numBlocks; b++) {
    Effect effect = band.getEffectAt(b);
    std::vector<double> block_dwt(bl, 0);
    Keyframe keyframe = effect.getKeyframeAt(bl);
    auto scalar = (double)keyframe.getAmplitudeModulation().value();
    //std::cout << "scalar decoder: " << scalar << std::endl;
    for (int i = 0; i < bl; i++) {
      Keyframe keyframe = effect.getKeyframeAt(i);
      block_dwt[i] = (double)keyframe.getAmplitudeModulation().value() * scalar;
    }

    /*for (auto v : block_dwt) {
      std::cout << v << std::endl;
    }*/

    std::vector<double> block_time(bl, 0);

    wavelet.inv_DWT(block_dwt, dwtlevel, block_time);
    std::copy(block_time.begin(), block_time.end(), sig_rec.begin() + effect.getPosition());
  }
}

auto WaveletDecoder::getSignal() -> std::vector<double> { return sig_rec; }

auto WaveletDecoder::getSignalSize() -> size_t { return sig_rec.size(); }

auto WaveletDecoder::getSampleAt(int pos) -> double { return sig_rec.at(pos); }

} // namespace haptics::waveletdecoder
