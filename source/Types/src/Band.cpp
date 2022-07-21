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

#include <Tools/include/Tools.h>
#include <Types/include/Band.h>
#include <algorithm>

using haptics::tools::akimaInterpolation;
using haptics::tools::bezierInterpolation;
using haptics::tools::bsplineInterpolation;
using haptics::tools::cubicInterpolation;
using haptics::tools::cubicInterpolation2;
using haptics::tools::linearInterpolation2;

namespace haptics::types {

[[nodiscard]] auto Band::getBandType() const -> BandType { return bandType; }

auto Band::setBandType(BandType newBandType) -> void { bandType = newBandType; }

[[nodiscard]] auto Band::getCurveType() const -> CurveType { return curveType; }

auto Band::setCurveType(CurveType newCurveType) -> void { curveType = newCurveType; }

[[nodiscard]] auto Band::getWindowLength() const -> int { return windowLength; }

auto Band::setWindowLength(int newWindowLength) -> void { windowLength = newWindowLength; }

[[nodiscard]] auto Band::getUpperFrequencyLimit() const -> int { return upperFrequencyLimit; }

auto Band::setUpperFrequencyLimit(int newUpperFrequencyLimit) -> void {
  upperFrequencyLimit = newUpperFrequencyLimit;
}

[[nodiscard]] auto Band::getLowerFrequencyLimit() const -> int { return lowerFrequencyLimit; }

auto Band::setLowerFrequencyLimit(int newLowerFrequencyLimit) -> void {
  lowerFrequencyLimit = newLowerFrequencyLimit;
}

auto Band::getEffectsSize() -> size_t { return effects.size(); }

auto Band::getEffectAt(int index) -> haptics::types::Effect & { return effects.at(index); }

auto Band::addEffect(Effect &newEffect) -> void {
  auto it = std::find_if(effects.begin(), effects.end(), [newEffect](Effect &e) {
    return e.getPosition() > newEffect.getPosition();
  });

  effects.insert(it, newEffect);
}

auto Band::replaceEffectAt(int index, haptics::types::Effect &newEffect) -> bool {
  if (index < 0 || index >= (int)this->getEffectsSize()) {
    return false;
  }
  this->effects[index] = newEffect;
  return true;
}

[[nodiscard]] auto Band::isOverlapping(haptics::types::Effect &effect, const int start,
                                       const int stop) -> bool {
  const int position = effect.getPosition();
  double length = effect.getEffectTimeLength(bandType, TRANSIENT_DURATION_MS);

  return (position <= start && position + length >= start) ||
         (position <= stop && position + length >= stop) ||
         (position >= start && position + length <= stop) ||
         (position <= start && position + length >= stop);
}

auto Band::Evaluate(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double {
  // OUT OUF BOUND CHECK
  if (effects.empty() ||
      ((this->bandType != types::BandType::WaveletWave) &&
       (position > effects.back().getPosition() +
                       effects.back().getEffectTimeLength(bandType, TRANSIENT_DURATION_MS) ||
        position < 0))) {
    return 0;
  }

  for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
    if (it->getPosition() <= position) {
      return EvaluationSwitch(position, &*it, lowFrequencyLimit, highFrequencyLimit);
    }
    if (it == effects.begin()) {
      break;
    }
  }

  return 0;
}

auto Band::EvaluationSwitch(double position, haptics::types::Effect *effect, int lowFrequencyLimit,
                            int highFrequencyLimit) -> double {

  switch (this->bandType) {
  case BandType::Curve:
    return effect->EvaluateKeyframes(position, this->getCurveType());
  case BandType::VectorialWave:
    return effect->EvaluateVectorial(position, lowFrequencyLimit, highFrequencyLimit);
  case BandType::WaveletWave:
    return effect->EvaluateWavelet(position, this->getWindowLength());
  case BandType::Transient: {
    double res = 0;
    if (effect->getPosition() <= position &&
        position <= effect->getPosition() + TRANSIENT_DURATION_MS) {
      res = effect->EvaluateTransient(position, TRANSIENT_DURATION_MS);
    }
    return res;
  }
  default:
    return 0;
  }
}

auto Band::EvaluationBand(uint32_t sampleCount, int fs, int pad, int lowFrequencyLimit,
                          int highFrequencyLimit) -> std::vector<double> {
  std::vector<double> bandAmp(sampleCount, 0);
  switch (this->bandType) {
  case BandType::Curve:
    for (auto e : effects) {
      std::vector<std::pair<int, double>> keyframes(e.getKeyframesSize());
      for (int i = 0; i < static_cast<int>(e.getKeyframesSize()); i++) {
        types::Keyframe myKeyframe;
        myKeyframe = e.getKeyframeAt(i);
        keyframes[i].first =
            static_cast<int>(myKeyframe.getRelativePosition().value() * fs * MS_2_S);
        keyframes[i].first -= keyframes[0].first;
        keyframes[i].second = myKeyframe.getAmplitudeModulation().value();
      }
      std::vector<double> effectAmp(keyframes.back().first - keyframes[0].first + 1, 0);
      if (keyframes.size() == 2) {
        effectAmp = linearInterpolation2(keyframes);
      } else {
        switch (this->curveType) {
        case CurveType::Linear:
          effectAmp = linearInterpolation2(keyframes);
          break;
        case CurveType::Cubic:
          // effectAmp = cubicInterpolation(keyframes);
          effectAmp = cubicInterpolation2(keyframes);
          break;
        case CurveType::Akima:
          effectAmp = akimaInterpolation(keyframes);
          break;
        case CurveType::Bezier:
          effectAmp = bezierInterpolation(keyframes);
          break;
        case CurveType::Bspline:
          effectAmp = bsplineInterpolation(keyframes);
          break;
        default:
          break;
        }

        int count = 0;
        int position = static_cast<int>((e.getPosition() + keyframes[0].first) * fs * MS_2_S);
        for (int i = position; i < position + static_cast<int>(effectAmp.size()); i++) {
          bandAmp[i] += effectAmp[count];
          count++;
        }
      }
    }
    break;
  default:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      double position = S_2_MS * static_cast<double>(ti) / static_cast<double>(fs) - pad;
      if (effects.empty() ||
          ((position > effects.back().getPosition() +
                           effects.back().getEffectTimeLength(bandType,
                                                              TRANSIENT_DURATION_MS) ||
            position < 0) &&
           (this->bandType != types::BandType::WaveletWave))) {
        bandAmp[ti] = 0;
      }

      for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
        if (it->getPosition() <= position) {
          bandAmp[ti] += EvaluationSwitch(position, &*it, lowFrequencyLimit, highFrequencyLimit);
        }
        if (it == effects.begin()) {
          break;
        }
      }
    }
    break;
  }
  return bandAmp;
}

auto Band::getBandTimeLength() -> double {
  if (this->effects.empty()) {
    return 0;
  }
  return this->effects.back().getPosition() +
         this->effects.back().getEffectTimeLength(this->getBandType(), TRANSIENT_DURATION_MS);
}
} // namespace haptics::types
