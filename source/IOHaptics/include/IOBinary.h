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

#ifndef IOBINARY_H
#define IOBINARY_H

#include <Types/include/Haptics.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <string>

namespace haptics::io {

enum class DeviceInformationMask : uint16_t {
  MAXIMUM_FREQUENCY     = 0b0000'0000'0000'0001,
  MINIMUM_FREQUENCY     = 0b0000'0000'0000'0010,
  RESONANCE_FREQUENCY   = 0b0000'0000'0000'0100,
  MAXIMUM_AMPLITUDE     = 0b0000'0000'0000'1000,
  IMPEDANCE             = 0b0000'0000'0001'0000,
  MAXIMUM_VOLTAGE       = 0b0000'0000'0010'0000,
  MAXIMUM_CURRENT       = 0b0000'0000'0100'0000,
  MAXIMUM_DISPLACEMENT  = 0b0000'0000'1000'0000,
  WEIGHT                = 0b0000'0001'0000'0000,
  SIZE                  = 0b0000'0010'0000'0000,
  TYPE                  = 0b0000'0100'0000'0000,

  NOTHING               = 0b0000'0000'0000'0000,
  ALL                   = 0b1111'1111'1111'1111,
};

class IOBinary {
public:
  static auto loadFile(const std::string &filePath) -> bool;
  static auto writeFile(types::Haptics &haptic, const std::string &filePath) -> bool;

private:
  static auto readFileHeader(types::Haptics &haptic, std::ifstream &file) -> bool;
  static auto readFileBody(types::Haptics &haptic, std::ifstream &file) -> bool;
  static auto readAvatars(types::Haptics &haptic, std::ifstream &file) -> bool;
  static auto readPerceptionsHeader(types::Haptics &haptic, std::ifstream &file) -> bool;
  static auto readReferenceDevices(types::Perception &perception, std::ifstream &file) -> bool;
  static auto readTracksHeader(types::Perception &perception, std::ifstream &file) -> bool;
  static auto readBandHeader(types::Band &band, std::ifstream &file) -> bool;

  static auto writeFileHeader(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writeFileBody(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writeAvatars(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writePerceptionsHeader(types::Haptics &haptic, std::ofstream &file) -> bool;
  static auto writeReferenceDevices(types::Perception &perception, std::ofstream &file) -> bool;
  static auto writeTracksHeader(types::Perception &perception, std::ofstream &file) -> bool;
  static auto writeBandHeader(types::Band &band, std::ofstream &file) -> bool;

  static auto generateReferenceDeviceInformationMask(types::ReferenceDevice &referenceDevice) -> uint16_t;
};
} // namespace haptics::io
#endif // IOBINARY_H
