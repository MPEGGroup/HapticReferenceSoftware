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

#ifndef ARITHDEC_H
#define ARITHDEC_H

#include <array>
#include <iostream>
#include <vector>

#include <Spiht/include/ArithEnc.h>

namespace haptics::spiht {

constexpr int SHIFT_START = 9;
constexpr int DIGITS = 10;

class ArithDec {
public:
  void initDecoding(std::vector<unsigned char> &instream);
  auto decode(int context) -> int;

  void resetCounter();
  void static convert2bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out);

  void rescaleCounter();

private:
  std::array<int, CONTEXT_SIZE> counter = {RESET_HALF, RESET_HALF, RESET_HALF, RESET_HALF,
                                           RESET_HALF, RESET_HALF, RESET_HALF};
  std::array<int, CONTEXT_SIZE> counter_total = {RESET_TOTAL, RESET_TOTAL, RESET_TOTAL, RESET_TOTAL,
                                                 RESET_TOTAL, RESET_TOTAL, RESET_TOTAL};
  std::vector<unsigned char> instream;

  size_t in_index = 0;
  size_t max_index = 0;

  int range_diff = 0;
  int range_lower = 0;
  int range_upper = 0;

  int in_leading = 0;
};
} // namespace haptics::spiht
#endif // ARITDEC_H
