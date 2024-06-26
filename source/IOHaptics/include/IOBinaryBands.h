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

#ifndef IOBINARYBANDS_H
#define IOBINARYBANDS_H

// #include <IOHaptics>
#include <Spiht/include/Spiht_Dec.h>
#include <Spiht/include/Spiht_Enc.h>
#include <Types/include/Band.h>

#include <cmath>

namespace haptics::io {

constexpr int WAVELET_BL_FACTOR = 32;
constexpr int S2MS = 1000;

class IOBinaryBands {
public:
  static auto writeBandHeader(types::Band &band, std::vector<bool> &output, unsigned int timescale)
      -> bool;
  static auto writeBandBody(types::Band &band, std::vector<bool> &output) -> bool;
  static auto writeWaveletEffect(types::Effect &effect, std::vector<bool> &output) -> bool;

  static auto readBandHeader(types::Band &band, std::istream &file, std::vector<bool> &unusedBits,
                             unsigned int timescale) -> bool;

  static auto readBandBody(types::Band &band, std::istream &file, std::vector<bool> &unusedBits,
                           unsigned int timescale) -> bool;
  static auto readBandBodyBool(types::Band &band, std::vector<bool> &bitstream) -> bool;
  static auto readWaveletEffect(types::Effect &effect, std::vector<bool> &bitstream, int &idx)
      -> bool;

private:
  static auto writeTransientEffect(types::Effect &effect, std::vector<bool> &output) -> bool;
  static auto writeCurveEffect(types::Effect &effect, std::vector<bool> &output) -> bool;
  static auto writeVectorialEffect(types::Effect &effect, std::vector<bool> &output) -> bool;
  static auto writeReferenceEffect(types::Effect &effect, std::vector<bool> &output) -> bool;
  static auto writeTimelineEffect(types::Effect &effect, types::Band &band,
                                  std::vector<bool> &output) -> bool;

  static auto readTransientEffect(types::Effect &effect, std::istream &file,
                                  std::vector<bool> &unusedBits) -> bool;
  static auto readCurveEffect(types::Effect &effect, std::istream &file,
                              std::vector<bool> &unusedBits) -> bool;
  static auto readVectorialEffect(types::Effect &effect, std::istream &file,
                                  std::vector<bool> &unusedBits) -> bool;
  static auto readVectorialEffect(types::Effect &effect, int &idx, std::vector<bool> &bitstream)
      -> bool;
  static auto readWaveletEffect(types::Effect &effect, std::istream &file,
                                std::vector<bool> &unusedBits) -> bool;
  static auto readReferenceEffect(types::Effect &effect, std::istream &file,
                                  std::vector<bool> &unusedBits) -> bool;
  static auto readReferenceEffect(types::Effect &effect, int &idx, std::vector<bool> &bitstream)
      -> bool;
  static auto readTimelineEffect(types::Effect &effect, types::Band &band, std::istream &file,
                                 std::vector<bool> &unusedBits) -> bool;
  static auto readTimelineEffect(types::Effect &effect, types::Band &band, int &idx,
                                 std::vector<bool> &bitstream) -> bool;
};
} // namespace haptics::io
#endif // IOBINARYBANDS_H