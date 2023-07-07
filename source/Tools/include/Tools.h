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

#include <Types/include/CurveType.h>
#include <iostream>
#include <vector>

constexpr auto S_2_MS = 1000.0;
constexpr auto MS_2_S = 0.001;
constexpr auto MS_2_MICROSECONDS = 1000;
constexpr auto MICROSECONDS_2_MS = 0.001;
constexpr auto AKIMA_EPSILON = -100;
constexpr auto AKIMA_THRESHOLD = 10;
constexpr auto AKIMA_THRESHOLD2 = -8;
constexpr auto BSPLINE_STEP = 10;
constexpr auto CUBIC_COEFFICIENT = 6;

namespace haptics::tools {

[[nodiscard]] auto linearInterpolation(std::pair<int, double> a, std::pair<int, double> b, double x)
    -> double;

[[nodiscard]] auto chirpInterpolation(int start_time, int end_time, double start_frequency,
                                      double end_frequency, double position) -> double;

[[nodiscard]] auto genericNormalization(double start_in, double end_in, double start_out,
                                        double end_out, double x_in) -> double;

[[nodiscard]] auto is_eq(double a, double b) -> bool;

[[nodiscard]] auto linearInterpolation2(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto cubicInterpolation2(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto cubicInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto akimaInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto bezierInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto bsplineInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double>;

[[nodiscard]] auto interpolationCodec(const std::vector<std::pair<int, double>> &points,
                                      types::CurveType curveType) -> std::vector<double>;

[[nodiscard]] auto is_number(const std::string &s) -> bool;

} // namespace haptics::tools
#endif
