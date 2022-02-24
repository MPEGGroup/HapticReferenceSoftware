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

#include <Spiht/include/Spiht_Enc.h>

namespace haptics::spiht {

void Spiht_Enc::encodeEffect(Effect &effect, std::vector<unsigned char> &outstream) {
  auto bl = (int)effect.getKeyframesSize() - 2;
  double scalar = effect.getKeyframeAt(bl).getAmplitudeModulation().value();
  auto bits = (int)effect.getKeyframeAt(bl + 1).getAmplitudeModulation().value();
  double multiplier = pow(2, (double)bits);
  std::vector<int> block(bl, 0);
  int index = 0;
  for (auto &v : block) {
    v = (int)((double)effect.getKeyframeAt(index).getAmplitudeModulation().value() * multiplier);
    index++;
  }
  std::vector<unsigned char> bitwavmax;
  maximumWaveletCoefficient(scalar, bitwavmax);
  auto level = (int)(log2((double)bl) - 2);
  std::vector<int> context;
  std::vector<unsigned char> stream_spiht;
  encode(block, level, bitwavmax, bits, stream_spiht, context);
  std::vector<unsigned char> stream_arithmetic;
  arithEnc.encode(stream_spiht, context, stream_arithmetic);
  ArithEnc::convert2bytes(stream_arithmetic, outstream);
}

void Spiht_Enc::encode(std::vector<int> &instream, int level, std::vector<unsigned char> &bitwavmax,
                       int maxallocbits, std::vector<unsigned char> &outstream,
                       std::vector<int> &context) {

  outstream.reserve(outstream.size() + BUFFER_SIZE);
  context.reserve(context.size() + BUFFER_SIZE);
  size_t length = instream.size();
  // add maxallocbits to stream
  de2bi(maxallocbits, outstream, MAXALLOCBITS_SIZE);
  // add bitwavmax to stream
  outstream.insert(outstream.end(), bitwavmax.begin(), bitwavmax.end());

  // context
  std::vector<int> c(MAXALLOCBITS_SIZE + bitwavmax.size(), CONTEXT_0);
  context.insert(context.end(), c.begin(), c.end());

  // init LIP, LSP, LIS
  int bandsize = 2 << ((int)log2((double)length) - level);
  std::list<int> LIP;
  for (int i = 0; i < bandsize; i++) {
    LIP.push_back(i);
  }
  std::list<int> LIS1;
  std::list<int> LIS2;
  for (int i = (bandsize / 2); i < bandsize; i++) {
    LIS1.push_back(i);
    LIS2.push_back(0);
  }
  std::list<int> LSP;

  initMaxDescendants(instream);

  int n = maxallocbits;
  while (0 <= n) {
    int compare = 1 << n; // 2^n
    int LSP_index = (int)LSP.size();
    // sorting pass
    std::list<int>::iterator it;
    for (it = LIP.begin(); it != LIP.end();) {
      if (abs(instream[*it]) >= compare) {
        addToOutput(1, CONTEXT_2, outstream, context);
        addToOutput((char)(instream[*it] >= 0), CONTEXT_1, outstream, context);
        LSP.push_back(*it);
        it = LIP.erase(it);
      } else {
        addToOutput(0, CONTEXT_2, outstream, context);
        it++;
      }
    }

    auto it1 = LIS1.begin();
    auto it2 = LIS2.begin();
    int LISsize = (int)LIS1.size();
    for (int i = 0; i < LISsize; i++) {
      // Type A
      if (*it2 == 0) {
        int max_d = maxDescendant(*it1, *it2);
        if (max_d >= compare) {
          addToOutput(1, CONTEXT_3, outstream, context);
          size_t y = *it1;
          // Children
          int index = 2 * (int)y;
          if (fabs(instream[index]) >= compare) {
            LSP.push_back(index);
            addToOutput(1, CONTEXT_4, outstream, context);
            addToOutput((char)(instream[index] >= 0), CONTEXT_1, outstream, context);
          } else {
            addToOutput(0, CONTEXT_4, outstream, context);
            LIP.push_back(index);
          }
          index = 2 * (int)y + 1;
          if (fabs(instream[index]) >= compare) {
            LSP.push_back(index);
            addToOutput(1, CONTEXT_4, outstream, context);
            addToOutput((char)(instream[index] >= 0), CONTEXT_1, outstream, context);
          } else {
            addToOutput(0, CONTEXT_4, outstream, context);
            LIP.push_back(index);
          }
          // Grandchildren
          if ((4 * y + 3) < length) {
            LIS1.push_back(*it1);
            LIS2.push_back(1);
            LISsize++;
          }
          it1 = LIS1.erase(it1);
          it2 = LIS2.erase(it2);
        } else {
          addToOutput(0, CONTEXT_3, outstream, context);
          it1++;
          it2++;
        }
        // type B
      } else {
        int max_d = maxDescendant(*it1, *it2);
        if (max_d >= compare) {
          addToOutput(1, CONTEXT_5, outstream, context);
          int y = *it1;
          LIS1.push_back(2 * y);
          LIS1.push_back(2 * y + 1);
          LIS2.push_back(0);
          LIS2.push_back(0);
          LISsize += 2;
          it1 = LIS1.erase(it1);
          it2 = LIS2.erase(it2);
        } else {
          addToOutput(0, CONTEXT_5, outstream, context);
          it1++;
          it2++;
        }
      }
    }

    refinementPass(instream, LSP, LSP_index, n, outstream, context);
    n--;
  }
}

void Spiht_Enc::refinementPass(std::vector<int> &data, std::list<int> &LSP, int LSP_index, int n,
                               std::vector<unsigned char> &outstream, std::vector<int> &context) {
  auto it = LSP.begin();
  int temp = 0;
  while (temp < LSP_index) {

    int s = bitget((int)floor(fabs(data[*it])), n + 1);
    outstream.push_back((unsigned char)s);
    context.push_back(CONTEXT_6);
    temp++;
    it++;
  }
}

void Spiht_Enc::addToOutput(unsigned char bit, int c, std::vector<unsigned char> &outstream,
                            std::vector<int> &context) {
  outstream.push_back(bit);
  context.push_back(c);
}

auto Spiht_Enc::maxDescendant(int j, int type) -> int {
  if (type == 1) {
    if (j >= (int)maxDescendants1.size()) {
      std::cerr << "maxDescendants1 out of bounds" << std::endl;
      return 0;
    }
    return maxDescendants1[j];
  }
  if (j >= (int)maxDescendants.size()) {
    std::cerr << "maxDescendants out of bounds" << std::endl;
    return 0;
  }
  return maxDescendants[j];
}

void Spiht_Enc::initMaxDescendants(std::vector<int> &signal) {

  size_t length = signal.size();
  size_t start = length >> 1;

  maxDescendants.resize(start);
  maxDescendants1.resize(start >> 1);

  size_t p1 = start;
  size_t p2 = p1 + 1;
  size_t target = start >> 1;

  for (size_t i = 0; i < (start >> 1); i++) {
    int v1 = abs(signal[p1]);
    int v2 = abs(signal[p2]);
    if (v1 > v2) {
      maxDescendants[target] = v1;
    } else {
      maxDescendants[target] = v2;
    }

    p1 += 2;
    p2 += 2;
    target++;
  }

  size_t width = start >> 1;
  p1 = width;
  p2 = p1 + 1;
  target = width >> 1;

  while (target > 1) {
    for (size_t i = 0; i < (width >> 1); i++) {
      int v1 = maxDescendants[p1];
      int v2 = maxDescendants[p2];
      if (v1 > v2) {
        maxDescendants1[target] = v1;
      } else {
        maxDescendants1[target] = v2;
      }
      v1 = abs(signal[p1]);
      if (v1 > maxDescendants1[target]) {
        maxDescendants[target] = v1;
      } else {
        maxDescendants[target] = maxDescendants1[target];
      }
      v2 = abs(signal[p2]);
      if (v2 > maxDescendants[target]) {
        maxDescendants[target] = v2;
      }
      p1 += 2;
      p2 += 2;
      target++;
    }
    width = width >> 1;
    p1 = width;
    p2 = p1 + 1;
    target = width >> 1;
  }
}

void Spiht_Enc::maximumWaveletCoefficient(double qwavmax, std::vector<unsigned char> &bitwavmax) {

  int integerpart = 0;
  char mode = 0;
  quantMode m = {0, 0};
  if (qwavmax < 1) {
    m.integerbits = 0;
    m.fractionbits = FRACTIONBITS_0;
  } else {
    integerpart = 1;
    m.integerbits = INTEGERBITS_1;
    m.fractionbits = FRACTIONBITS_1;
    mode = 1;
  }

  bitwavmax.clear();
  bitwavmax.reserve(WAVMAXLENGTH);
  bitwavmax.push_back(mode);
  de2bi((int)((qwavmax - (double)integerpart) * pow(2, (double)m.fractionbits)), bitwavmax,
        m.integerbits + m.fractionbits);
}

void Spiht_Enc::de2bi(int val, std::vector<unsigned char> &outstream, int length) {
  int n = length;
  while (n > 0) {
    outstream.push_back((char)(val % 2));
    val = val >> 1;
    n--;
  }
}

auto Spiht_Enc::bitget(int in, int bit) -> int {
  int mask = 1 << (bit - 1);

  if ((in & mask) > 0) {
    return 1;
  }
  return 0;
}

} // namespace haptics::spiht
