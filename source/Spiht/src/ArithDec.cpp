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

#include <Spiht/include/ArithDec.h>

namespace haptics::spiht {

void ArithDec::initDecoding(std::vector<unsigned char> &instream) {
  this->instream = instream;
  in_index = 0;
  max_index = instream.size() - 1;

  // get first 10 digits
  in_leading = 0;
  int shift = SHIFT_START;
  for (int i = 0; i < DIGITS; i++) {
    if (i < instream.size()) {
      in_leading += (int)instream.at(in_index) << shift;
      in_index++;
      shift--;
    } else {
      break;
    }
  }

  range_diff = RANGE_MAX;
  range_lower = 0;
  range_upper = RANGE_MAX;
}

auto ArithDec::decode(int context) -> int {

  double p = round((double)counter.at(context) / (double)counter_total.at(context) *
                   RANGE_MAX); // p scaled to full range
  int compare = ((int)((double)range_diff * p)) / RANGE_MAX;

  // if p is close to 0 or maximum, value has to be adjusted
  if (compare == 0) {
    compare = 1;
  } else if (compare == range_diff) {
    compare = range_diff - 1;
  }

  int value = in_leading - range_lower;

  // determine decoded symbol; range is updated
  int s = 0;
  if (value < compare) {
    range_upper = range_lower + compare;
  } else {
    s = 1;
    range_lower = range_lower + compare;
  }

  // check, if range has to be adjusted
  while (true) {

    if (range_upper <= HALF) {
      range_lower = range_lower << 1;
      range_upper = range_upper << 1;
      if (in_index <= max_index) {
        in_leading = (in_leading << 1) + instream.at(in_index);
        in_index++;
      } else {
        in_leading = in_leading << 1;
      }
    } else if (range_lower >= HALF) {
      range_lower = (range_lower - HALF) << 1;
      range_upper = (range_upper - HALF) << 1;
      if (in_index <= max_index) {
        in_leading = ((in_leading - HALF) << 1) + instream.at(in_index);
        in_index++;
      } else {
        in_leading = (in_leading - HALF) << 1;
      }
    } else if (range_lower >= FIRST_QTR && range_upper <= THIRD_QTR) {
      range_lower = (range_lower - FIRST_QTR) << 1;
      range_upper = (range_upper - FIRST_QTR) << 1;
      if (in_index <= max_index) {
        in_leading = ((in_leading - FIRST_QTR) << 1) + instream.at(in_index);
        in_index++;
      } else {
        in_leading = (in_leading - FIRST_QTR) << 1;
      }
    } else {
      break;
    }
  }

  range_diff = range_upper - range_lower;

  // update counter for probabilities
  if (s == 0) {
    counter.at(context)++;
  }
  counter_total.at(context)++;

  return s;
}

void ArithDec::resetCounter() {
  for (int i = 0; i < CONTEXT_SIZE; i++) {
    counter.at(i) = RESET_TOTAL / 2;
    counter_total.at(i) = RESET_TOTAL;
  }
}

void ArithDec::rescaleCounter() {
  for (int i = 0; i < CONTEXT_SIZE; i++) {
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

void ArithDec::convert2bits(std::vector<unsigned char> &in, std::vector<unsigned char> &out) {
  out.resize(in.size() * BYTE_SIZE);
  int index = 0;
  for (auto &v : in) {
    std::bitset<BYTE_SIZE> temp((unsigned long)v);
    for (int j = 0; j < BYTE_SIZE; j++) {
      if (temp[j]) {
        out.at(index) = 1;
      }
      index++;
    }
  }
}

} // namespace haptics::spiht
