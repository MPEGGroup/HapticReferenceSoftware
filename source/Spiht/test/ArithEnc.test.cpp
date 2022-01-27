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

#include <Spiht/include/ArithDec.h>
#include <Spiht/include/ArithEnc.h>
#include <iostream>

constexpr size_t streamsize = 10;

TEST_CASE("haptics::spiht::ArithEnc") {

  using haptics::spiht::ArithDec;
  using haptics::spiht::ArithEnc;

  SECTION("Encoding") {
    ArithEnc enc;
    std::vector<unsigned char> in(streamsize, 0);
    std::vector<int> context(streamsize, 0);
    std::vector<unsigned char> out;
    in[1] = 1;
    enc.encode(in, context, out);
  }

  SECTION("Encoding & Decoding") {
    ArithEnc enc;
    std::vector<unsigned char> in(streamsize, 0);
    std::vector<int> context(streamsize, 0);
    std::vector<unsigned char> out;
    in[1] = 1;
    enc.encode(in, context, out);

    ArithDec dec;
    dec.initDecoding(out);
    bool equal = true;
    for (int i = 0; i < streamsize; i++) {
      if (dec.decode(context[i]) != in[i]) {
        equal = false;
      }
    }
    CHECK(equal);
  }

  SECTION("convert to bytes") {
    ArithEnc enc;
    std::vector<unsigned char> in = {0, 1, 1, 1, 0, 1, 0, 1, 1, 1};
    std::vector<unsigned char> converted;
    std::vector<unsigned char> out;
    ArithEnc::convert2bytes(in, converted);
    ArithDec::convert2bits(converted, out);
    bool equal = true;
    for (int i = 0; i < in.size(); i++) {
      if (out[i] != in[i]) {
        equal = false;
      }
    }
    CHECK(equal);
    if (!equal) {
      std::cout << "output in bits:" << std::endl;
      for (auto v : out) {
        std::cout << (int)v << std::endl;
      }
    }
  }
}
