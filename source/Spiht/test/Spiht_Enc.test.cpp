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

#include <Spiht/include/ArithEnc.h>
#include <Spiht/include/Spiht_Dec.h>
#include <Spiht/include/Spiht_Enc.h>
#include <iostream>

constexpr size_t bl = 512;
constexpr size_t level = 7;
constexpr int BITS_EFFECT = 15;
constexpr float precision = 0.00000000001;
constexpr int MOD_VAL = 256;
constexpr float scalar = 1.5;
constexpr float scalar2 = 0.5;

TEST_CASE("haptics::spiht::Spiht_Enc") {

  using haptics::spiht::ArithEnc;
  using haptics::spiht::Spiht_Dec;
  using haptics::spiht::Spiht_Enc;

  SECTION("maxDescendant") {
    Spiht_Enc enc;
    std::vector<int> signal(bl, 0);
    signal.at(bl / 2) = 1;
    enc.initMaxDescendants(signal);
    CHECK(enc.maxDescendant(2, 0) == 1);
    CHECK(enc.maxDescendant(3, 0) == 0);
    CHECK(enc.maxDescendant(bl / 4, 0) == 1);
    CHECK(enc.maxDescendant(bl / 4, 1) == 0);
  }

  SECTION("Encoder") {
    Spiht_Enc enc;
    std::vector<int> signal(bl, 0);
    signal.at(bl / 2) = 1;
    std::vector<unsigned char> bitwavmax(haptics::spiht::WAVMAXLENGTH, 0);
    bitwavmax.at(0) = 1;
    std::vector<unsigned char> outstream;
    std::vector<int> context;
    enc.encode(signal, level, bitwavmax, 1, outstream, context);
  }

  SECTION("Encoder & Decoder") {
    Spiht_Enc enc;
    std::vector<int> signal(bl, 0);
    signal.at(bl / 2) = 1;
    signal.at(0) = 4;
    signal.at(3) = 3;
    std::vector<unsigned char> bitwavmax(haptics::spiht::WAVMAXLENGTH, 0);
    bitwavmax.at(0) = 1;
    std::vector<unsigned char> stream_spiht;
    std::vector<int> context;
    enc.encode(signal, level, bitwavmax, 4, stream_spiht, context);
    ArithEnc arithEnc;
    std::vector<unsigned char> outstream_arithmetic;
    arithEnc.encode(stream_spiht, context, outstream_arithmetic);

    Spiht_Dec dec;
    std::vector<int> signal_rec(bl, 0);
    double wavmax = 0;
    int n_real = 0;
    dec.decode(outstream_arithmetic, signal_rec, bl, level, wavmax, n_real);
    bool equal = true;
    for (size_t i = 0; i < bl; i++) {
      if (signal.at(i) != signal_rec.at(i)) {
        equal = false;
      }
    }
    CHECK(equal);
    if (!equal) {
      std::cout << "decoder output:" << std::endl;
      for (auto v : signal_rec) {
        std::cout << v << std::endl;
      }
    }
    CHECK(std::fabs(wavmax - 1) < precision);
  }
}

TEST_CASE("haptics::spiht::Spiht_Enc, wavmax") {

  using haptics::spiht::Spiht_Dec;
  using haptics::spiht::Spiht_Enc;

  SECTION("QuantMode > 1") {
    auto m = Spiht_Enc::getQuantMode(scalar);
    CHECK(m.mode == 1);
    CHECK(m.integerbits == haptics::spiht::INTEGERBITS_1);
    CHECK(m.fractionbits == haptics::spiht::FRACTIONBITS_1);
  }

  SECTION("QuantMode < 1") {
    auto m = Spiht_Enc::getQuantMode(0);
    CHECK(m.mode == 0);
    CHECK(m.integerbits == 0);
    CHECK(m.fractionbits == haptics::spiht::FRACTIONBITS_0);
  }

  SECTION("Example wavmax") {
    std::vector<unsigned char> bitwavmax;
    Spiht_Enc::maximumWaveletCoefficient(scalar, bitwavmax);
    CHECK(bitwavmax[0] == 1);
    CHECK(bitwavmax[1] == 0);
    CHECK(bitwavmax[2] == 0);
    CHECK(bitwavmax[3] == 0);
    CHECK(bitwavmax[4] == 0);
    CHECK(bitwavmax[5] == 1);
  }

  SECTION("Example wavmax 2") {
    std::vector<unsigned char> bitwavmax;
    Spiht_Enc::maximumWaveletCoefficient(scalar2, bitwavmax);
    CHECK(bitwavmax[0] == 0);
    CHECK(bitwavmax[1] == 1);
    CHECK(bitwavmax[2] == 0);
  }
}

TEST_CASE("haptics::spiht::Spiht_Enc,2") {

  using haptics::spiht::ArithEnc;
  using haptics::spiht::Spiht_Dec;
  using haptics::spiht::Spiht_Enc;
  using haptics::types::Effect;

  SECTION("Effect Encoding") {
    Spiht_Enc enc;
    std::vector<int> in;
    for (size_t i = 0; i < bl; i++) {
      in.push_back((int)((float)(i % MOD_VAL) / MOD_VAL));
    }
    std::vector<unsigned char> stream_enc;
    enc.encodeEffect(in, BITS_EFFECT, scalar, stream_enc);

    Spiht_Dec dec;
    std::vector<int> out;
    bool equal = true;
    double scalar_out = 0;
    int bits_out = 0;
    dec.decodeEffect(stream_enc, out, bl, scalar_out, bits_out);
    for (int i = 0; i < (int)bl; i++) {
      if (!(std::fabs(in[i] - out[i]) < precision)) {
        equal = false;
      }
    }
    CHECK(equal);
    CHECK(std::fabs(scalar - scalar_out) < precision);
    if (!equal) {
      for (int i = 0; i < (int)bl; i++) {
        std::cout << in[i] << "," << out[i] << std::endl;
      }
      std::cout << "bits: " << BITS_EFFECT << "," << bits_out << std::endl;
    }
    if (std::fabs(scalar - scalar_out) >= precision) {
      std::cout << "scalar: " << scalar << "," << scalar_out << std::endl;
    }
  }
}
