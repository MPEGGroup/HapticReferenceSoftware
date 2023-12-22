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

#ifndef SPIHT_DEC_H
#define SPIHT_DEC_H

#include <cmath>
#include <iostream>
#include <list>
#include <vector>

#include <Spiht/include/ArithDec.h>
#include <Spiht/include/Spiht_Enc.h>
#include <Types/include/Effect.h>

namespace haptics::spiht {

class Spiht_Dec {
public:
  void decodeEffect(std::vector<unsigned char> &in, std::vector<double> block, int origlength,
                    double &wavmax, int &bits);
  void decode(std::vector<unsigned char> &bitstream, std::vector<int> &out, int origlength,
              int level, double &wavmax, int &n_real);

private:
  void initLists(int origlength, int level);
  auto getMaxAllocBits() -> int;
  auto getWavmax() -> double;
  void sortingPass(std::vector<int> &out, int origlength, int compare);
  void refinementPass(std::vector<int> &out, int LSP_index, int compare);
  auto getBit(int context) -> int;
  void getBits(std::vector<int> &out, int length, int context);
  template <typename T> auto bi2de(std::vector<T> &data, int length) -> T;
  auto static sgn(int val) -> int;

  std::list<int> LIP;
  std::list<int> LIS1;
  std::list<int> LIS2;
  std::list<int> LSP;

  ArithDec arithDec;
};
} // namespace haptics::spiht
#endif // SPIHT_DEC_H
