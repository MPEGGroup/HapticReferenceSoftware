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

#ifndef FOURIERTOOLS_H
#define FOURIERTOOLS_H

#include <complex>
#include <valarray>
#include <vector>

namespace haptics::filterbank {

class FourierTools {
public:
  auto static FFT(std::vector<double> &in, std::valarray<std::complex<double>> &out) -> bool;
  auto static FFT(std::valarray<std::complex<double>> &in) -> bool;

  auto static GetAmplitude(std::complex<double> c) -> double;
  auto static GetPhase(std::complex<double> c) -> double;
  auto static GetFrequency(int index, int fftSize, double samplerate) -> double;

private:
  static const uint32_t SHIFTBITS_1 = 1;
  static const uint32_t SHIFTBITS_2 = 2;
  static const uint32_t SHIFTBITS_4 = 4;
  static const uint32_t SHIFTBITS_8 = 8;
  static const uint32_t SHIFTBITS_16 = 16;
  static const uint32_t SHIFTBITS_32 = 32;
  static const unsigned int BINARYMASK_1 = 0xaaaaaaaa;
  static const unsigned int BINARYMASK_INVERSE_1 = 0x55555555;
  static const unsigned int BINARYMASK_2 = 0xcccccccc;
  static const unsigned int BINARYMASK_INVERSE_2 = 0x33333333;
  static const unsigned int BINARYMASK_4 = 0xf0f0f0f0;
  static const unsigned int BINARYMASK_INVERSE_4 = 0x0f0f0f0f;
  static const unsigned int BINARYMASK_8 = 0xff00ff00;
  static const unsigned int BINARYMASK_INVERSE_8 = 0x00ff00ff;
};
} // namespace haptics::filterbank
#endif // FOURIERTOOLS_H
