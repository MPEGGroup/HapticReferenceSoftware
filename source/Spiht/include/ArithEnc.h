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

#ifndef ARITHENC_H
#define ARITHENC_H

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

namespace haptics::spiht {

constexpr int RANGE_MAX = 1024;
constexpr int HALF = 512;
constexpr int FIRST_QTR = 256;
constexpr int THIRD_QTR = 768;
constexpr size_t CONTEXT_SIZE = 7;
constexpr int RESET_HALF = 8;
constexpr int RESET_TOTAL = 16;
constexpr int RESIZE_TOTAL = 32;
constexpr int BYTE_SIZE = 8;

class ArithEnc {
public:
  void ArithEnc::encode(std::vector<unsigned char> &instream, std::vector<int> &context,
                        std::vector<unsigned char> &outstream);

  void resetCounter();
  void static convert2bytes(std::vector<unsigned char> &in, std::vector<unsigned char> &out);

private:
  void static remainder(int bits_to_follow, std::vector<unsigned char> &outstream, int range_lower,
                        int range_upper);

  void rescaleCounter();

  std::array<int, CONTEXT_SIZE> counter = {RESET_HALF, RESET_HALF, RESET_HALF, RESET_HALF,
                                           RESET_HALF, RESET_HALF, RESET_HALF};
  std::array<int, CONTEXT_SIZE> counter_total = {RESET_TOTAL, RESET_TOTAL, RESET_TOTAL, RESET_TOTAL,
                                                 RESET_TOTAL, RESET_TOTAL, RESET_TOTAL};

  std::array<int, BYTE_SIZE> masks = {0b00000001, 0b00000010, 0b00000100, 0b00001000,
                                      0b00010000, 0b00100000, 0b01000000, 0b10000000};
};
} // namespace haptics::spiht
#endif // ARITHENC_H
