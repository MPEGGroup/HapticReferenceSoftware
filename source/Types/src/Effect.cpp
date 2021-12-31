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

#include <Types/include/Effect.h>
#include <Tools/include/Tools.h>
#include <cmath>
#include <algorithm>
#include <functional>

namespace haptics::types {

  [[nodiscard]] auto Effect::getPosition() const -> int {
    return position;
  }

  auto Effect::setPosition(int newPosition) -> void {
    position = newPosition;
  }

  [[nodiscard]] auto Effect::getPhase() const -> float {
    return phase;
  }

  auto Effect::setPhase(float newPhase) -> void {
    phase = newPhase;
  }

  [[nodiscard]] auto Effect::getBaseSignal() const -> BaseSignal {
    return baseSignal;
  }

  auto Effect::setBaseSignal(BaseSignal newBaseSignal) -> void {
    baseSignal = newBaseSignal;
  }

  auto Effect::getKeyframesSize() -> size_t {
    return keyframes.size();
  }

  auto Effect::getKeyframeAt(int index) -> haptics::types::Keyframe& {
    return keyframes.at(index);
  }

  auto Effect::addKeyframe(haptics::types::Keyframe& newKeyframe) -> void {
    keyframes.push_back(newKeyframe);
  }

  auto Effect::addAmplitudeAt(float amplitude, int position) -> bool {

    bool ret = false;

    auto kit =
        std::find_if(keyframes.begin(), keyframes.end(), [position](haptics::types::Keyframe k) {
          return k.getRelativePosition() >= position;
        });

    if (kit == keyframes.begin()) {
      return ret;
    }

    if (kit == keyframes.end()) {
      Keyframe kf = Keyframe(position, amplitude, std::optional<int>());
      keyframes.push_back(kf);
      return true;
    }

    if ((kit)->getRelativePosition() == position && !(kit)->getAmplitudeModulation().has_value()) {
      (kit)->setAmplitudeModulation(amplitude);
    } else {
      Keyframe kf = Keyframe(position, amplitude, std::optional<int>());
      keyframes.insert(kit+1, kf);
    }

    return true;
  }

  auto Effect::addFrequencyAt(int frequency, int position) -> bool {
    auto kit =
        std::find_if(keyframes.begin(), keyframes.end(), [position](haptics::types::Keyframe k) {
          return k.getRelativePosition() >= position;
        });

    if (kit == keyframes.begin()) {
      return false;
    }

    if (kit == keyframes.end()) {
      Keyframe kf = Keyframe(position, std::optional<float>(), frequency);
      keyframes.push_back(kf);
      return true;
    }

    if (kit->getRelativePosition() == position && !kit->getFrequencyModulation().has_value()) {
      kit->setFrequencyModulation(frequency);
    } else {
      Keyframe kf = Keyframe(position, std::optional<float>(), frequency);
      keyframes.insert(kit + 1, kf);
    }

    return true;
  }

  auto Effect::addKeyframe(int position, std::optional<double> amplitudeModulation, std::optional<int> frequencyModulation) -> void {
    this->addKeyframe(*(new Keyframe(position, amplitudeModulation, frequencyModulation)));
  }

