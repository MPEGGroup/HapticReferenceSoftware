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
#include <cmath>

namespace haptics::types {

namespace {
auto clearKeyframes(types::Effect &effect) -> void {
  while (effect.getKeyframesSize() > 0) {
    effect.removeKeyframeAt(static_cast<int>(effect.getKeyframesSize()) - 1);
  }
}

auto hasContiguousCurveEffects(std::vector<types::Effect> &effects) -> bool {
  for (size_t effectIndex = 0; effectIndex + 1 < effects.size(); effectIndex++) {
    auto &effect = effects[effectIndex];
    auto &nextEffect = effects[effectIndex + 1];
    if (nextEffect.getPosition() ==
        effect.getPosition() + effect.getEffectTimeLength(types::BandType::Curve, 0)) {
      return true;
    }
  }
  return false;
}

auto hasContiguousNextCurveEffect(std::vector<types::Effect> &effects, size_t effectIndex) -> bool {
  if (effectIndex + 1 >= effects.size()) {
    return false;
  }

  auto &effect = effects[effectIndex];
  auto &nextEffect = effects[effectIndex + 1];
  return nextEffect.getPosition() ==
         effect.getPosition() + effect.getEffectTimeLength(types::BandType::Curve, 0);
}

auto addKeyframeAt(types::Effect &effect, int relativePosition, std::optional<float> amplitude,
                   std::optional<int> frequency) -> void {
  if (amplitude.has_value()) {
    effect.addAmplitudeAt(amplitude.value(), relativePosition);
  }
  if (frequency.has_value()) {
    effect.addFrequencyAt(frequency.value(), relativePosition);
  }
  if (!amplitude.has_value() && !frequency.has_value()) {
    effect.addKeyframe(relativePosition, std::nullopt, std::nullopt);
  }
}

auto interpolateAmplitudeAt(const std::vector<types::Keyframe> &keyframes, int position)
    -> std::optional<float> {
  bool hasPrev = false;
  bool hasNext = false;
  int prevPos = 0;
  int nextPos = 0;
  float prevVal = 0.0F;
  float nextVal = 0.0F;

  for (const auto &keyframe : keyframes) {
    const auto relPos = keyframe.getRelativePosition();
    const auto amp = keyframe.getAmplitudeModulation();
    if (!relPos.has_value() || !amp.has_value()) {
      continue;
    }

    if (relPos.value() == position) {
      return amp.value();
    }
    if (relPos.value() < position && (!hasPrev || relPos.value() > prevPos)) {
      hasPrev = true;
      prevPos = relPos.value();
      prevVal = amp.value();
    }
    if (relPos.value() > position && (!hasNext || relPos.value() < nextPos)) {
      hasNext = true;
      nextPos = relPos.value();
      nextVal = amp.value();
    }
  }

  if (hasPrev && hasNext && nextPos != prevPos) {
    const double ratio =
        static_cast<double>(position - prevPos) / static_cast<double>(nextPos - prevPos);
    return static_cast<float>(prevVal + ratio * (nextVal - prevVal));
  }
  if (hasPrev) {
    return prevVal;
  }
  if (hasNext) {
    return nextVal;
  }
  return std::nullopt;
}

auto interpolateFrequencyAt(const std::vector<types::Keyframe> &keyframes, int position)
    -> std::optional<int> {
  bool hasPrev = false;
  bool hasNext = false;
  int prevPos = 0;
  int nextPos = 0;
  int prevVal = 0;
  int nextVal = 0;

  for (const auto &keyframe : keyframes) {
    const auto relPos = keyframe.getRelativePosition();
    const auto freq = keyframe.getFrequencyModulation();
    if (!relPos.has_value() || !freq.has_value()) {
      continue;
    }

    if (relPos.value() == position) {
      return freq.value();
    }
    if (relPos.value() < position && (!hasPrev || relPos.value() > prevPos)) {
      hasPrev = true;
      prevPos = relPos.value();
      prevVal = freq.value();
    }
    if (relPos.value() > position && (!hasNext || relPos.value() < nextPos)) {
      hasNext = true;
      nextPos = relPos.value();
      nextVal = freq.value();
    }
  }

  if (hasPrev && hasNext && nextPos != prevPos) {
    const double ratio =
        static_cast<double>(position - prevPos) / static_cast<double>(nextPos - prevPos);
    return static_cast<int>(std::round(prevVal + ratio * (nextVal - prevVal)));
  }
  if (hasPrev) {
    return prevVal;
  }
  if (hasNext) {
    return nextVal;
  }
  return std::nullopt;
}
} // namespace

[[nodiscard]] auto Band::getBandType() const -> BandType { return bandType; }

auto Band::setBandType(BandType newBandType) -> void { bandType = newBandType; }
auto Band::getPriority() const -> std::optional<int> { return priority; }
auto Band::getPriorityOrDefault() const -> int {
  if (priority.has_value()) {
    return priority.value();
  }
  return 0;
}
auto Band::setPriority(int newPriority) -> void { priority = newPriority; }

[[nodiscard]] auto Band::getCurveTypeOrDefault() const -> CurveType {
  if (curveType.has_value()) {
    return curveType.value();
  }
  return DEFAULT_CURVE_TYPE;
}
[[nodiscard]] auto Band::getCurveType() const -> std::optional<CurveType> { return curveType; }

auto Band::setCurveType(CurveType newCurveType) -> void { curveType = newCurveType; }

[[nodiscard]] auto Band::getBlockLengthOrDefault() const -> int {
  if (blockLength.has_value()) {
    return blockLength.value();
  }
  return DEFAULT_BLOCK_LENGTH;
}

[[nodiscard]] auto Band::getBlockLength() const -> std::optional<int> { return blockLength; }

auto Band::setBlockLength(int newBlockLength) -> void { blockLength = newBlockLength; }

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

auto Band::Evaluate(double position, int lowFrequencyLimit, int highFrequencyLimit,
                    unsigned int timescale) -> double {
  // OUT OUF BOUND CHECK
  if (effects.empty() ||
      ((this->bandType != types::BandType::WaveletWave) &&
       (position > effects.back().getPosition() +
                       effects.back().getEffectTimeLength(bandType, TRANSIENT_DURATION_MS) ||
        position < 0))) {
    return 0;
  }

  if (!effects.empty()) {
    for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
      if (it->getPosition() <= position) {
        return EvaluationSwitch(position, &*it, lowFrequencyLimit, highFrequencyLimit, timescale);
      }
      if (it == effects.begin()) {
        break;
      }
    }
  }

