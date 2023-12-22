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

#include <iostream>
#include <vector>

#include "../include/WaveletEncoder.h"
#include "FilterBank/include/Filterbank.h"
#include "Tools/include/WavParser.h"
#include "WaveletDecoder/include/WaveletDecoder.h"

constexpr int val = 3;
constexpr int bits = 3;
constexpr double positive = 3;
constexpr double negative = -3;
constexpr size_t pos = 5;
constexpr size_t size = 10;

constexpr double unquantized = 0.7499;
constexpr double quantized = 0.75000000;

constexpr size_t bl_test = 512;
constexpr int fs_test = 8000;
constexpr size_t book_size = 8;

constexpr int FS = 8000;
constexpr size_t BL = 128;
constexpr size_t BITS = 90;
constexpr double F_CUTOFF = 72;

constexpr size_t bl = 128;
constexpr int levels = 1;
constexpr int hSize = 7;
constexpr int prec = 15;
constexpr double prec_comparison = 0.0001;

constexpr double S_2_MS_TEST = 1000;

constexpr double FS_filter = 8000;

constexpr double timescale = 1000;

TEST_CASE("haptics::encoder::WaveletEncoder,1") {

  using haptics::encoder::WaveletEncoder;

  SECTION("Encoder tools") {

    std::vector<char> outstream(1, '0');
    WaveletEncoder::de2bi(val, outstream, bits);
    CHECK(outstream.size() == bits + 1);
    CHECK(outstream[1] == 1);
    CHECK(outstream[2] == 1);
    CHECK(outstream[3] == 0);

    auto sign = WaveletEncoder::sgn(positive);
    CHECK(sign == 1);
    sign = WaveletEncoder::sgn(negative);
    CHECK(sign == -1);
  }
}

TEST_CASE("haptics::encoder::WaveletEncoder,2") {

  using haptics::encoder::quantMode;
  using haptics::encoder::WaveletEncoder;

  SECTION("Encoder tools") {

    std::vector<double> data(size, 1);
    data[pos] = negative;
    size_t pos_found = WaveletEncoder::findMinInd(data);
    CHECK(pos_found == pos);

    std::vector<double> data2(size, 1);
    data2[pos] = positive;
    double max = WaveletEncoder::findMax(data2);
    CHECK(max == positive);

    quantMode mode{0, FRACTIONBITS_0, 0};
    double quant = WaveletEncoder::maxQuant(unquantized, mode);
    CHECK(fabs(quant - quantized) < prec_comparison);
    quantMode mode2{3, 4, 0};
    quant = WaveletEncoder::maxQuant(unquantized + 1, mode2);
    CHECK(quant == quantized + 1);

    std::vector<double> v_unquantized(3, unquantized);
    std::vector<double> v_quantized(3, 0);
    WaveletEncoder::uniformQuant(v_unquantized, 1, 1, bits, 1, v_quantized);
    CHECK(v_quantized[0] == 0);
    CHECK(v_quantized[1] == quantized);
    CHECK(v_quantized[2] == 0);
  }
}

TEST_CASE("haptics::encoder::WaveletEncoder,3") {

  using haptics::encoder::WaveletEncoder;
  using haptics::types::Band;

  SECTION("Encoder tools") {
    std::vector<double> v_unquantized(3, unquantized);
    double qwavmax = 0;
    std::vector<char> bitwavmax;
    std::vector<char> bitwavmax_compare = {0, 0, 0, 0, 0, 0, 1, 1};
    WaveletEncoder::maximumWaveletCoefficient(v_unquantized, qwavmax, bitwavmax);
    CHECK(fabs(qwavmax - quantized) < prec_comparison);
  }

  SECTION("Encoder") {
    std::vector<double> data_time(bl_test, 0);
    data_time[0] = 1;
    WaveletEncoder waveletEncoder(bl_test, fs_test, timescale);
    double scalar = 0;
    int maxbits = 0;
    std::vector<double> data_quant = waveletEncoder.encodeBlock(data_time, 1, scalar, maxbits);
  }

  SECTION("Encoder Integration") {
    std::vector<double> data_time(bl_test, 0);
    data_time[0] = 1;
    WaveletEncoder waveletEncoder(bl_test / 2, fs_test);
    Band band;
    bool success = false;
    success = waveletEncoder.encodeSignal(data_time, 1, 0, band);
    CHECK(success);
  }
}

TEST_CASE("Encoder/Decoder Integration") {

  using haptics::encoder::WaveletEncoder;
  using haptics::waveletdecoder::WaveletDecoder;

  SECTION("Input/Output test") {

    WaveletEncoder enc(BL, FS);
    std::vector<double> sig_time(BL * 2, 0);
    sig_time[0] = 1;
    Band b;
    enc.encodeSignal(sig_time, BITS, F_CUTOFF, b);

    std::vector<double> sig_rec = WaveletDecoder::decodeBand(b);
    CHECK(sig_time.size() == sig_rec.size());
  }
}

TEST_CASE("Band transformation") {

  using haptics::encoder::WaveletEncoder;
  using haptics::waveletdecoder::WaveletDecoder;

  SECTION("Input/Output test") {

    WaveletEncoder enc(BL, FS);
    std::vector<double> sig_time(BL * 2, 0);
    sig_time[0] = 1;
    Band b;
    enc.encodeSignal(sig_time, BITS, F_CUTOFF, b);

    WaveletDecoder::transformBand(b);

    for (double t = 0; t < 1.0 / FS * (BL * 2); t += 1.0 / FS) {
      std::cout << b.Evaluate(t * S_2_MS_TEST, 0, FS) << std::endl;
    }
  }
}
