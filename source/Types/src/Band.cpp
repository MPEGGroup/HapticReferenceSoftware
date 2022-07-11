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
#include <algorithm>
#include <Tools/include/Tools.h>

using haptics::tools::akimaInterpolation;

namespace haptics::types {

[[nodiscard]] auto Band::getBandType() const -> BandType { return bandType; }

auto Band::setBandType(BandType newBandType) -> void { bandType = newBandType; }

[[nodiscard]] auto Band::getCurveType() const -> CurveType { return curveType; }

auto Band::setCurveType(CurveType newCurveType) -> void { curveType = newCurveType; }

[[nodiscard]] auto Band::getEncodingModality() const -> EncodingModality {
  return encodingModality;
}

auto Band::setEncodingModality(EncodingModality newEncodingModality) -> void {
  encodingModality = newEncodingModality;
}

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
  double length = effect.getEffectTimeLength(bandType, encodingModality, TRANSIENT_DURATION_MS);

  return (position <= start && position + length >= start) ||
         (position <= stop && position + length >= stop) ||
         (position >= start && position + length <= stop) ||
         (position <= start && position + length >= stop);
}

auto Band::Evaluate(double position, int lowFrequencyLimit, int highFrequencyLimit) -> double {
  // OUT OUF BOUND CHECK
  if (effects.empty() || ((position > effects.back().getPosition() +
                                          effects.back().getEffectTimeLength(
                                              bandType, encodingModality, TRANSIENT_DURATION_MS) ||
                           position < 0) &&
                          (this->encodingModality != types::EncodingModality::Wavelet))) {
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
    return 0;
  case BandType::Wave:
    if (encodingModality == EncodingModality::Vectorial) {
      return effect->EvaluateVectorial(position, lowFrequencyLimit, highFrequencyLimit);
    } else if (encodingModality == EncodingModality::Wavelet) {
      auto sample = effect->EvaluateWavelet(position, this->getWindowLength());
      return sample;
    }
    break;
  case BandType::Transient: {
    double res = 0;
    for (Effect e : effects) {
      if (e.getPosition() <= position && position <= e.getPosition() + TRANSIENT_DURATION_MS) {
        res += e.EvaluateTransient(position, TRANSIENT_DURATION_MS);
      }
    }
    return res;
  }
  default:
    return 0;
  }

  return -1;
}

auto Band::EvaluationBand(uint32_t sampleCount, const int fs, const int pad, int lowFrequencyLimit,
                          int highFrequencyLimit) -> std::vector<double> {
  std::vector<double> bandAmp(sampleCount);
  switch (this->bandType) {
  case BandType::Curve:
    for (haptics::types::Effect e : effects) {
      std::vector<std::pair<int, double>> keyframes(e.getKeyframesSize());
      for (uint32_t i = 0; i < e.getKeyframesSize(); i++) {
        types::Keyframe myKeyframe;
        myKeyframe = e.getKeyframeAt(i);
        keyframes[i].first = myKeyframe.getRelativePosition().value() * fs / 1000;
        keyframes[i].second =  myKeyframe.getAmplitudeModulation().value();
      }
      bandAmp = akimaInterpolation(keyframes);
      return bandAmp; // On considère n'avoir qu'un seul effet pour le type curve, à modifier pour les cas où on aura plusieurs effets
    }
  default:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      double position = S_2_MS * static_cast<double>(ti) / static_cast<double>(fs) - pad;
      if (effects.empty() ||
          ((position > effects.back().getPosition() +
                           effects.back().getEffectTimeLength(bandType, encodingModality,
                                                              TRANSIENT_DURATION_MS) ||
            position < 0) &&
           (this->encodingModality != types::EncodingModality::Wavelet))) {
        bandAmp[ti] = 0;
      }

      for (auto it = effects.end() - 1; it >= effects.begin(); it--) {
        if (it->getPosition() <= position) {
          bandAmp[ti] = EvaluationSwitch(position, &*it, lowFrequencyLimit, highFrequencyLimit);
        }
        if (it == effects.begin()) {
          break;
        }
      }
    }
    return bandAmp;
  }
}

/*
auto Band::EvaluationBand(uint32_t sampleCount, const int fs, const int pad, int lowFrequencyLimit, int highFrequencyLimit) -> std::vector<double> {
  std::vector<double> bandAmp(sampleCount);
  switch (this->bandType) {
  case BandType::Curve:
    for (Effect e : effects) {
      std::vector<std::pair<int, double>> keyframes(e.getKeyframesSize());
      for (uint32_t i = 0; i < e.getKeyframesSize(); i++) {
        keyframes[i] = e.getKeyframeAt(i);
      }
      bandAmp = akimaInterpolation(keyframes);
      return bandAmp;
    }
  case BandType::Wave:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      for (Effect e : effects) {
        double t = S_2_MS * static_cast<double>(ti) / static_cast<double>(fs) - pad;
        double sample = 0;
        if (encodingModality == EncodingModality::Vectorial) {
          sample =  e.EvaluateVectorial(t, lowFrequencyLimit, highFrequencyLimit);
        }
        else if (encodingModality == EncodingModality::Wavelet) {
          sample = e.EvaluateWavelet(t, this->getWindowLength());
        }
        bandAmp[ti] = sample;
      }
      return bandAmp;
    }
  case BandType::Transient:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      double t = S_2_MS * static_cast<double>(ti) / static_cast<double>(fs) - pad;
      double res = 0;
      for (Effect e : effects) {
        if (e.getPosition() <= t && t <= e.getPosition() + TRANSIENT_DURATION_MS) {
          res += e.EvaluateTransient(t, TRANSIENT_DURATION_MS);
        }
      }
      bandAmp[ti] = res;
    }
    return bandAmp;
  default:
    for (uint32_t ti = 0; ti < sampleCount; ti++) {
      bandAmp[ti] = 0;
    }
    return (bandAmp);
  }
}
*/

auto Band::getBandTimeLength() -> double {
  if (this->effects.empty()) {
    return 0;
  }
  return this->effects.back().getPosition() +
         this->effects.back().getEffectTimeLength(this->getBandType(), this->getEncodingModality(),
                                                  TRANSIENT_DURATION_MS);
}
} // namespace haptics::types