  return 0;
}

auto Band::EvaluationSwitch(double position, haptics::types::Effect *effect, int lowFrequencyLimit,
                            int highFrequencyLimit, unsigned int timescale) -> double {

  switch (this->bandType) {
  case BandType::Curve:
    return effect->EvaluateKeyframes(position, this->getCurveTypeOrDefault(), timescale);
  case BandType::VectorialWave:
    return effect->EvaluateVectorial(position, lowFrequencyLimit, highFrequencyLimit, timescale);
  case BandType::WaveletWave:
    return effect->EvaluateWavelet(position, this->getUpperFrequencyLimit(), timescale);
  case BandType::Transient: {
    double res = 0;
    if (effect->getPosition() <= position &&
        position <= effect->getPosition() + effect->getEffectTimeLength(
                                                bandType, Band::getTransientDuration(timescale))) {
      res = effect->EvaluateTransient(position, Band::getTransientDuration(timescale));
    } // TODO: transform condition above to ticks?
    return res;
  }
  default:
    return 0;
  }
}

auto Band::EvaluationBand(uint32_t sampleCount, int fs, int pad, unsigned int timescale)
    -> std::vector<double> { // TODO: check impact of pad (which is in ms)
  std::vector<double> bandAmp(sampleCount, 0);
  switch (this->bandType) {
  case BandType::Curve:
    if (hasContiguousCurveEffects(effects)) {
      for (size_t effectIndex = 0; effectIndex < effects.size(); effectIndex++) {
        auto &e = effects[effectIndex];
        std::vector<std::pair<int, double>> keyframes(e.getKeyframesSize());
        for (int i = 0; i < static_cast<int>(e.getKeyframesSize()); i++) {
          types::Keyframe myKeyframe = e.getKeyframeAt(i);
          keyframes[i].first =
              static_cast<int>(myKeyframe.getRelativePosition().value() * fs / timescale);
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
        }

        int count = 0;
        int position = static_cast<int>((e.getPosition() + pad) * fs / timescale);
        if (position < 0) {
          count = -position;
          position = 0;
        }

        int lastCount = keyframes.back().first;
        if (hasContiguousNextCurveEffect(effects, effectIndex)) {
          lastCount--;
        }

        for (int i = position; (i < static_cast<int>(sampleCount)) && (count <= lastCount); i++) {
          bandAmp[i] += effectAmp[count];
          count++;
        }
      }
      break;
    }

    for (auto e : effects) {
      std::vector<std::pair<int, double>> keyframes(
          e.getKeyframesSize()); // keyframes converted to position relative to fs
      for (int i = 0; i < static_cast<int>(e.getKeyframesSize()); i++) {
        types::Keyframe myKeyframe;
        myKeyframe = e.getKeyframeAt(i);
        keyframes[i].first = static_cast<int>(myKeyframe.getRelativePosition().value() * fs /
                                              timescale); // assuming position in ticks as input
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
        int position =
            static_cast<int>((e.getPosition() + pad) * fs *
                             timescale); // position converted from ticks to samples rel. to fs
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
      double position = (double)timescale * (static_cast<double>(ti) / static_cast<double>(fs) -
                                             (pad * MS_2_S)); // position in ticks needed
      if (effects.empty() ||
          ((position > effects.back().getPosition() +
                           effects.back().getEffectTimeLength(
                               bandType, Band::getTransientDuration(timescale)) ||
            position < 0) &&
           (this->bandType != types::BandType::WaveletWave))) {
        bandAmp[ti] = 0;
      } // TODO: TRANSIENT_DURATION_MS: should it be transformed to ticks?

      if (!effects.empty()) {
        for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
          if (it->getPosition() <= position) {
            bandAmp[ti] += EvaluationSwitch(position, &*it, lowerFrequencyLimit,
                                            upperFrequencyLimit, timescale);
          }
          if (it == effects.begin()) {
            break;
          }
        }
      }
    }
    break;
  }
  return bandAmp;
}

