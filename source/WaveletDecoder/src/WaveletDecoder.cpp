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

auto WaveletDecoder::decodeBand(Band &band) -> std::vector<double> {
  size_t numBlocks = band.getEffectsSize();
  int bl = (int)((double)band.getWindowLength() * MS_2_S_WAVELET *
                 (double)band.getUpperFrequencyLimit());
  int dwtlevel = (int)log2((double)bl / 4);
  std::vector<double> sig_rec(numBlocks * bl, 0);

  for (uint32_t b = 0; b < numBlocks; b++) {
    Effect effect = band.getEffectAt((int)b);
    std::vector<double> block_dwt(bl, 0);
    Keyframe keyframe = effect.getKeyframeAt(bl);
    auto scalar = (double)keyframe.getAmplitudeModulation().value();
    for (int i = 0; i < bl; i++) {
      Keyframe keyframe = effect.getKeyframeAt(i);
      block_dwt[i] = (double)keyframe.getAmplitudeModulation().value();
    }

    std::vector<double> block_time = decodeBlock(block_dwt, scalar, dwtlevel);
    std::copy(block_time.begin(), block_time.end(), sig_rec.begin() + effect.getPosition());
  }
  return sig_rec;
}

void WaveletDecoder::transformBand(Band &band) {

  if (band.getEncodingModality() != EncodingModality::Wavelet) {
    return;
  }
  size_t numBlocks = band.getEffectsSize();
  int bl = (int)((double)band.getWindowLength() * MS_2_S_WAVELET *
                 (double)band.getUpperFrequencyLimit());
  int dwtlevel = (int)log2((double)bl / 4);

  for (uint32_t b = 0; b < numBlocks; b++) {
    Effect effect = band.getEffectAt((int)b);
    std::vector<double> block_dwt(bl, 0);
    Keyframe keyframe = effect.getKeyframeAt(bl);
    auto scalar = (double)keyframe.getAmplitudeModulation().value();
    for (int i = 0; i < bl; i++) {
      Keyframe keyframe = effect.getKeyframeAt(i);
      block_dwt[i] = (double)keyframe.getAmplitudeModulation().value();
    }

    std::vector<double> block_time = decodeBlock(block_dwt, scalar, dwtlevel);

    Effect newEffect;
    newEffect.setPosition(effect.getPosition());
    for (int i = 0; i < bl; i++) {
      Keyframe keyframe;
      keyframe.setAmplitudeModulation(block_time[i]);
      keyframe.setRelativePosition(i);
      newEffect.addKeyframe(keyframe);
    }
    band.replaceEffectAt((int)b, newEffect);
  }
}

auto WaveletDecoder::decodeBlock(std::vector<double> &block_dwt, double scalar, int dwtl)
    -> std::vector<double> {

  Wavelet wavelet;
  std::vector<double> block_scaled(block_dwt.size(), 0);
  std::transform(block_dwt.begin(), block_dwt.end(), block_scaled.begin(),
                 [scalar](double d) -> double { return d * scalar; });
  std::vector<double> block_time(block_dwt.size(), 0);
  wavelet.inv_DWT(block_scaled, dwtl, block_time);
  return block_time;
}

} // namespace haptics::waveletdecoder