  auto Effect::EvaluateVectorial(double position, int lowFrequencyLimit, int highFrequencyLimit)
      -> double {
    double res = 0;

    if (position < this->position || position > this->position + this->keyframes.back().getRelativePosition()) {
      return res;
    }

    if (keyframes.empty()) {
      return res;
    }

    double relativePosition = position - this->position;

    // IF AMPLITUDE MODULATION (SHOULD BE BUT TO BE SURE)

    double amp_modulation = 0;
    //First KF AFTER
    auto k_a_after = std::find_if(keyframes.begin(), keyframes.end(),
                                      [relativePosition](haptics::types::Keyframe k) {
                                        return k.getAmplitudeModulation().has_value() && k.getRelativePosition() > relativePosition;
                                      });
    if (k_a_after < keyframes.end()) {
      // first KF before position
      auto k_a_before = keyframes.begin();
      for (auto it = k_a_after; it >= keyframes.begin(); it--) {
        if (it->getRelativePosition() <= relativePosition && it->getAmplitudeModulation().has_value()) {
          k_a_before = it;
          break;
        }
      }

      amp_modulation = haptics::tools::linearInterpolation(
          {k_a_before->getRelativePosition(), k_a_before->getAmplitudeModulation().value()},
          {k_a_after->getRelativePosition(), k_a_after->getAmplitudeModulation().value()}, relativePosition);
    } else {
      amp_modulation = (keyframes.end() - 1)->getAmplitudeModulation().value();
    }

    // IF FREQUENCY MODULATION (SHOULD BE BUT TO BE SURE)

    double freq_modulation = 1;
    // first KF after position
    auto k_f_after = std::find_if(keyframes.begin(), keyframes.end(),
                                  [relativePosition](haptics::types::Keyframe k) {
                                    return k.getFrequencyModulation().has_value() &&
                                           k.getRelativePosition() > relativePosition;
                                  });
    if (k_f_after < keyframes.end()) {
      // first KF before position
      auto k_f_before = keyframes.begin();
      for (auto it = k_f_after; it >= keyframes.begin(); it--) {
        if (it->getRelativePosition() <= relativePosition && it->getFrequencyModulation().has_value()) {
          k_f_before = it;
          break;
        }
      }

      // Modulation
      double f0 = std::max(k_f_before->getFrequencyModulation().value(),0);
      double f1 = std::max(k_f_after->getFrequencyModulation().value(),0);
      double t = MS_2_S * relativePosition;
      double DeltaT = MS_2_S * (static_cast<double>(k_f_after->getRelativePosition()) -
                             static_cast<double>(k_f_before->getRelativePosition()));

      freq_modulation =
          std::clamp(f0 + t * (f1 - f0) / DeltaT, static_cast<double>(lowFrequencyLimit),
                     static_cast<double>(highFrequencyLimit)) +
          f0;

      double phase = this->getPhase();
      switch (this->getBaseSignal()) {
      case BaseSignal::Sine:
        freq_modulation = std::sin(M_PI * t * freq_modulation + phase);
        break;
      case BaseSignal::Square:
        freq_modulation = std::copysign(1, std::sin(M_PI * t * freq_modulation + phase));
        break;
      case BaseSignal::Triangle:
        t += phase / (2 * M_PI * freq_modulation);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        freq_modulation = 1 - 4 * std::abs(std::round(t * freq_modulation - .25) - (t * freq_modulation - .25));
        break;
      case BaseSignal::SawToothUp:
        t += phase / (2 * M_PI * freq_modulation);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        freq_modulation = 2 * (t * freq_modulation - std::floor(t * freq_modulation + .5));
        break;
      case BaseSignal::SawToothDown:
        t += phase / (2 * M_PI * freq_modulation);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        freq_modulation = 2 * (std::floor(t * freq_modulation + .5) - t * freq_modulation);
        break;
      default:
        freq_modulation = 1;
        break;
      }
    }

    return amp_modulation * freq_modulation;
  }

  auto Effect::EvaluateQuantized(double position, double windowLength) -> double {
    double relativePosition = position - this->getPosition();
    int index = std::floor(relativePosition / windowLength);
    if (index >= this->getKeyframesSize()) {
      return 0;
    }

    auto myKeyframe = keyframes.begin() + index;
    if (!myKeyframe->getAmplitudeModulation().has_value() ||
        !myKeyframe->getFrequencyModulation().has_value()) {
      return 0;
    }

    double t = MS_2_S * relativePosition;
    return std::sin(t * myKeyframe->getFrequencyModulation().value() * 2 * M_PI + this->getPhase()) *
           myKeyframe->getAmplitudeModulation().value();
  }

  auto Effect::EvaluateTransient(double position, double transientDuration) -> double {
    const double relativePosition = position - this->getPosition();

    auto checkingFunction = [&](Keyframe kf) {
      return kf.getRelativePosition() <= relativePosition &&
             kf.getRelativePosition() + transientDuration >= relativePosition;
    };
    auto it = std::find_if(keyframes.rbegin(), keyframes.rend(), checkingFunction);

    double res = 0;
    while (it != keyframes.rend() && checkingFunction(*it)) {
      if (!it->getAmplitudeModulation().has_value()) {
        continue;
      }

      res +=
          // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
          std::sin(4 * M_PI * (relativePosition - it->getRelativePosition()) / transientDuration) *
          it->getAmplitudeModulation().value();
      it++;
    }

    return res;
  }

  auto Effect::EvaluateKeyframes(double position) -> double {
    double res = 0;

    auto k_after = std::find_if(keyframes.begin(), keyframes.end(),
                                  [position](haptics::types::Keyframe k) {
                                    return k.getAmplitudeModulation().has_value() &&
                                           k.getRelativePosition() > position;
                                  });

    if (k_after < keyframes.end()) {
      // first KF before position
      auto k_before = keyframes.begin();
      for (auto it = k_after - 1; it >= keyframes.begin(); it--) {
        if (it->getAmplitudeModulation().has_value()) {
          k_before = it;
          break;
        }
      }

      double x0 = MS_2_S * k_before->getRelativePosition();
      double f0 = k_before->getAmplitudeModulation().value();
      double t0 = 0;
      double x1 = MS_2_S * k_after->getRelativePosition();
      double f1 = k_after->getAmplitudeModulation().value();
      double t1 = 0;

      double t = MS_2_S * position;

      double c2 = 0;
      double c3 = 0;
      double df = 0;
      double h = 0;
      h = x1 - x0;
      df = (f1 - f0) / h;

      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
      c2 = -(2.0 * t0 - 3.0 * df + t1) / h;
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
      c3 = (t0 - 2.0 * df + t1) / h / h;

      auto result = static_cast<float>(f0 + (t - x0) * (t0 + (t - x0) * (c2 + (t - x0) * c3)));
      return std::max(std::min(1.0F, result), -1.0F);
    }

    return res;
  }

} // namespace haptics::types