auto Band::getBandTimeLength(unsigned int timescale) -> double {
  if (this->effects.empty()) {
    return 0;
  }
  return this->effects.back().getPosition() +
         this->effects.back().getEffectTimeLength(this->getBandType(),
                                                  Band::getTransientDuration(timescale));
}

auto Band::splitLongEffects(int maxEffectDuration) -> void {
  if (maxEffectDuration <= 0 || this->bandType == BandType::WaveletWave) {
    return;
  }

  std::vector<types::Effect> splitEffects;
  for (const auto &effectSource : effects) {
    auto effect = effectSource;
    std::vector<types::Keyframe> originalKeyframes;
    int maxRelPos = -1;
    for (int k = 0; k < static_cast<int>(effect.getKeyframesSize()); k++) {
      auto keyframe = effect.getKeyframeAt(k);
      originalKeyframes.push_back(keyframe);
      if (keyframe.getRelativePosition().has_value()) {
        maxRelPos = std::max(maxRelPos, keyframe.getRelativePosition().value());
      }
    }

    if (maxRelPos <= maxEffectDuration) {
      splitEffects.push_back(effect);
      continue;
    }

    const int effectStartPosition = effect.getPosition();
    for (int segmentStart = 0; segmentStart <= maxRelPos; segmentStart += maxEffectDuration) {
      const int segmentEnd = std::min(segmentStart + maxEffectDuration, maxRelPos);
      const bool isLastSegment = (segmentEnd == maxRelPos);
      types::Effect splitEffect(effect);
      splitEffect.setPosition(effectStartPosition + segmentStart);
      clearKeyframes(splitEffect);

      if (this->bandType == BandType::Curve || this->bandType == BandType::VectorialWave) {
        if (segmentStart > 0) {
          const auto ampAtStart = interpolateAmplitudeAt(originalKeyframes, segmentStart);
          const auto freqAtStart = this->bandType == BandType::VectorialWave
                                       ? interpolateFrequencyAt(originalKeyframes, segmentStart)
                                       : std::nullopt;
          if (ampAtStart.has_value() || freqAtStart.has_value()) {
            addKeyframeAt(splitEffect, 0, ampAtStart, freqAtStart);
          }
        }
        if (segmentEnd < maxRelPos) {
          const auto ampAtEnd = interpolateAmplitudeAt(originalKeyframes, segmentEnd);
          const auto freqAtEnd = this->bandType == BandType::VectorialWave
                                     ? interpolateFrequencyAt(originalKeyframes, segmentEnd)
                                     : std::nullopt;
          if (ampAtEnd.has_value() || freqAtEnd.has_value()) {
            addKeyframeAt(splitEffect, segmentEnd - segmentStart, ampAtEnd, freqAtEnd);
          }
        }
      }

      for (const auto &keyframe : originalKeyframes) {
        if (!keyframe.getRelativePosition().has_value()) {
          continue;
        }
        const int relPos = keyframe.getRelativePosition().value();
        if (relPos < segmentStart || relPos > segmentEnd ||
            (!isLastSegment && relPos == segmentEnd)) {
          continue;
        }

        addKeyframeAt(splitEffect, relPos - segmentStart, keyframe.getAmplitudeModulation(),
                      keyframe.getFrequencyModulation());
      }

      if (splitEffect.getKeyframesSize() > 0) {
        splitEffects.push_back(splitEffect);
      }
    }
  }

  effects = splitEffects;
}

