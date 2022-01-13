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

#include <Spiht/include/Spiht_Dec.h>

namespace haptics::spiht {

void Spiht_Dec::decode(std::vector<char> &bitstream, std::vector<int> &out, int origlength,
                       int level, double &wavmax, int &n_real) {

  // arithDec->initDecoding(bitstream, pos, streamlength);
  instream = bitstream;
  out.resize(origlength, 0);
  n_real = getMaxAllocBits();
  wavmax = getWavmax();
  initLists(origlength, level);

  int n = n_real;
  while (0 <= n) {
    int compare = 1 << n; // 2^n
    int LSP_index = (int)LSP.size();
    
    sortingPass(out, origlength, compare);

    refinementPass(out, LSP_index, compare);

    n--;
  }

  // arithDec->rescaleCounter();
}

void Spiht_Dec::initLists(int origlength, int level) {
  int bandsize = 2 << ((int)log2((double)origlength) - level);
  LIP.clear();
  LIS1.clear();
  LIS2.clear();
  LSP.clear();
  for (int i = 0; i < bandsize; i++) {
    LIP.push_back(i);
  }
  for (int i = (bandsize / 2); i < bandsize; i++) {
    LIS1.push_back(i);
    LIS2.push_back(0);
  }
}

auto Spiht_Dec::getMaxAllocBits() -> int {
  std::vector<int> maxallocbitsArray(MAXALLOCBITS_SIZE, 0);
  getBits(maxallocbitsArray, MAXALLOCBITS_SIZE, CONTEXT_0);
  return bi2de(maxallocbitsArray, MAXALLOCBITS_SIZE);
}

auto Spiht_Dec::getWavmax() -> double {
  int mode = getBit(CONTEXT_0);
  std::vector<int> wavmaxArray(WAVMAX_SIZE - 1, 0);
  getBits(wavmaxArray, WAVMAX_SIZE - 1, 0);
  int temp = bi2de(wavmaxArray, WAVMAX_SIZE - 1);
  double wavmax = 0;
  if (mode == 0) {
    wavmax = (double)temp * pow(2, -FRACTIONBITS_0);
  } else {
    wavmax = (double)temp * pow(2, -4) + 1;
  }
  return wavmax;
}

void Spiht_Dec::sortingPass(std::vector<int> &out, int origlength, int compare) {
  std::list<int>::iterator it;
  for (it = LIP.begin(); it != LIP.end();) {
    if (getBit(CONTEXT_2) == 1) {
      if (getBit(CONTEXT_1) == 1) {
        out[*it] = compare;
      } else {
        out[*it] = -compare;
      }
      LSP.push_back(*it);
      it = LIP.erase(it);
    } else {
      it++;
    }
  }

  auto it1 = LIS1.begin();
  auto it2 = LIS2.begin();
  int LISsize = (int)LIS1.size();
  for (int i = 0; i < LISsize; i++) {
    // type A
    if (*it2 == 0) {
      if (getBit(CONTEXT_3) == 1) {
        int y = *it1;
        // Children
        int index = 2 * y;
        if (getBit(CONTEXT_4) == 1) {
          LSP.push_back(index);
          if (getBit(CONTEXT_1) == 1) {
            out[index] = compare;
          } else {
            out[index] = -compare;
          }
        } else {
          LIP.push_back(index);
        }

        index = 2 * y + 1;
        if (getBit(CONTEXT_4) == 1) {
          LSP.push_back(index);
          if (getBit(CONTEXT_1) == 1) {
            out[index] = compare;
          } else {
            out[index] = -compare;
          }
        } else {
          LIP.push_back(index);
        }

        // Grandchildren
        if ((4 * y + 3) < origlength) {
          LIS1.push_back(*it1);
          LIS2.push_back(1);
          LISsize++;
        }
        it1 = LIS1.erase(it1);
        it2 = LIS2.erase(it2);
      } else {
        it1++;
        it2++;
      }

      // type B
    } else {
      if (getBit(CONTEXT_5) == 1) {
        int y = *it1;
        LIS1.push_back(2 * y);
        LIS1.push_back(2 * y + 1);
        LIS2.push_back(0);
        LIS2.push_back(0);
        LISsize += 2;
        it1 = LIS1.erase(it1);
        it2 = LIS2.erase(it2);
      } else {
        it1++;
        it2++;
      }
    }
  }
}

void Spiht_Dec::refinementPass(std::vector<int> &out, int LSP_index, int compare) {
  auto it = LSP.begin();
  int temp = 0;
  while (temp < LSP_index) {
    if (getBit(CONTEXT_6) == 1) {
      out[*it] += sgn(out[*it]) * compare;
    }
    temp++;
    it++;
  }
}

auto Spiht_Dec::getBit(int context) -> int { // NOLINT
  int out = (unsigned char)instream.at(0);
  instream.erase(instream.begin());
  return out;
}

void Spiht_Dec::getBits(std::vector<int> &out, int length, int context) { // NOLINT
  out.resize(length);
  for (int i = 0; i < length; i++) {
    out[i] = (unsigned char)instream.at(0);
    instream.erase(instream.begin());
  }
}

template <typename T> auto Spiht_Dec::bi2de(std::vector<T> &data, int length) -> T {
  T val = 0;
  for (int i = 0; i < length; i++) {
    val += data.at(i) << i;
  }
  return val;
}

auto Spiht_Dec::sgn(int val) -> int { return (int)(0 < val) - (int)(val < 0); }

} // namespace haptics::spiht
