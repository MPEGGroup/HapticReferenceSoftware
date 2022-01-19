
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
#include <catch2/catch.hpp>
#include <cmath>
#include <complex>
#include <valarray>
#include <vector>

using haptics::filterbank::FourierTools;

TEST_CASE("haptics::filterbank::FourierTools on double", "[FourierTools][FFT][IFFT]") {
  constexpr double FS = 2048;
  constexpr double SINE_FREQUENCY = 100;
  constexpr double SINE_AMPLITUDE = .6754;
  std::vector<double> testingValue;
  for (double i = 0; i < FS; i++) {
    testingValue.push_back(SINE_AMPLITUDE * std::sin(SINE_FREQUENCY * 2 * M_PI * i / FS));
  }

  std::valarray<std::complex<double>> res;
  REQUIRE(FourierTools::FFT(testingValue, res));

  auto *firstPeakIt = std::max_element(
      std::begin(res), std::end(res), [](std::complex<double> e1, std::complex<double> e2) {
        return FourierTools::GetAmplitude(e1) < FourierTools::GetAmplitude(e2);
      });
  auto firstPeakIndex = static_cast<unsigned int>(firstPeakIt - std::begin(res));
  auto secondPeakIndex = static_cast<unsigned int>(res.size() - firstPeakIndex);

  const double marginEquals_0 = 1e-10;
  for (size_t i = 0; i < res.size(); i++) {
    if (i == firstPeakIndex || i == secondPeakIndex) {
      CHECK(FourierTools::GetFrequency(i, res.size(), FS) == Approx(SINE_FREQUENCY));
      CHECK(FourierTools::GetAmplitude(res[i]) == Approx(SINE_AMPLITUDE));
    } else {
      CHECK(FourierTools::GetAmplitude(res[i]) == Approx(0).margin(marginEquals_0));
    }
  }

  CHECK(true);
}

TEST_CASE("haptics::filterbank::FourierTools on complex", "[FourierTools][FFT][IFFT]") {
  constexpr double FS = 2048;
  constexpr double SINE_FREQUENCY = 250;
  constexpr double SINE_AMPLITUDE = .45;
  std::vector<std::complex<double>> testingValue;
  for (double i = 0; i < FS; i++) {
    testingValue.emplace_back(
        std::complex<double>(SINE_AMPLITUDE * std::sin(SINE_FREQUENCY * 2 * M_PI * i / FS), 0));
  }

  std::valarray<std::complex<double>> v(testingValue.data(), testingValue.size());
  REQUIRE(FourierTools::FFT(v));

  auto *firstPeakIt = std::max_element(
      std::begin(v), std::end(v), [](std::complex<double> e1, std::complex<double> e2) {
        return FourierTools::GetAmplitude(e1) < FourierTools::GetAmplitude(e2);
      });
  auto firstPeakIndex = static_cast<unsigned int>(firstPeakIt - std::begin(v));
  auto secondPeakIndex = static_cast<unsigned int>(v.size() - firstPeakIndex);

  const double marginEquals_0 = 1e-10;
  for (size_t i = 0; i < v.size(); i++) {
    if (i == firstPeakIndex || i == secondPeakIndex) {
      CHECK(FourierTools::GetFrequency(i, v.size(), FS) == Approx(SINE_FREQUENCY));
      CHECK(FourierTools::GetAmplitude(v[i]) == Approx(SINE_AMPLITUDE));
    } else {
      CHECK(FourierTools::GetAmplitude(v[i]) == Approx(0).margin(marginEquals_0));
    }
  }
}
