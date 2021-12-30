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

#include <Types/include/Band.h>

namespace haptics::types {

  [[nodiscard]] auto Band::getBandType() const -> BandType {
    return bandType;
  }

  auto Band::setBandType(BandType newBandType) -> void {
    bandType = newBandType;
  }

  [[nodiscard]] auto Band::getEncodingModality() const -> EncodingModality {
    return encodingModality;
  }

  auto Band::setEncodingModality(EncodingModality newEncodingModality) -> void {
    encodingModality = newEncodingModality;
  }

  [[nodiscard]] auto Band::getWindowLength() const -> int {
    return windowLength;
  }

  auto Band::setWindowLength(int newWindowLength) -> void {
    windowLength = newWindowLength;
  }

  [[nodiscard]] auto Band::getUpperFrequencyLimit() const -> int {
    return upperFrequencyLimit;
  }

  auto Band::setUpperFrequencyLimit(int newUpperFrequencyLimit) -> void {
    upperFrequencyLimit = newUpperFrequencyLimit;
  }

  [[nodiscard]] auto Band::getLowerFrequencyLimit() const -> int {
    return lowerFrequencyLimit;
  }

  auto Band::setLowerFrequencyLimit(int newLowerFrequencyLimit) -> void {
    lowerFrequencyLimit = newLowerFrequencyLimit;
  }

  auto Band::getEffectsSize() -> size_t {
    return effects.size();
  }

  auto Band::getEffectAt(int index) -> haptics::types::Effect& {
    return effects.at(index);
  }

auto Band::addEffect(Effect &newEffect) -> void {
  auto it = std::find_if(effects.begin(), effects.end(), [newEffect](Effect &e) {
    return e.getPosition() > newEffect.getPosition();
  });

  effects.insert(it, newEffect);
  //effects.push_back(newEffect);
}

  [[nodiscard]] auto Band::isOverlapping(haptics::types::Effect &effect, const int start,
                                         const int stop) -> bool {
    const int position = effect.getPosition();
    int length = 0;
    if (encodingModality == EncodingModality::Quantized) {
      length = static_cast<int>(effect.getKeyframesSize()) * windowLength;
    } else {
      length = effect.getKeyframeAt(static_cast<int>(effect.getKeyframesSize()) - 1)
                   .getRelativePosition();
    }

    return (position <= start && position + length >= start) ||
           (position <= stop && position + length >= stop) ||
           (position >= start && position + length <= stop) ||
           (position <= start && position + length >= stop);
  }

  auto Band::Evaluate(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double {

    //OUT OUF BOUND CHECK
    if (effects.empty() || position >
        effects.back().getPosition() + effects.back()
                                           .getKeyframeAt(static_cast<int>(effects.back().getKeyframesSize()) - 1)
                                           .getRelativePosition() || position < 0) {
      return 0;
    }

    for (auto it = effects.end()-1; it >= effects.begin(); it--) {
      if (it->getPosition() <= position) {
        return EvaluationSwitch(position, &*it, lowFrequencyLimit, highFrequencyLimit);
      }
      if (it == effects.begin()) {
        break;
      }
    }

    return 0;
  }

  auto Band::EvaluationSwitch(double position, haptics::types::Effect *effect,
                              int lowFrequencyLimit, int highFrequencyLimit) -> double {

    switch (this->bandType) {
    case BandType::Curve:
      return effect->EvaluateKeyframes(position);
      break;
    case BandType::Wave:
      if (encodingModality == EncodingModality::Quantized) {
        return effect->EvaluateQuantized(position);
      } else if (encodingModality == EncodingModality::Vectorial) {
        return effect->EvaluateVectorial(position, lowFrequencyLimit, highFrequencyLimit);
      }
      break;
    case BandType::Transient:
      return effect->EvaluateTransient(position);
      break;
    default:
      break;
    }

    return -1;
  }
} // namespace haptics::types