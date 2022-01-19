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

#include <FilterBank/include/FourierTools.h>
#include <algorithm>
#include <cmath>

namespace haptics::filterbank {

auto FourierTools::FFT(std::vector<double> &in, std::valarray<std::complex<double>> &out) -> bool {
  out.resize(in.size());
  auto it = in.begin();
  for (size_t i = 0; i < in.size(); i++) {
    out[i] = std::complex<double>(*it, 0);
    it++;
  }

  return FourierTools::FFT(out);
}

auto FourierTools::FFT(std::valarray<std::complex<double>> &in) -> bool {
  int size = static_cast<int>(in.size());
  if (size == 0) {
    return false;
  }

  int zeroCount = 0;
  while (((size + zeroCount) & ((size + zeroCount) - 1)) != 0) {
    zeroCount++;
  }
  std::vector<std::complex<double>> tmp =
      std::vector<std::complex<double>>(zeroCount, std::complex<double>(0, 0));
  tmp.insert(tmp.end(), std::begin(in), std::end(in));
  in = std::valarray<std::complex<double>>(tmp.data(), tmp.size());

  auto N = static_cast<unsigned int>(in.size());
  unsigned int k = N;
  unsigned int n = 0;
  double thetaT = M_PI / N;
  std::complex<double> phiT = std::complex<double>(cos(thetaT), -sin(thetaT));
  std::complex<double> T;
  while (k > 1) {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    const double init_value = 1.0L;
    T = init_value;
    for (unsigned int l = 0; l < k; l++) {
      for (unsigned int a = l; a < N; a += n) {
        unsigned int b = a + k;
        std::complex<double> t = in[a] - in[b];
        in[a] += in[b];
        in[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  auto m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++) {
    unsigned int b = a;
    // Reverse bits
    b = (((b & BINARYMASK_1) >> SHIFTBITS_1) | ((b & BINARYMASK_INVERSE_1) << SHIFTBITS_1));
    b = (((b & BINARYMASK_2) >> SHIFTBITS_2) | ((b & BINARYMASK_INVERSE_2) << SHIFTBITS_2));
    b = (((b & BINARYMASK_4) >> SHIFTBITS_4) | ((b & BINARYMASK_INVERSE_4) << SHIFTBITS_4));
    b = (((b & BINARYMASK_8) >> SHIFTBITS_8) | ((b & BINARYMASK_INVERSE_8) << SHIFTBITS_8));
    b = ((b >> SHIFTBITS_16) | (b << SHIFTBITS_16)) >> (SHIFTBITS_32 - m);
    if (b > a) {
      std::complex<double> t = in[a];
      in[a] = in[b];
      in[b] = t;
    }
  }
  std::complex<double> f = 1.0 / N;
  for (unsigned int i = 0; i < N; i++) {
    in[i] *= f;
  }
  return true;
}

auto FourierTools::GetAmplitude(std::complex<double> c) -> double {
  const double amplitude = 2 * std::abs(c);
  return amplitude;
}

auto FourierTools::GetPhase(std::complex<double> c) -> double {
  return std::atan2(c.imag(), c.real());
}

auto FourierTools::GetFrequency(int index, int fftSize, double samplerate) -> double {
  int N = (fftSize - 1) / 2 + 1;
  std::vector<int> values(fftSize);
  std::generate(values.begin(), values.begin() + N, [n = 0]() mutable { return n++; });
  std::generate(values.begin() + N, values.end(), [n = -fftSize / 2]() mutable { return n++; });
  return std::abs(values[index] * samplerate / fftSize);
}

} // namespace haptics::filterbank