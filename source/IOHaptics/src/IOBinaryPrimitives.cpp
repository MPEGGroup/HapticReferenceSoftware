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

#include <IOHaptics/include/IOBinaryPrimitives.h>

namespace haptics::io {

auto IOBinaryPrimitives::writeString(const std::string &text, std::vector<bool> &output) -> void {

  writeNBits<uint8_t, BYTE_SIZE>(static_cast<uint8_t>(text.size()), output);
  for (auto byte : text) {
    for (uint8_t j = 0; j < BYTE_SIZE; j++) {
      output.push_back(((byte >> (BYTE_SIZE - j - 1)) & 1U) == 1);
    }
  }
}

auto IOBinaryPrimitives::readString(std::istream &file, std::vector<bool> &unusedBits)
    -> std::string {
  auto size = IOBinaryPrimitives::readNBits<uint8_t, BYTE_SIZE>(file, unusedBits);
  char c = 0;
  std::string str;
  unsigned int i = 0;
  while (i < size) {
    c = IOBinaryPrimitives::readNBits<char, BYTE_SIZE>(file, unusedBits);
    str += c;
    i++;
  }
  return str;
}
} // namespace haptics::io
