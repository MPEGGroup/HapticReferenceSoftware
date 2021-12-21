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

#include <Tools/include/PsychohapticModel.h>

#include <dj_fft.h>

namespace haptics::tools {

PsychohapticModel::PsychohapticModel(int bl, int fs) {

    this->bl = bl;
    this->fs = fs;

    freqs.resize(bl);
    double step = ((double)fs) / (double)(2*bl - 1); //correct stepsize?
    double freq_cur = 0.0;
    for (size_t i = 0; i < bl; ++i) {
        freqs[i] = freq_cur;
        freq_cur += step;
    }

    percthres.resize(bl);
    perceptualThreshold();

    int dwtlevel = (int)log2((double)bl/4);

    int l_book = dwtlevel + 1;
    book.resize(l_book);
    book[0] = bl >> dwtlevel;
    book[1] = book[0];
    book_cumulative.resize(l_book+1);
    book_cumulative[0] = 0;
    book_cumulative[1] = book[0];
    book_cumulative[2] = book[1] << 1;
    for(int i=2; i<l_book; i++){
        book[i] = book[i-1] << 1;
        book_cumulative[i+1] = book_cumulative[i] << 1;
    }
}

void PsychohapticModel::getSMR(std::vector<double> &block, std::vector<double> &SMR, std::vector<double> &bandenergy){

    std::vector<double> spect;
    spect.resize(bl);
    std::copy(block.begin(),block.end(),spect.begin());

    std::vector<std::complex<double>> block_complex(bl*2,0);
    std::transform(block.begin(), block.end(), block_complex.begin(), [](double a) {
        return std::complex<double>(a,0);
    } );

    std::vector<std::complex<double>> spect_complex = dj::fft1d(block_complex,dj::fft_dir::DIR_FWD);

    /*for(size_t i=0; i<block_complex.size(); i++){
        std::cout << spect_complex[i].real() << ", " << spect_complex[i].imag() << std::endl;
    }*/

    std::vector<double> spect_real;
    spect_real.resize(bl);

    static double correction = sqrt(2);
    std::transform( spect_complex.begin(), spect_complex.end()-bl, spect_real.begin(), [](std::complex<double> a) {
        return LOGFACTOR_SPECT*log10(abs(correction*a.real()));
    } );

    /*for(size_t i=0; i<bl; i++){
        std::cout << spect_real[i] << std::endl;
    }*/

    std::vector<double> globalmask(bl,0);
    globalMaskingThreshold(spect,globalmask);
    SMR.resize(book.size());
    bandenergy.resize(book.size());
    std::vector<double> maskenergy(bl,0);
    int i = 0;
    for(int b = 0; b<book.size(); b++){
        bandenergy[b] = 0;
        maskenergy[b] = 0;
        for(; i<book_cumulative[b+1]; i++){
            bandenergy[b] += pow(base,spect[i]/factor);
            maskenergy[b] += globalmask[i];
        }
        SMR[b] = factor * log10(bandenergy[b]/maskenergy[b]);
    }

}

void PsychohapticModel::globalMaskingThreshold(std::vector<double> &spect, std::vector<double> &globalmask){


    double min_peak_height = findMaxVector(spect) - MIN_PEAK_HEIGHT_DIFF;
    std::vector<double> peaks_height;
    std::vector<size_t> peaks_location;
    findPeaks(spect,MIN_PEAK_PROMINENCE,min_peak_height,peaks_height,peaks_location);
    std::vector<double> mask;
    peakMask(peaks_height,peaks_location,mask);
    if(mask.empty()){
        for(int i=0; i<bl; i++){
            globalmask[i] = percthres[i]; //percthres is in linear domain
        }
    }else{
        for(int i=0; i<bl; i++){
            globalmask[i] = pow(base,mask.at(i)/factor)+percthres[i]; //percthres is in linear domain
        }
    }


}

void PsychohapticModel::perceptualThreshold() {

    double temp = a/(pow(log10(b),2));
    int i = 0;
    while(i<bl){
        percthres[i] = pow(base,(abs(temp * pow(log10(c*freqs[i]+b),2)) -e)/factor);
        //limit values at high frequencies
        if(percthres[i]>=1){
            percthres[i]=1;
            break;
        }
        i++;
    }
    i++;
    for(; i<bl; i++){
        percthres[i] = percthres[i-1];
    }

}

void PsychohapticModel::findAllPeakLocations(std::vector<double> &x, std::vector<double> &height, std::vector<size_t> &location) {

    location.reserve(x.size()/2);
    height.reserve(x.size()/2);
      size_t num_peaks = 0;
      size_t i = 1;
      size_t i_max = x.size() - 1;
      size_t i_plateau = 0;
      while (i < i_max) {
          if ((i == 0) || (x[i-1] < x[i])) {
              if (x[i+1] < x[i]) {
                  height.push_back(x[i]);
                  location.push_back(i);
                  ++num_peaks;
              }
              else if (x[i+1] == x[i]) {
              //else if (abs(x[i+1] - x[i]) < abs(x[i]/PLATEAU_COMP_FACTOR)) {
                  i_plateau = i + 1;
                  while (x[i_plateau] == x[i]) {
                  //while (abs(x[i_plateau] - x[i]) < abs(x[i]/PLATEAU_COMP_FACTOR)) {
                      ++i_plateau;
                  }
                  if (x[i_plateau+1] < x[i]) {
                      height.push_back(x[i]);
                      location.push_back((i + i_plateau) / 2);
                      ++num_peaks;
                      i = i_plateau;
                  }
              }
          }
          ++i;
      }
    height.resize(num_peaks);
    location.resize(num_peaks);
}

void PsychohapticModel::peakProminence(std::vector<double> &spectrum, std::vector<double> &peaks_height, std::vector<size_t> &peaks_location, std::vector<double> &prominences_height, std::vector<size_t> &prominences_location) {

    size_t num_peaks = peaks_location.size();
    prominences_height.reserve(num_peaks);
    prominences_location.reserve(num_peaks);
    for(size_t i = 0; i<num_peaks; i++){
        prominences_height.push_back(0); //simple init instead?
    }
    std::vector<size_t> valley_left(num_peaks,0);
    std::vector<size_t> valley_right(num_peaks,0);
    valley_right.reserve(num_peaks);
    for (size_t i = 0; i < num_peaks; ++i) {
      size_t j_min = 0;
      for (int k = (int)i-1; k >= 0; --k) {
          if (peaks_height[k] > peaks_height[i]) {
              j_min = peaks_location[k];
              break;
          }
      }
      size_t j_max = peaks_location[i] - 1;
      size_t j = j_max;
      double min_val_left = peaks_height[i];
      while ((j >= j_min) && (j <= j_max)) {
          if (peaks_location[i] == 0) {
              valley_left[i] = -1;
              break;
          }
          if (spectrum[j] <= min_val_left) {
              min_val_left = spectrum[j];
              valley_left[i] = j;
          }
          --j;
      }

      j_max = spectrum.size() - 1;
      for (size_t k = i+1; k < num_peaks; ++k) {
          if (peaks_height[k] > peaks_height[i]) {
              j_max = peaks_location[k];
              break;
          }
      }
      j_min = peaks_location[i] + 1;
      j = j_min;
      double min_val_right = peaks_height[i];
      while ((j >= j_min) && (j <= j_max)) {
          if (peaks_location[i] == (int32_t)(j_max)) {
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
    for (size_t i = 0; i < num_peaks; ++i) {
      if (valley_left[i] == -1) {
          valley_left_height_cur = -PEAK_HUGE_VAL;
      }
      else {
          valley_left_height_cur = spectrum[valley_left[i]];
      }
      if (valley_right[i] == -1) {
          valley_right_height_cur = -PEAK_HUGE_VAL;
      }
      else {
          valley_right_height_cur = spectrum[valley_right[i]];
      }
      prominences_height[i] = peaks_height[i] - max(valley_left_height_cur, valley_right_height_cur);
    }
    std::copy(peaks_location.begin(),peaks_location.end(),std::back_inserter(prominences_location));
}

void PsychohapticModel::filterPeakCriterion(std::vector<double> &input_height, std::vector<size_t> &input_location, std::vector<double> &result_height, std::vector<size_t> &result_location, double min_peak_val) {

    size_t length = input_height.size();
    result_height.reserve(length);
    result_location.reserve(length);
    size_t num_peaks = 0;
    for (size_t i = 0; i < length; ++i) {
        if (input_height[i] >= min_peak_val) {
            result_height.push_back(input_height[i]);
            result_location.push_back(input_location[i]);
            ++num_peaks;
        }
    }
    result_height.resize(num_peaks);
    result_location.resize(num_peaks);
}

void PsychohapticModel::findPeaks(std::vector<double> &spectrum, double min_peak_prominence, double min_peak_height, std::vector<double> &result_height, std::vector<size_t> &result_location) {

    if(spectrum.empty()) {
        return;
    }

    std::vector<double> peaks_all_height;
    std::vector<size_t> peaks_all_location;
    findAllPeakLocations(spectrum,peaks_all_height,peaks_all_location);

    if (peaks_all_height.empty()) {
        return;
    }

    std::vector<double> peaks_min_h_height;
    std::vector<size_t> peaks_min_h_location;
    filterPeakCriterion(peaks_all_height,peaks_all_location,peaks_min_h_height,peaks_min_h_location,min_peak_height);

    if (peaks_min_h_height.empty()) {
        return;
    }

    std::vector<double> prominences_height;
    std::vector<size_t> prominences_location;
    peakProminence(spectrum,peaks_min_h_height,peaks_min_h_location,prominences_height,prominences_location);

    std::vector<double> peaks_min_prominence_height;
    std::vector<size_t> peaks_min_prominence_location;
    filterPeakCriterion(prominences_height,prominences_location,peaks_min_prominence_height,peaks_min_prominence_location,min_peak_prominence);

    size_t prominences_length = peaks_min_prominence_height.size();

    result_location.reserve(prominences_length);
    result_height.reserve(prominences_length);
    std::copy(peaks_min_prominence_location.begin(),peaks_min_prominence_location.end(),std::back_inserter(result_location));
    for (size_t i = 0; i < prominences_length; ++i) {
        result_height.push_back(spectrum[peaks_min_prominence_location[i]]);
    }
}

void PsychohapticModel::peakMask(std::vector<double> &peaks_height, std::vector<size_t> &peaks_loc, std::vector<double> &mask) {

    if(peaks_loc.empty()){
        mask.clear();
    }else{

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
        for(size_t j=1; j<peaks_loc.size(); j++){
            f = freqs[peaks_loc[j]];
            sum1 = peaks_height[j] - mask_a + (mask_a / mask_b) * f;
            factor1 = -mask_c / (f * f);
            for (size_t i = 0; i < bl; ++i) {
                double val = freqs[i];
                val -= f;
                val *= val;

                val *= factor1;
                val += sum1;
                if(val>mask[i]){
                    mask[i] = val;
                }
            }
        }
    }
}

auto PsychohapticModel::max(double v1, double v2) -> double{

    double result = 0;
    if(v1>v2){
        result = v1;
    }else{
        result = v2;
    }
    return result;
}

auto PsychohapticModel::findMaxVector(std::vector<double> &data) -> double{

    double max = data[0];

    for(size_t i = 1; i<data.size(); i++){
        if(data[i]>max){
            max = data[i];
        }
    }

    return max;
}

} // namespace haptics::tools
