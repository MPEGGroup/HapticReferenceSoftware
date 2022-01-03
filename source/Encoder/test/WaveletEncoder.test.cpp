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
#include "WaveletDecoder/include/WaveletDecoder.h"
#include "FilterBank/include/Filterbank.h"
#include "Tools/include/WavParser.h"

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
constexpr double prec_comparison = 0.00001;

constexpr double S_2_MS_TEST = 1000;

constexpr double FS_filter = 8000;

TEST_CASE("haptics::encoder::WaveletEncoder,1") {

  using haptics::encoder::WaveletEncoder;

  SECTION("Encoder tools") {

    std::vector<unsigned char> outstream(1, '0');
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

/*TEST_CASE("haptics::encoder::WaveletEncoder,2") {

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

    quantMode mode{0, FRACTIONBITS_0};
    double quant = WaveletEncoder::maxQuant(unquantized, mode);
    CHECK(quant == quantized);
    quantMode mode2{3, 4};
    quant = WaveletEncoder::maxQuant(unquantized + 1, mode2);
    CHECK(quant == quantized + 1);

    std::vector<double> v_unquantized(3, unquantized);
    std::vector<double> v_quantized(3, 0);
    WaveletEncoder::uniformQuant(v_unquantized, 1, 1, bits, 1, v_quantized);
    CHECK(v_quantized[0] == 0);
    CHECK(v_quantized[1] == quantized);
    CHECK(v_quantized[2] == 0);
  }
}*/

TEST_CASE("haptics::encoder::WaveletEncoder,3") {

  using haptics::encoder::WaveletEncoder;
  using haptics::types::Band;

  /*SECTION("Encoder tools") {

    std::vector<double> v_unquantized(3, unquantized);
    double qwavmax = 0;
    std::vector<unsigned char> bitwavmax;
    std::vector<unsigned char> bitwavmax_compare = {0, 0, 0, 0, 0, 0, 1, 1};
    WaveletEncoder::maximumWaveletCoefficient(v_unquantized, qwavmax, bitwavmax);
    CHECK(qwavmax == quantized);
    CHECK(std::equal(bitwavmax.begin(), bitwavmax.end(), bitwavmax_compare.begin()));

    for(size_t i=0; i<WAVMAXLENGTH; i++){
        std::cout << (int)bitwavmax[i] << std::endl;
    }
  }

  SECTION("Encoder") {

    std::vector<double> data_time(bl_test, 0);
    data_time[0] = 1;
    WaveletEncoder waveletEncoder(bl_test, fs_test);
    double scalar = 0;
    std::vector<double> data_quant = waveletEncoder.encodeBlock(data_time, 1, scalar);
  }*/

  SECTION("Encoder Integration") {
    std::vector<double> data_time(bl_test, 0);
    data_time[0] = 1;
    WaveletEncoder waveletEncoder(bl_test / 2, fs_test);
    Band band;
    bool success = false;
    success = waveletEncoder.encodeSignal(data_time, 1, 0, band);
    CHECK(success);

    /*std::cout << "number of effects: " << band.getEffectsSize() << std::endl;
    Effect effect = band.getEffectAt(0);
    std::cout << "number of keyframes: " << effect.getKeyframesSize() << std::endl;
    effect = band.getEffectAt(1);
    std::cout << "number of keyframes: " << effect.getKeyframesSize() << std::endl;*/
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

    /*for (auto v : sig_rec) {
      std::cout << v << std::endl;
    }*/
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

    /*int size = 0;
    for (int i = 0; i < b.getEffectsSize(); i++) {
      for (int j = 0; j < b.getEffectAt(i).getKeyframesSize(); j++) {
        std::cout << b.getEffectAt(i).getKeyframeAt(j).getAmplitudeModulation().value()
                  << std::endl;
        size++;
      }
    }
    std::cout << "total size: " << size << std::endl;*/

    for (double t = 0; t < 1.0/FS*(BL*2); t += 1.0/FS) {
      std::cout << b.Evaluate(t * S_2_MS_TEST, 0, FS) << std::endl;
    }
  }
}

/*TEST_CASE("haptics::filterbank::Wavelet") {

  using haptics::filterbank::Wavelet;

  SECTION("DWT") {

    Wavelet wavelet;
    std::vector<double> in(bl, 0);
    std::vector<double> out(bl, 0);
    std::vector<double> in_rec(bl, 0);
    for (size_t i = 0; i < bl; i++) {
      in[i] = (double)i;
    }
    //in.at(0) = 1;

    wavelet.DWT(in, levels, out);
    wavelet.inv_DWT(out, levels, in_rec);

    bool equal = true;
    for (size_t i = 0; i < bl; i++) {
      if (fabs(in_rec[i] - in[i]) > prec_comparison) {
        equal = false;
        break;
      }
    }
    std::cout << "outputTest" << std::endl;
    for (size_t i = 0; i < bl; i++) {
      std::cout << in[i] << ", " << in_rec[i] << std::endl;
    }
    CHECK(equal);
  }
}*/

TEST_CASE("haptics::filterbank::Filterbank") {

  using haptics::filterbank::Filterbank;
  using haptics::tools::WavParser;

  SECTION("LP") {

    WavParser wavparser;
    wavparser.loadFile("pantheon.wav");

    //std::vector<double> in(BL, 0);
    std::vector<double> in = wavparser.getSamplesChannel(0);
    //in[(BL + 1) / 2 - 1] = 1;
    Filterbank fb(FS);
    std::vector<double> out = fb.LP(in, 72.5);//NOLINT

    std::cout << "LP:" << std::endl;
    /*for (int i = 0; i < BL; i++) {
        std::cout << out[i] << std::endl;
    }*/

    std::string out_name("pantheon_LP.wav");
    WavParser::saveFile(out_name, out, FS);

    CHECK(true);
  }

  SECTION("HP") {

    WavParser wavparser;
    wavparser.loadFile("pantheon.wav");

    // std::vector<double> in(BL, 0);
    std::vector<double> in = wavparser.getSamplesChannel(0);
    //in[(BL + 1) / 2 - 1] = 1;
    Filterbank fb(FS);
    std::vector<double> out = fb.HP(in, 72.5); //NOLINT

    std::cout << "HP:" << std::endl;
    /*for (int i = 0; i < BL; i++) {
        std::cout << out[i] << std::endl;
    }*/
    std::string out_name("pantheon_HP.wav");
    WavParser::saveFile(out_name, out, FS);
    CHECK(true);
  }
}
