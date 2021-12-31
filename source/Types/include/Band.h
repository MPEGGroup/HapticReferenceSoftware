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

#ifndef BAND_H
#define BAND_H

#include <Types/include/Effect.h>
#include <vector>

namespace haptics::types {

enum class BandType {
  Wave = 0,
  Curve = Wave + 1,
  Transient = Curve + 1,
};

enum class EncodingModality {
  Quantized = 0,
  Vectorial = Quantized + 1,
};

class Band {
public:
  explicit Band() = default;
  explicit Band(BandType newBandType, EncodingModality newEncodingModality, int newWindowLength,
                int newLowerFrequencyLimit, int newUpperFrequencyLimit)
      : bandType(newBandType)
      , encodingModality(newEncodingModality)
      , windowLength(newWindowLength)
      , lowerFrequencyLimit(newLowerFrequencyLimit)
      , upperFrequencyLimit(newUpperFrequencyLimit)
      , effects({}) {};

  [[nodiscard]] auto getBandType() const -> BandType;
  auto setBandType(BandType newBandType) -> void;
  [[nodiscard]] auto getEncodingModality() const -> EncodingModality;
  auto setEncodingModality(EncodingModality newEncodingModality) -> void;
  [[nodiscard]] auto getWindowLength() const -> int;
  auto setWindowLength(int newWindowLength) -> void;
  [[nodiscard]] auto getUpperFrequencyLimit() const -> int;
  auto setUpperFrequencyLimit(int newUpperFrequencyLimit) -> void;
  [[nodiscard]] auto getLowerFrequencyLimit() const -> int;
  auto setLowerFrequencyLimit(int newLowerFrequencyLimit) -> void;
  auto getEffectsSize() -> size_t;
  auto getEffectAt(int index) -> haptics::types::Effect &;
  auto addEffect(haptics::types::Effect &newEffect) -> void;
  auto replaceEffectAt(int index, haptics::types::Effect &newEffect) -> bool;
  [[nodiscard]] auto isOverlapping(haptics::types::Effect &effect, int start, int stop)
      -> bool;
  auto Evaluate(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double;

private:
  static constexpr double TRANSIENT_DURATION_MS = 22;
  auto EvaluationSwitch(double position, haptics::types::Effect *effect, int lowFrequencyLimit,
                        int highFrequencyLimit) -> double;

  BandType bandType = BandType::Wave;
  EncodingModality encodingModality = EncodingModality::Quantized;
  int windowLength = 0;
  int lowerFrequencyLimit = 0;
  int upperFrequencyLimit = 0;
  std::vector<Effect> effects = std::vector<Effect>{};
};
} // namespace haptics::types

#endif //BAND_H