//[[nodiscard]] auto Band::getTimescale() const -> int { return this->timescale; }

// auto Band::setTimescale(int newTimescale) -> void { timescale = newTimescale; }

[[nodiscard]] auto Band::getTransientDuration(unsigned int timescale) -> double {
  return TRANSIENT_DURATION_MS / static_cast<double>((double)TIMESCALE / timescale);
}

auto Band::equals(const Band &band) const -> bool {
  if (bandType != band.getBandType()) {
    std::cerr << "bandType fields are different" << std::endl;
    return false;
  }
  if (curveType != band.getCurveType()) {
    std::cerr << "curveType fields are different" << std::endl;
    return false;
  }
  if (blockLength != band.getBlockLength()) {
    std::cerr << "blockLength fields are different" << std::endl;
    return false;
  }
  if (lowerFrequencyLimit != band.getLowerFrequencyLimit()) {
    std::cerr << "lowerFrequencyLimit fields are different" << std::endl;
    return false;
  }
  if (upperFrequencyLimit != band.getUpperFrequencyLimit()) {
    std::cerr << "upperFrequencyLimit fields are different" << std::endl;
    return false;
  }
  if (priority != band.getPriority()) {
    std::cerr << "priority fields are different" << std::endl;
    return false;
  }
  if (effects.size() != band.effects.size()) {
    std::cerr << "Number of effects are different" << std::endl;
    return false;
  }
  bool isEqual = true;
  for (int i = 0; i < static_cast<int>(effects.size()); i++) {
    const auto effect1 = effects.at(i);
    const auto effect2 = band.effects.at(i);
    isEqual = isEqual && (effect1.equals(effect2));
  }

  return isEqual;
}

} // namespace haptics::types
