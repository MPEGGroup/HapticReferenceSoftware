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

namespace haptics::types {

[[nodiscard]] auto Band::getBandType() const -> BandType { return bandType; }

auto Band::setBandType(BandType newBandType) -> void { bandType = newBandType; }

[[nodiscard]] auto Band::getCurveTypeOrDefault() const -> CurveType {
  if (curveType.has_value()) {
    return curveType.value();
  }
    return DEFAULT_CURVE_TYPE;
}
[[nodiscard]] auto Band::getCurveType() const -> std::optional<CurveType> { return curveType; }

auto Band::setCurveType(CurveType newCurveType) -> void { curveType = newCurveType; }

[[nodiscard]] auto Band::getBlockLength() const -> double { return blockLength; }

auto Band::setBlockLength(double newBlockLength) -> void { blockLength = newBlockLength; }

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

auto Band::removeEffectAt(int index) -> bool {
  if (index < 0 || index >= (int)this->getEffectsSize()) {
    return false;
  }
  this->effects.erase(this->effects.begin() + index);
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
    return effect->EvaluateKeyframes(position, this->getCurveTypeOrDefault());
  case BandType::VectorialWave:
    return effect->EvaluateVectorial(position, lowFrequencyLimit, highFrequencyLimit);
  case BandType::WaveletWave:
    return effect->EvaluateWavelet(position, this->getUpperFrequencyLimit(), this->timescale);
  case BandType::Transient: {
    double res = 0;
    if (effect->getPosition() <= position &&
        position <=
            effect->getPosition() + effect->getEffectTimeLength(bandType, TRANSIENT_DURATION_MS)) {
      res = effect->EvaluateTransient(position, TRANSIENT_DURATION_MS);
    } // TODO: transform condition above to ticks?
    return res;
  }
  default:
    return 0;
  }
}

auto Band::EvaluationBand(uint32_t sampleCount, int fs, int pad)
    -> std::vector<double> { // TODO: check impact of pad (which is in ms)
  std::vector<double> bandAmp(sampleCount, 0);
  switch (this->bandType) {
  case BandType::Curve:
    for (auto e : effects) {
      std::vector<std::pair<int, double>> keyframes(
          e.getKeyframesSize()); // keyframes converted to position relative to fs
      for (int i = 0; i < static_cast<int>(e.getKeyframesSize()); i++) {
        types::Keyframe myKeyframe;
        myKeyframe = e.getKeyframeAt(i);
        keyframes[i].first =
            static_cast<int>(myKeyframe.getRelativePosition().value() * fs /
                             this->timescale); // assuming position in ticks as input
        if (i > 0) {
          keyframes[i].first -= keyframes[0].first;
        }
        keyframes[i].second = myKeyframe.getAmplitudeModulation().value();
      }
      keyframes[0].first = 0;
      std::vector<double> effectAmp(static_cast<::std::size_t>(keyframes.back().first) + 1, 0);
      if (keyframes.size() == 2) {
        effectAmp = haptics::tools::linearInterpolation2(keyframes);
      } else {
        switch (getCurveTypeOrDefault()) {
        case CurveType::Linear:
          effectAmp = haptics::tools::linearInterpolation2(keyframes);
          break;
        case CurveType::Cubic:
          effectAmp = haptics::tools::cubicInterpolation2(keyframes);
          break;
        case CurveType::Akima:
          effectAmp = haptics::tools::akimaInterpolation(keyframes);
          break;
        case CurveType::Bezier:
          effectAmp = haptics::tools::bezierInterpolation(keyframes);
          break;
        case CurveType::Bspline:
          effectAmp = haptics::tools::bsplineInterpolation(keyframes);
          break;
        default:
          effectAmp = haptics::tools::cubicInterpolation2(keyframes);
          break;
        }

        int count = 0;
        int position = static_cast<int>(
            (e.getPosition() + pad) * fs *
            this->timescale); // position converted from ticks to samples rel. to fs
        if (position < 0) {
          count = -position;
          position = 0;
        }
        for (int i = position;
             (i < static_cast<int>(sampleCount)) && (count <= keyframes.back().first); i++) {
          bandAmp[i] += effectAmp[count];
          count++;
        }
      }
    }
    break;
  default:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      double position = this->timescale * (static_cast<double>(ti) / static_cast<double>(fs) -
                                           (pad * MS_2_S)); // position in ticks needed
      if (effects.empty() ||
          ((position > effects.back().getPosition() +
                           effects.back().getEffectTimeLength(bandType, TRANSIENT_DURATION_MS) ||
            position < 0) &&
           (this->bandType != types::BandType::WaveletWave))) {
        bandAmp[ti] = 0;
      } // TODO: TRANSIENT_DURATION_MS: should it be transformed to ticks?

      for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
        if (it->getPosition() <= position) {
          bandAmp[ti] += EvaluationSwitch(position, &*it, lowerFrequencyLimit, upperFrequencyLimit);
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

[[nodiscard]] auto Band::getTimescale() const -> int { return this->timescale; }

auto Band::setTimescale(int newTimescale) -> void { timescale = newTimescale; }

} // namespace haptics::types
