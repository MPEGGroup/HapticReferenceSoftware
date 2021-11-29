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

#include "../include/PcmEncoder.h"

namespace haptics::encoder {

[[nodiscard]] auto PcmEncoder::localExtrema(std::vector<int16_t> signal, bool includeBorder)
    -> std::vector<Point> {
  std::vector<Point> extremaIndexes;
  
  auto it = signal.begin();
  if (it == signal.end()) {
    return {};
  }

  int16_t lastValue = *it;
  ++it;
  if (it == signal.end()) {
    if (includeBorder) {
      Point p1 = Point();
      p1.x = 0;
      p1.y = signal[p1.x];
      extremaIndexes.push_back(p1);
      Point p2 = Point();
      p2.x = 0;
      p2.y = signal[p2.x];
      extremaIndexes.push_back(p2);
      return extremaIndexes;
    }
    return {};
  }

  int16_t value = *it;
  ++it;
  if (it == signal.end()) {
    if (includeBorder) {
      Point p1 = Point();
      p1.x = 0;
      p1.y = signal[p1.x];
      extremaIndexes.push_back(p1);
      Point p2 = Point();
      p2.x = 1;
      p2.y = signal[p2.x];
      extremaIndexes.push_back(p2);
      return extremaIndexes;
    }
    return {};
  }

  Point p{};
  int16_t i = 1;
  int16_t nextValue = 0;
  if (includeBorder) {
    p = Point(0, signal[0]);
    extremaIndexes.push_back(p);
  }
  do {
    nextValue = *it;
    if (((value >= lastValue && value >= nextValue) ||
         (value <= lastValue && value <= nextValue)) &&
        (value != lastValue || value != nextValue)) {

      p = Point(i, value);
      extremaIndexes.push_back(p);
    }

    lastValue = value;
    value = nextValue;
    ++it;
    i++;
  }
  while (it != signal.end());

  if (includeBorder) {
    p = Point(i, value);
    extremaIndexes.push_back(p);
  }

  return extremaIndexes;
}

} // namespace haptics::encoder