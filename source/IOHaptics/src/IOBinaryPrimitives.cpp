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

auto IOBinaryPrimitives::writeString(const std::string &text, std::ofstream &file) -> void {
  std::string str = text;
  str.append(1, '\x00');
  file.write(str.c_str(), static_cast<int>(str.size()));
}

auto IOBinaryPrimitives::readString(std::ifstream &file) -> std::string {
  char c = 0;
  std::string str;
  while (file.get(c), c != '\0') {
    str += c;
  }
  return str;
}

auto IOBinaryPrimitives::writeFloat(float f, std::ofstream &file) -> void {
  std::array<char, 4> bytes{};
  memcpy(&bytes, &f, sizeof(f));
  file.write(bytes.data(), 4);
}

auto IOBinaryPrimitives::readFloat(std::ifstream &file) -> float {
  float value = 0;
  std::array<char, 4> bytes{};
  file.read(bytes.data(), 4);
  memcpy(&value, &bytes, sizeof(value));
  return value;
}
} // namespace haptics::io
