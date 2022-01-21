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

#include <PsychohapticModel/include/PsychohapticModel.h>

#include <dj_fft.h>

namespace haptics::tools {

PsychohapticModel::PsychohapticModel(size_t bl_new, int fs_new) : bl(bl_new), fs(fs_new) {

  freqs.resize(bl);
  double step = ((double)fs) / (double)(2 * bl - 1);
  double freq_cur = 0.0;
  for (size_t i = 0; i < bl; ++i) {
    freqs[i] = freq_cur;
    freq_cur += step;
  }

  percthres.resize(bl);
  perceptualThreshold();

  int dwtlevel = (int)log2((double)bl / 4);

  int l_book = dwtlevel + 1;
  book.resize(l_book);
  book[0] = (int)bl >> dwtlevel;
  book[1] = book[0];
  book_cumulative.resize(l_book + 1);
  book_cumulative[0] = 0;
  book_cumulative[1] = book[0];
  book_cumulative[2] = book[1] << 1;
  for (int i = 2; i < l_book; i++) {
    book[i] = book[i - 1] << 1;
    book_cumulative[i + 1] = book_cumulative[i] << 1;
  }
}

auto PsychohapticModel::getSMR(std::vector<double> &block) -> modelResult {

  std::vector<double> spect;
  spect.resize(bl);
  std::copy(block.begin(), block.end(), spect.begin());

  std::vector<std::complex<double>> block_complex(bl * 2, 0);
  std::transform(block.begin(), block.end(), block_complex.begin(),
                 [](double a) { return std::complex<double>(a, 0); });

  std::vector<std::complex<double>> spect_complex = dj::fft1d(block_complex, dj::fft_dir::DIR_FWD);

  std::vector<double> spect_real;
  spect_real.resize(bl);

  static double correction = sqrt(2);
  std::transform(
      spect_complex.begin(), spect_complex.end() - (long long)bl, spect_real.begin(),
      [](std::complex<double> a) { return LOGFACTOR_SPECT * log10(fabs(correction * a.real())); });

  std::vector<double> globalmask = globalMaskingThreshold(spect);
  modelResult result;
  result.SMR.resize(book.size());
  result.bandenergy.resize(book.size());
  std::vector<double> maskenergy(bl, 0);
  int i = 0;
  for (uint32_t b = 0; b < book.size(); b++) {
    result.bandenergy[b] = 0;
    maskenergy[b] = 0;
    for (; i < book_cumulative[b + 1]; i++) {
      result.bandenergy[b] += pow(base, spect[i] / factor);
      maskenergy[b] += globalmask[i];
    }
    result.SMR[b] = factor * log10(result.bandenergy[b] / maskenergy[b]);
  }
  return result;
}

auto PsychohapticModel::globalMaskingThreshold(std::vector<double> &spect) -> std::vector<double> {

  std::vector<double> globalmask(bl, 0);
  double min_peak_height = findMaxVector(spect) - MIN_PEAK_HEIGHT_DIFF;
  peaks p = findPeaks(spect, MIN_PEAK_PROMINENCE, min_peak_height);
  std::vector<double> mask;
  peakMask(p.heights, p.locations, mask);
  if (mask.empty()) {
    for (uint32_t i = 0; i < bl; i++) {
      globalmask[i] = percthres[i]; // percthres is in linear domain
    }
  } else {
    for (uint32_t i = 0; i < bl; i++) {
      globalmask[i] = pow(base, mask.at(i) / factor) + percthres[i]; // percthres is in linear
                                                                     // domain
    }
  }
  return globalmask;
}

void PsychohapticModel::perceptualThreshold() {

  double temp = a / (pow(log10(b), 2));
  uint32_t i = 0;
  while (i < bl) {
    percthres[i] = pow(base, (fabs(temp * pow(log10(c * freqs[i] + b), 2)) - e) / factor);
    // limit values at high frequencies
    if (percthres[i] >= 1) {
      percthres[i] = 1;
      break;
    }
    i++;
  }
  i++;
  for (; i < bl; i++) {
    percthres[i] = percthres[i - 1];
  }
}

auto PsychohapticModel::findAllPeakLocations(std::vector<double> &x) -> peaks {

  peaks p;
  p.locations.reserve(x.size() / 2);
  p.heights.reserve(x.size() / 2);
  size_t num_peaks = 0;
  size_t i = 1;
  size_t i_max = x.size() - 1;
  size_t i_plateau = 0;
  while (i < i_max) {
    if ((i == 0) || (x[i - 1] < x[i])) {
      if (x[i + 1] < x[i]) {
        p.heights.push_back(x[i]);
        p.locations.push_back(i);
        ++num_peaks;
      } else if (i + 1 < x.size()) {
        if (x[i + 1] == x[i]) {
          i_plateau = i + 1;
          while (i_plateau < x.size()) {
            if (x[i_plateau] != x[i]) {
              break;
            }
            ++i_plateau;
          }
          if (i_plateau >= (x.size() - 1)) { // NOLINT
            p.heights.push_back(x[i]);
            size_t i_peak = (i + i_plateau) / 2;
            if (i_peak >= x.size()) {
              i_peak = x.size() - 1;
            }
            p.locations.push_back(i_peak);
            ++num_peaks;
            i = i_plateau;
          } else if (x[i_plateau + 1] < x[i]) { // NOLINT
            p.heights.push_back(x[i]);
            size_t i_peak = (i + i_plateau) / 2;
            if (i_peak >= x.size()) {
              i_peak = x.size() - 1;
            }
            p.locations.push_back(i_peak);
            ++num_peaks;
            i = i_plateau;
          }
        }
      }
    }
    ++i;
  }
  p.heights.resize(num_peaks);
  p.locations.resize(num_peaks);
  return p;
}

auto PsychohapticModel::peakProminence(std::vector<double> &spectrum, peaks input) -> peaks {

  peaks prominences;
  int num_peaks = (int)input.locations.size();
  prominences.heights.reserve(num_peaks);
  prominences.locations.reserve(num_peaks);
  for (int i = 0; i < num_peaks; i++) {
    prominences.heights.push_back(0);
  }
  std::vector<int> valley_left(num_peaks, 0);
  std::vector<int> valley_right(num_peaks, 0);
  valley_right.reserve(num_peaks);
  for (int i = 0; i < num_peaks; ++i) {
    int j_min = 0;
    for (int k = (int)i - 1; k >= 0; --k) {
      if (input.heights[k] > input.heights[i]) {
        j_min = (int)input.locations[k];
        break;
      }
    }
    int j_max = (int)input.locations[i] - 1;
    int j = j_max;
    double min_val_left = input.heights[i];
    while ((j >= j_min) && (j <= j_max)) {
      if (input.locations[i] == 0) {
        valley_left[i] = -1;
        break;
      }
      if (spectrum[j] <= min_val_left) {
        min_val_left = spectrum[j];
        valley_left[i] = j;
      }
      --j;
    }

    j_max = (int)spectrum.size() - 1;
    for (int k = i + 1; k < num_peaks; ++k) {
      if (input.heights[k] > input.heights[i]) {
        j_max = (int)input.locations[k];
        break;
      }
    }
    j_min = (int)input.locations[i] + 1;
    j = j_min;
    double min_val_right = input.heights[i];
    while ((j >= j_min) && (j <= j_max)) {
      if (input.locations[i] == (uint32_t)(j_max)) {
        valley_right[i] = -1;
        break;
      }
      if (spectrum[j] <= min_val_right) {
        min_val_right = spectrum[j];
        valley_right[i] = j;
      }
      ++j;
    }
  }
  double valley_left_height_cur = 0;
  double valley_right_height_cur = 0;
  for (int i = 0; i < num_peaks; ++i) {
    if (valley_left[i] == -1) {
      valley_left_height_cur = -PEAK_HUGE_VAL;
    } else {
      valley_left_height_cur = spectrum[valley_left[i]];
    }
    if (valley_right[i] == -1) {
      valley_right_height_cur = -PEAK_HUGE_VAL;
    } else {
      valley_right_height_cur = spectrum[valley_right[i]];
    }
    prominences.heights[i] =
        input.heights[i] - max(valley_left_height_cur, valley_right_height_cur);
  }
  std::copy(input.locations.begin(), input.locations.end(),
            std::back_inserter(prominences.locations));
  return prominences;
}

auto PsychohapticModel::filterPeakCriterion(peaks &input, double min_peak_val) -> peaks {

  size_t length = input.heights.size();
  peaks output;
  output.heights.reserve(length);
  output.locations.reserve(length);
  size_t num_peaks = 0;
  for (size_t i = 0; i < length; ++i) {
    if (input.heights[i] >= min_peak_val) {
      output.heights.push_back(input.heights[i]);
      output.locations.push_back(input.locations[i]);
      ++num_peaks;
    }
  }
  output.heights.resize(num_peaks);
  output.locations.resize(num_peaks);
  return output;
}

auto PsychohapticModel::findPeaks(std::vector<double> &spectrum, double min_peak_prominence,
                                  double min_peak_height) -> peaks {

  if (spectrum.empty()) {
    peaks p;
    return p;
  }
  bool zeros = true;
  for (auto v : spectrum) {
    if (v > ZERO_COMP) {
      zeros = false;
      break;
    }
  }

  if (zeros) {
    peaks p;
    return p;
  }

  peaks peaks_all = findAllPeakLocations(spectrum);

  if (peaks_all.heights.empty()) {
    return peaks_all;
  }

  peaks peaks_min_h = filterPeakCriterion(peaks_all, min_peak_height);

  if (peaks_min_h.heights.empty()) {
    return peaks_min_h;
  }

  peaks prominences = peakProminence(spectrum, peaks_min_h);

  peaks peaks_min_prominence = filterPeakCriterion(prominences, min_peak_prominence);

  size_t prominences_length = peaks_min_prominence.heights.size();

  peaks result;
  result.locations.reserve(prominences_length);
  result.heights.reserve(prominences_length);
  std::copy(peaks_min_prominence.locations.begin(), peaks_min_prominence.locations.end(),
            std::back_inserter(result.locations));
  for (size_t i = 0; i < prominences_length; ++i) {
    result.heights.push_back(spectrum[peaks_min_prominence.locations[i]]);
  }
  return result;
}

void PsychohapticModel::peakMask(std::vector<double> &peaks_height, std::vector<size_t> &peaks_loc,
                                 std::vector<double> &mask) {

  if (peaks_loc.empty()) {
    mask.clear();
  } else {

    mask.clear();
    mask.reserve(bl);
    double f = freqs[peaks_loc[0]];
    double sum1 = peaks_height[0] - mask_a + (mask_a / mask_b) * f;
    double factor1 = -mask_c / (f * f);

    for (size_t i = 0; i < bl; ++i) {
      double val = freqs[i];
      val -= f;
      val *= val;

      val *= factor1;
      val += sum1;
      mask.push_back(val);
    }
    for (size_t j = 1; j < peaks_loc.size(); j++) {
      f = freqs[peaks_loc[j]];
      sum1 = peaks_height[j] - mask_a + (mask_a / mask_b) * f;
      factor1 = -mask_c / (f * f);
      for (size_t i = 0; i < bl; ++i) {
        double val = freqs[i];
        val -= f;
        val *= val;

        val *= factor1;
        val += sum1;
        if (val > mask[i]) {
          mask[i] = val;
        }
      }
    }
  }
}

auto PsychohapticModel::max(double v1, double v2) -> double {

  double result = 0;
  if (v1 > v2) {
    result = v1;
  } else {
    result = v2;
  }
  return result;
}

auto PsychohapticModel::findMaxVector(std::vector<double> &data) -> double {

  double max = data[0];

  for (size_t i = 1; i < data.size(); i++) {
    if (data[i] > max) {
      max = data[i];
    }
  }

  return max;
}

} // namespace haptics::tools
