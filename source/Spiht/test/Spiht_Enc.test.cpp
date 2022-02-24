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
  }
}

TEST_CASE("haptics::spiht::Spiht_Enc,2") {

  using haptics::spiht::ArithEnc;
  using haptics::spiht::Spiht_Dec;
  using haptics::spiht::Spiht_Enc;
  using haptics::types::Effect;

  SECTION("Effect Encoding") {
    Spiht_Enc enc;
    Effect effect_in;
    for (size_t i = 0; i < bl; i++) {
      Keyframe keyframe(i, (float)(i % MOD_VAL) / MOD_VAL, 0);
      effect_in.addKeyframe(keyframe);
    }
    Keyframe keyframe(bl, (float)1, 0);
    effect_in.addKeyframe(keyframe);
    Keyframe keyframeBits(bl + 1, (float)BITS_EFFECT, 0);
    effect_in.addKeyframe(keyframeBits);
    std::vector<unsigned char> stream_enc;
    enc.encodeEffect(effect_in, stream_enc);

    Spiht_Dec dec;
    Effect effect_out;
    bool equal = true;
    dec.decodeEffect(stream_enc, effect_out, bl);
    for (int i = 0; i < (int)bl; i++) {
      if (!(fabs(effect_in.getKeyframeAt(i).getAmplitudeModulation().value() -
                 effect_out.getKeyframeAt(i).getAmplitudeModulation().value()) < precision)) {
        equal = false;
      }
    }
    CHECK(equal);
    if (!equal) {
      for (int i = 0; i < (int)bl; i++) {
        std::cout << effect_in.getKeyframeAt(i).getAmplitudeModulation().value() << ","
                  << effect_out.getKeyframeAt(i).getAmplitudeModulation().value() << std::endl;
      }
      std::cout << "scalar: " << effect_in.getKeyframeAt(bl).getAmplitudeModulation().value() << ","
                << effect_out.getKeyframeAt(bl).getAmplitudeModulation().value() << std::endl;
      std::cout << "bits: " << effect_in.getKeyframeAt(bl + 1).getAmplitudeModulation().value()
                << "," << effect_out.getKeyframeAt(bl + 1).getAmplitudeModulation().value()
                << std::endl;
    }
  }
}
