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

#include <catch2/catch.hpp>
#include <FilterBank/include/FourierTools.h>
#include <complex>
#include <valarray>
#include <vector>
#include <cmath>

using haptics::filterbank::FourierTools;

TEST_CASE("haptics::filterbank::FourierTools on double", "[FourierTools][FFT][IFFT]") {
  constexpr double FS = 2048;
  constexpr int frequency = 250;
  std::vector<double> testingValue;
  for (double i = 0; i < FS; i++) {
    testingValue.emplace_back(std::cos(2 * haptics::filterbank::PI / FS));
  }

  std::valarray<std::complex<double>> res;
  REQUIRE(FourierTools::FFT(testingValue, res));
  REQUIRE(FourierTools::IFFT(res));

  REQUIRE(res.size() == testingValue.size());
  auto itTestingValue = testingValue.begin();
  for (std::complex<double> e : res) {
    CHECK(e.real() == Approx(*itTestingValue));
    CHECK(e.imag() == Approx(0));

    itTestingValue++;
  }
}

TEST_CASE("haptics::filterbank::FourierTools on complex", "[FourierTools][FFT][IFFT]") {
  constexpr double FS = 2048;
  constexpr int frequency = 250;
  std::vector<std::complex<double>> testingValue;
  for (double i = 0; i < FS; i++) {
    testingValue.emplace_back(std::complex<double>(std::cos(2 * haptics::filterbank::PI / FS), 0));
  }

  std::valarray<std::complex<double>> v(testingValue.data(), testingValue.size());
  REQUIRE(FourierTools::FFT(v));
  REQUIRE(FourierTools::IFFT(v));

  REQUIRE(v.size() == testingValue.size());
  auto itTestingValue = testingValue.begin();
  for (std::complex<double> res : v) {
    CHECK(res.real() == Approx(itTestingValue->real()));
    CHECK(res.imag() == Approx(itTestingValue->imag()));

    itTestingValue++;
  }
}
