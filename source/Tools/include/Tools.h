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

#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>

namespace haptics::tools {

  extern const double S_2_MS = 1000.0;
  extern const double MS_2_S = .001;

	[[nodiscard]] extern auto linearInterpolation(std::pair<int,double> a, std::pair<int,double> b, int x) -> double {

    int start = a.first;
    int end = b.first;
    double start_a = a.second;
    double end_a = b.second;

    if (x < start || x > end) {
      return EXIT_FAILURE;
    }

    if (start >= end) {
      std::swap(start, end);
      std::swap(start_a, end_a);
    }

    return (start_a * (end - x) + end_a * (x - start)) / (end - start);
  }


  [[nodiscard]] extern auto chirpInterpolation(int start_time, int end_time, double start_frequency,
                                               double end_frequency, int position) -> double {

    int start_t = start_time;
    int end_t = end_time;
    double start_f = start_frequency;
    double end_f = end_frequency;

    if (end_t > start_t) {
      std::swap(start_t, end_t);
      std::swap(start_f, end_f);
    }

    if (start_t == end_t) {
      return end_f;
    }

    if (position < start_t && position > end_t) {
      return EXIT_FAILURE;
    }

    return position * (end_f - start_f) / (end_t - start_t) + start_f;
  }


  [[nodiscard]] extern auto genericNormalization(double start_in, double end_in, double start_out,
                                                 double end_out, double x_in) -> double {

    double x_out = -1;

    if (end_in - start_in != 0) {
      x_out = ((end_out - start_out) / (end_in - start_in)) * (x_in - end_in) + end_out;
    }

    return x_out;
  }

  [[nodiscard]] extern auto is_eq(double a, double b) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return std::fabs(a - b) <= std::numeric_limits<double>::epsilon() * 100;
  }
  }
#endif
