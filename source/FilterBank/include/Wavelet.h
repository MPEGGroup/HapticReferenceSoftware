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

#ifndef WAVELET_H
#define WAVELET_H

#include <cmath>
#include <iostream>
#include <vector>
#include <array>

namespace haptics::filterbank {

constexpr double LP_0 = 0.852698679009404;
constexpr double LP_1 = 0.377402855612654;
constexpr double LP_2 = -0.110624404418423;
constexpr double LP_3 = -0.023849465019380;
constexpr double LP_4 = 0.037828455506995;
constexpr size_t LP_SIZE = 9;

constexpr double HP_0 = -0.788485616405665;
constexpr double HP_1 = 0.418092273222212;
constexpr double HP_2 = 0.040689417609559;
constexpr double HP_3 = -0.064538882628938;
constexpr size_t HP_SIZE = 7;

class Wavelet {
public:
  void DWT(std::vector<double> &in, int levels, std::vector<double> &out);
  void inv_DWT(std::vector<double> &in, int levels, std::vector<double> &out);

  template <size_t hSize>
  static void symconv1D(std::vector<double> &in, std::array<double, hSize> &h, std::vector<double> &out);
  template <size_t hSize>
  static void symconv1DAdd(std::vector<double> &in, std::array<double, hSize> &h,
                           std::vector<double> &out);
  template <size_t hSize>
  static void conv1D(std::vector<double> &in, std::array<double, hSize> &h, std::vector<double> &out);

private:
  std::array<double,LP_SIZE> lp = {LP_4,  LP_3, LP_2, LP_1, LP_0, LP_1, LP_2, LP_3, LP_4};
  std::array<double,HP_SIZE> hp = {HP_3, HP_2, HP_1, HP_0, HP_1, HP_2, HP_3};
  std::array<double,HP_SIZE> lpr = {HP_3, -HP_2, HP_1, -HP_0, HP_1, -HP_2, HP_3};
  std::array<double,LP_SIZE> hpr = {-LP_4,  LP_3, -LP_2, LP_1, -LP_0, LP_1, -LP_2, LP_3, -LP_4};

};
} // namespace haptics::filterbank
#endif // WAVELET_H
