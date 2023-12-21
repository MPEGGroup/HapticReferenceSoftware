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

#ifndef SPIHT_ENC_H
#define SPIHT_ENC_H

#include <cmath>
#include <iostream>
#include <list>
#include <vector>

#include <Spiht/include/ArithEnc.h>
#include <Types/include/Effect.h>

namespace haptics::spiht {

using haptics::types::Effect;

constexpr size_t WAVMAXLENGTH = 8;
constexpr int MAXBITS = 15;
constexpr int FRACTIONBITS_0 = 7;
constexpr int FRACTIONBITS_1 = 3;
constexpr int INTEGERBITS_1 = 4;

constexpr size_t MAXALLOCBITS_SIZE = 4;
constexpr int CONTEXT_0 = 0;
constexpr int CONTEXT_1 = 1;
constexpr int CONTEXT_2 = 2;
constexpr int CONTEXT_3 = 3;
constexpr int CONTEXT_4 = 4;
constexpr int CONTEXT_5 = 5;
constexpr int CONTEXT_6 = 6;
constexpr size_t BUFFER_SIZE = 100000;

struct quantMode {
  int integerbits;
  int fractionbits;
  char mode;
};

class Spiht_Enc {
public:
  void encodeEffect(std::vector<int> &block, int bits, double scalar, std::vector<unsigned char> &outstream);
  void encode(std::vector<int> &instream, int level, std::vector<unsigned char> &bitwavmax,
              int maxallocbits, std::vector<unsigned char> &outstream, std::vector<int> &context);

  auto maxDescendant(int j, int type) -> int;
  void initMaxDescendants(std::vector<int> &signal);
  auto static getQuantMode(double wavmax) -> quantMode;
  void static setBitwavmax(double qwavmax, int integerpart, quantMode m, std::vector<unsigned char> &bitwavmax);

private:
  void static refinementPass(std::vector<int> &data, std::list<int> &LSP, int LSP_index, int n,
                             std::vector<unsigned char> &outstream, std::vector<int> &context);

  void static addToOutput(unsigned char bit, int c, std::vector<unsigned char> &outstream,
                          std::vector<int> &context);

  static void maximumWaveletCoefficient(double qwavmax, std::vector<unsigned char> &bitwavmax);
  void static de2bi(int val, std::vector<unsigned char> &outstream, int length);
  auto static bitget(int in, int bit) -> int;

  std::vector<int> maxDescendants;
  std::vector<int> maxDescendants1;

  ArithEnc arithEnc;
};
} // namespace haptics::spiht
#endif // SPIHT_ENC_H
