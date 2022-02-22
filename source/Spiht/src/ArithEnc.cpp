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

#include <Spiht/include/ArithEnc.h>

namespace haptics::spiht {

void ArithEnc::encode(std::vector<unsigned char> &instream, std::vector<int> &context,
                      std::vector<unsigned char> &outstream) {

  // init loop variables
  int range_lower = 0;
  int range_upper = RANGE_MAX;
  int bits_to_follow = 0;
  int range_diff = 0;
  int new_symbol = 0;
  int c = 0;
  int range_add = 0;
  outstream.reserve(instream.size() * 2); // reserve memory with a little buffer

  for (size_t i = 0; i < instream.size(); i++) {

    // calculate range
    range_diff = range_upper - range_lower;
    new_symbol = (unsigned char)instream.at(i);

    c = context.at(i);

    double p = round((double)counter.at(c) / (double)counter_total.at(c) *
                     RANGE_MAX); // p scaled to full range
    range_add = ((int)((double)range_diff * p)) / RANGE_MAX;

    // if p is close to 0 or maximum, value has to be adjusted
    if (range_add == 0) {
      range_add = 1;
    } else if (range_add == range_diff) {
      range_add = range_diff - 1;
    }

    if (new_symbol == 0) {
      range_upper = range_lower + range_add;
    } else {
      range_lower = range_lower + range_add;
    }

    // adjust range to prevent underflow and set output
    while (true) {

      if (range_upper <= HALF) {
        if (bits_to_follow > 0) {
          outstream.push_back(0);
          for (int j = 0; j < bits_to_follow; j++) {
            outstream.push_back(1);
          }
          bits_to_follow = 0;
        } else {
          outstream.push_back(0);
        }
      } else if (range_lower >= HALF) {
        if (bits_to_follow > 0) {
          outstream.push_back(1);
          for (int j = 0; j < bits_to_follow; j++) {
            outstream.push_back(0);
          }
          bits_to_follow = 0;
        } else {
          outstream.push_back(1);
        }
        range_lower -= HALF;
        range_upper -= HALF;
      } else if (range_lower >= FIRST_QTR && range_upper <= THIRD_QTR) {
        bits_to_follow++;
        range_lower -= FIRST_QTR;
        range_upper -= FIRST_QTR;
      } else {
        break;
      }
      range_lower = range_lower << 1;
      range_upper = range_upper << 1;
    }

    // update counter for probabilities
    if (instream.at(i) == 0) {
      counter.at(c)++;
    }
    counter_total.at(c)++;
  }

  // add remainder to output
  remainder(bits_to_follow, outstream, range_lower, range_upper);

  // cut off unnecessary zeros at end
  size_t index_end = outstream.size() - 1;
  while (outstream.at(index_end) == 0 && index_end >= 0) {
    index_end--;
  }
  outstream.resize(index_end + 1);
  rescaleCounter();
}

void ArithEnc::remainder(int bits_to_follow, std::vector<unsigned char> &outstream, int range_lower,
                         int range_upper) {
  if (bits_to_follow > 0) {
    // if bits_to_follow is not reset to 0, setting the LSB of the output to 1
    // is the shortest encoded number in the correct range
    outstream.push_back(1);
  } else {
    int val = HALF;
    while (range_lower > 0) {
      if (val < range_upper) {
        outstream.push_back(1);
        range_lower -= val;
        range_upper -= val;
      } else {
        outstream.push_back(0);
      }
      val = val >> 1;
    }
  }
}

void ArithEnc::resetCounter() {
  for (size_t i = 0; i < CONTEXT_SIZE; i++) {
    counter.at(i) = RESET_TOTAL / 2;
    counter_total.at(i) = RESET_TOTAL;
  }
}

void ArithEnc::rescaleCounter() {
  for (size_t i = 0; i < CONTEXT_SIZE; i++) {
    counter.at(i) = (int)((double)counter.at(i) / (double)(counter_total.at(i) * RESIZE_TOTAL));
    if (counter.at(i) == 0) {
      counter.at(i) = 1;
    }
    counter_total.at(i) = RESIZE_TOTAL;
    if (counter.at(i) == counter_total.at(i)) {
      counter.at(i) = counter_total.at(i) - 1;
    }
  }
}

void ArithEnc::convert2bytes(std::vector<unsigned char> &in, std::vector<unsigned char> &out) {
  out.resize(ceil((double)in.size() / BYTE_SIZE));
  int index = 0;
  for (auto &v : out) {
    std::bitset<BYTE_SIZE> temp;
    for (int j = 0; j < BYTE_SIZE; j++) {
      if (index >= in.size()) {
        break;
      }
      if (in.at(index) == 1) {
        temp[j] = true;
      }
      index++;
    }
    v = (unsigned char)temp.to_ulong();
  }
}

} // namespace haptics::spiht
