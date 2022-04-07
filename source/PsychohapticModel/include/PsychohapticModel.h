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

#ifndef PSYCHOHAPTICMODEL_H
#define PSYCHOHAPTICMODEL_H

#include <algorithm>
#include <iostream>
#include <vector>

constexpr double a = 50;
constexpr double c = (double)1 / (double)550;
constexpr double b = 1 - (250 * c);
constexpr double e = 45;
constexpr double base = 10;
constexpr double factor = 10;

constexpr double mask_a = 5;
constexpr double mask_b = 1400;
constexpr double mask_c = 30;

constexpr double PEAK_HUGE_VAL = 2147483647; // 2^32 - 1
constexpr double MIN_PEAK_PROMINENCE = 12;
constexpr double MIN_PEAK_HEIGHT_DIFF = 45;

constexpr double LOGFACTOR_SPECT = 20;
constexpr double ZERO_COMP = 1e-35;

namespace haptics::tools {

struct peaks {
  std::vector<size_t> locations;
  std::vector<double> heights;
};

struct modelResult {
  std::vector<double> SMR;
  std::vector<double> bandenergy;
};

class PsychohapticModel {
public:
  PsychohapticModel(size_t bl_new, int fs_new);

  auto getSMR(std::vector<double> &block) -> modelResult;

  static auto findPeaks(std::vector<double> &spectrum, double min_peak_prominence,
                        double min_peak_height) -> peaks;
  void peakMask(std::vector<double> &peaks_height, std::vector<size_t> &peaks_loc,
                std::vector<double> &mask);

private:
  auto globalMaskingThreshold(std::vector<double> &spect) -> std::vector<double>;
  void perceptualThreshold();

  static auto findAllPeakLocations(std::vector<double> &x) -> peaks;
  static auto peakProminence(std::vector<double> &spectrum, peaks input) -> peaks;
  static auto filterPeakCriterion(peaks &input, double min_peak_val) -> peaks;

  static auto max(double v1, double v2) -> double;
  static auto findMaxVector(std::vector<double> &data) -> double;

  size_t bl;
  int fs;
  std::vector<double> freqs;
  std::vector<double> percthres;
  std::vector<int> book;
  std::vector<int> book_cumulative;
};
} // namespace haptics::tools
#endif // PSYCHOHAPTICMODEL_H
