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

auto WaveletDecoder::decodeBand(Band &band, int timescale) -> std::vector<double> {
  if (band.getBandType() != BandType::WaveletWave || !band.getBlockLength().has_value()) {
    return std::vector<double>();
  }
  size_t numBlocks = band.getEffectsSize();
  int bl = band.getBlockLength().value() * band.getUpperFrequencyLimit() / timescale;
  int dwtlevel = (int)log2((double)bl / 4);
  std::vector<double> sig_rec(numBlocks * bl, 0);

  for (uint32_t b = 0; b < numBlocks; b++) {
    Effect effect = band.getEffectAt((int)b);
    auto bitstream = effect.getWaveletBitstream();
    std::vector<int> block_dwt(bl, 0);
    double scalar = 0;
    int bits = 0;
    spihtDec.decodeEffect(bitstream, block_dwt, bl, scalar, bits);
    std::vector<double> block_time(bl);
    decodeBlock(block_dwt, block_time, scalar, dwtlevel);
    std::copy(block_time.begin(), block_time.end(), sig_rec.begin() + effect.getPosition());
  }
  return sig_rec;
}

void WaveletDecoder::transformBand(Band &band, unsigned int timescale) {

  if (band.getBandType() != BandType::WaveletWave) {
    return;
  }
  size_t numBlocks = band.getEffectsSize();
  auto bl = (int)(band.getBlockLengthOrDefault() * MS_2_S_WAVELET *
                  (double)band.getUpperFrequencyLimit());
  int dwtlevel = (int)log2((double)bl / 4);

  for (uint32_t b = 0; b < numBlocks; b++) {
    Effect effect = band.getEffectAt((int)b);
    auto bitstream = effect.getWaveletBitstream();
    std::vector<int> block_dwt(bl, 0);
    double scalar = 0;
    int bits = 0;
    spihtDec.decodeEffect(bitstream, block_dwt, bl, scalar, bits);
    scalar /= pow(2, (double)bits);
    Effect newEffect;
    newEffect.setPosition(
        (int)((double)b * (double)bl * (double)timescale / (double)band.getUpperFrequencyLimit()));
    auto block_time = std::vector<double>();
    decodeBlock(block_dwt, block_time, scalar, dwtlevel);
    newEffect.setWaveletSamples(block_time);
    band.replaceEffectAt((int)b, newEffect);
  }
}

void WaveletDecoder::decodeBlock(std::vector<int> &block_dwt, std::vector<double> &block_time,
                                 double scalar, int dwtl) {

  Wavelet wavelet;
  std::vector<double> block_scaled(block_dwt.size(), 0);
  std::transform(block_dwt.begin(), block_dwt.end(), block_scaled.begin(),
                 [scalar](int d) -> double { return (double)d * scalar; });
  block_time.resize(block_dwt.size());
  wavelet.inv_DWT(block_scaled, dwtl, block_time);
}

} // namespace haptics::waveletdecoder
