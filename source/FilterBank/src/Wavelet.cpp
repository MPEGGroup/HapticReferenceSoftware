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

#include <FilterBank/include/Wavelet.h>
#include <algorithm>

namespace haptics::filterbank {

void Wavelet::DWT(std::vector<double> &in, int levels, std::vector<double> &out) {

  out.resize(in.size());
  std::vector<double> x(in.begin(), in.end());
  for (int i = 0; i < levels; i++) {
    auto len = in.size() >> i;
    std::vector<double> l(len, 0);
    std::vector<double> h(len, 0);

    std::vector<double> x_temp(len, 0);
    std::copy(x.begin(), x.begin() + (long long)len, x_temp.begin());
    symconv1D(x_temp, hp, h);
    symconv1D(x_temp, lp, l);

    auto out_ind = 0;
    auto out_add = len >> 1;
    for (uint32_t j = 0; j < len; j += 2) {
      out[out_ind] = l[j];
      out[out_ind + out_add] = h[j + 1];
      x[out_ind] = l[j];
      out_ind++;
    }
  }
}

void Wavelet::inv_DWT(std::vector<double> &in, int levels, std::vector<double> &out) {

  std::copy(in.begin(), in.end(), out.begin());

  for (int i = levels - 1; i >= 0; i--) {
    auto len = in.size() >> i;
    std::vector<double> l(len, 0);
    std::vector<double> h(len, 0);
    for (uint32_t j = 0; j < len; j += 2) {
      l[j] = out[j / 2];
      h[j + 1] = out[j / 2 + (len / 2)];
    }
    /*for (int j = 0; j < len; j += 2) {
      l[j + 1] = 0;
      h[j] = 0;
    }*/
    symconv1D(h, hpr, out);
    symconv1DAdd(l, lpr, out);
  }
}

template <size_t hSize>
void Wavelet::symconv1D(std::vector<double> &in, std::array<double, hSize> &h,
                        std::vector<double> &out) {

  size_t inSize = in.size();

  // symmetric extension
  auto lext = (long)floor((double)hSize / 2); // floor: if h has odd length
  std::vector<double> temp(in.begin(), in.end());
  std::vector<double> temp_l(in.begin() + 1, in.begin() + lext + 1);
  std::vector<double> temp_r(in.end() - lext - 1, in.end() - 1);

  std::reverse(temp_l.begin(), temp_l.end());
  std::reverse(temp_r.begin(), temp_r.end());
  temp.insert(temp.begin(), temp_l.begin(), temp_l.end());
  temp.insert(temp.end(), temp_r.begin(), temp_r.end());

  auto extension = 2 * lext;
  std::vector<double> conv(inSize + hSize - 1 + extension, 0);
  conv1D(temp, h, conv);
  std::copy(conv.begin() + extension, conv.end() - extension, out.begin());
}

template <size_t hSize>
void Wavelet::symconv1DAdd(std::vector<double> &in, std::array<double, hSize> &h,
                           std::vector<double> &out) {

  size_t inSize = in.size();

  // symmetric extension
  auto lext = (long)floor((double)hSize / 2); // floor: if h has odd length
  std::vector<double> temp(in.begin(), in.end());
  std::vector<double> temp_l(in.begin() + 1, in.begin() + lext + 1);
  std::vector<double> temp_r(in.end() - lext - 1, in.end() - 1);

  std::reverse(temp_l.begin(), temp_l.end());
  std::reverse(temp_r.begin(), temp_r.end());
  temp.insert(temp.begin(), temp_l.begin(), temp_l.end());
  temp.insert(temp.end(), temp_r.begin(), temp_r.end());

  auto extension = 2 * lext;
  std::vector<double> conv(inSize + hSize - 1 + extension, 0);
  conv1D(temp, h, conv);
  for (uint32_t i = 0; i < inSize; i++) {
    out[i] += conv[i + hSize - 1];
  }
}

template <size_t hSize>
void Wavelet::conv1D(std::vector<double> &in, std::array<double, hSize> &h,
                     std::vector<double> &out) {

  size_t j = 0;
  size_t inSize = in.size();
  for (j = 0; j < inSize; j++) {
    out[j] = in[j] * h[0];
  }
  for (; j < inSize + hSize - 1; j++) {
    out[j] = 0;
  }
  for (uint32_t i = 1; i < hSize; i++) {
    for (j = i; j < inSize + i; j++) {
      out[j] += in[j - i] * h.at(i);
    }
  }
}

} // namespace haptics::filterbank
