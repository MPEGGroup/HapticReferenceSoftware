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

#include <IOHaptics/include/IOBinaryBands.h>
#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <limits>

namespace haptics::io {

auto IOBinaryBands::readBandHeader(types::Band &band, std::ifstream &file) -> bool {
  auto bandType = IOBinaryPrimitives::readNBytes<uint8_t, 1>(file);
  band.setBandType(static_cast<types::BandType>(bandType));
  if (band.getBandType() == types::BandType::Curve) {
    auto curveType = IOBinaryPrimitives::readNBytes<uint8_t, 1>(file);
    band.setCurveType(static_cast<types::CurveType>(curveType));
  } else if (band.getBandType() == types::BandType::WaveletWave) {
    auto windowLength = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
    band.setWindowLength(static_cast<int>(windowLength));
  }

  auto lowerFrequencyLimit = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  band.setLowerFrequencyLimit(static_cast<int>(lowerFrequencyLimit));

  auto upperFrequencyLimit = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  band.setUpperFrequencyLimit(static_cast<int>(upperFrequencyLimit));

  auto effectCount = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  for (unsigned int i = 0; i < effectCount; i++) {
    types::Effect e;
    band.addEffect(e);
  }

  return true;
}

auto IOBinaryBands::writeBandHeader(types::Band &band, std::ofstream &file) -> bool {
  types::BandType t = band.getBandType();
  auto bandType = static_cast<unsigned short>(t);
  IOBinaryPrimitives::writeNBytes<uint8_t, 1>(bandType, file);

  if (band.getBandType() == types::BandType::Curve) {
    auto curveType = static_cast<uint8_t>(band.getCurveType());
    IOBinaryPrimitives::writeNBytes<uint8_t, 1>(curveType, file);
  } else if (band.getBandType() == types::BandType::WaveletWave) {
    auto windowLength = static_cast<uint16_t>(band.getWindowLength());
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(windowLength, file);
  }

  auto lowerFrequencyLimit = static_cast<unsigned int>(band.getLowerFrequencyLimit());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(lowerFrequencyLimit, file);

  auto upperFrequencyLimit = static_cast<unsigned int>(band.getUpperFrequencyLimit());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(upperFrequencyLimit, file);

  auto effectCount = static_cast<unsigned int>(band.getEffectsSize());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(effectCount, file);

  return true;
}

auto IOBinaryBands::readBandBody(types::Band &band, std::ifstream &file) -> bool {
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    auto myEffect = band.getEffectAt(effectIndex);
    auto effectType =
        static_cast<types::EffectType>(IOBinaryPrimitives::readNBytes<uint8_t, 1>(file));
    myEffect.setEffectType(effectType);
    auto position = 0;
    if ((myEffect.getEffectType() == types::EffectType::Basis &&
         band.getBandType() == types::BandType::WaveletWave)) {
      position = effectIndex * band.getWindowLength();
    } else {
      position = static_cast<int>(IOBinaryPrimitives::readNBytes<uint32_t, 4>(file));
    }
    myEffect.setPosition(position);
    if (effectType == types::EffectType::Reference) {
      readReferenceEffect(myEffect, file);
    } else if (effectType == types::EffectType::Timeline) {
      readTimelineEffect(myEffect, band, file);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        if (!IOBinaryBands::readTransientEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::Curve:
        if (!IOBinaryBands::readCurveEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::VectorialWave:
        if (!IOBinaryBands::readVectorialEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::WaveletWave:
        if (!IOBinaryBands::readWaveletEffect(myEffect, band, file)) {
          return false;
        }
        break;
      default:
        return false;
      }
    }
    band.replaceEffectAt(effectIndex, myEffect);
  }
  return true;
}

auto IOBinaryBands::writeBandBody(types::Band &band, std::ofstream &file) -> bool {
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    auto myEffect = band.getEffectAt(effectIndex);
    auto effectType = static_cast<uint8_t>(myEffect.getEffectType());
    IOBinaryPrimitives::writeNBytes<uint8_t, 1>(effectType, file);
    if ((myEffect.getEffectType() != types::EffectType::Basis ||
         band.getBandType() != types::BandType::WaveletWave)) {
      auto position = static_cast<uint32_t>(myEffect.getPosition());
      IOBinaryPrimitives::writeNBytes<uint32_t, 4>(position, file);
    }
    if (myEffect.getEffectType() == types::EffectType::Reference) {
      writeReferenceEffect(myEffect, file);
    } else if (myEffect.getEffectType() == types::EffectType::Timeline) {
      writeTimelineEffect(myEffect, band, file);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        if (!IOBinaryBands::writeTransientEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::Curve:
        if (!IOBinaryBands::writeCurveEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::VectorialWave:
        if (!IOBinaryBands::writeVectorialEffect(myEffect, file)) {
          return false;
        }
        break;
      case types::BandType::WaveletWave:
        if (!IOBinaryBands::writeWaveletEffect(myEffect, file)) {
          return false;
        }
        break;
      default:
        return true;
      }
    }
  }
  return true;
}

auto IOBinaryBands::readTransientEffect(types::Effect &effect, std::ifstream &file) -> bool {

  auto keyframeCount = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    float amplitude =
        IOBinaryPrimitives::readFloatNBytes<uint8_t, 1>(file, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    auto position = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
    auto frequency = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);

    auto myKeyframe = types::Keyframe(position, amplitude, frequency);
    effect.addKeyframe(myKeyframe);
  }

  return true;
}

auto IOBinaryBands::writeTransientEffect(types::Effect &effect, std::ofstream &file) -> bool {
  types::Keyframe myKeyframe;
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(keyframeCount, file);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    myKeyframe = effect.getKeyframeAt(kfIndex);
    float amplitude = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      amplitude = myKeyframe.getAmplitudeModulation().value();
    }
    IOBinaryPrimitives::writeFloatNBytes<uint8_t, 1>(amplitude, file, -MAX_AMPLITUDE,
                                                     MAX_AMPLITUDE);

    uint16_t position = 0;
    if (myKeyframe.getRelativePosition().has_value()) {
      position += static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    }
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(position, file);

    unsigned int frequency = 0;
    if (myKeyframe.getFrequencyModulation().has_value()) {
      frequency = static_cast<unsigned int>(myKeyframe.getFrequencyModulation().value());
    }
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(frequency, file);
  }

  return true;
}

auto IOBinaryBands::readCurveEffect(types::Effect &effect, std::ifstream &file) -> bool {
  auto keyframeCount = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  for (unsigned short keyframeIndex = 0; keyframeIndex < keyframeCount; keyframeIndex++) {
    float amplitude =
        IOBinaryPrimitives::readFloatNBytes<uint8_t, 1>(file, -MAX_AMPLITUDE, MAX_AMPLITUDE);
    auto position = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
    auto myKeyframe = types::Keyframe(static_cast<int>(position), amplitude, std::nullopt);
    effect.addKeyframe(myKeyframe);
  }
  return true;
}

auto IOBinaryBands::writeCurveEffect(types::Effect &effect, std::ofstream &file) -> bool {
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(keyframeCount, file);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    auto myKeyframe = effect.getKeyframeAt(kfIndex);
    float amplitude = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      amplitude = myKeyframe.getAmplitudeModulation().value();
    }
    IOBinaryPrimitives::writeFloatNBytes<uint8_t, 1>(amplitude, file, -MAX_AMPLITUDE,
                                                     MAX_AMPLITUDE);

    uint16_t position = 0;
    if (myKeyframe.getRelativePosition().has_value()) {
      position += static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    }
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(static_cast<uint32_t>(position), file);
  }

  return true;
}

auto IOBinaryBands::readVectorialEffect(types::Effect &effect, std::ifstream &file) -> bool {
  float phase = IOBinaryPrimitives::readFloatNBytes<uint32_t, 4>(file, -MAX_PHASE, MAX_PHASE);
  effect.setPhase(phase);
  auto baseSignal = IOBinaryPrimitives::readNBytes<uint8_t, 1>(file);
  effect.setBaseSignal(static_cast<types::BaseSignal>(baseSignal));
  auto keyframeCount = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  for (unsigned short keyframeIndex = 0; keyframeIndex < keyframeCount; keyframeIndex++) {
    auto amplitudeFrequencyMask = IOBinaryPrimitives::readNBytes<uint8_t, 1>(file);
    auto myKeyframe = types::Keyframe(std::nullopt, std::nullopt, std::nullopt);
    if ((amplitudeFrequencyMask & 0b0000'0001) != 0) {
      float amplitude =
          IOBinaryPrimitives::readFloatNBytes<uint8_t, 1>(file, -MAX_AMPLITUDE, MAX_AMPLITUDE);
      myKeyframe.setAmplitudeModulation(amplitude);
    }
    auto position = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
    myKeyframe.setRelativePosition(position);
    if ((amplitudeFrequencyMask & 0b0000'0010) != 0) {
      auto frequency = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
      myKeyframe.setFrequencyModulation(static_cast<int>(frequency));
    }
    effect.addKeyframe(myKeyframe);
  }

  return true;
}

auto IOBinaryBands::writeVectorialEffect(types::Effect &effect, std::ofstream &file) -> bool {

  float phase = effect.getPhase();
  IOBinaryPrimitives::writeFloatNBytes<uint32_t, 4>(phase, file, -MAX_PHASE, MAX_PHASE);
  auto baseSignal = static_cast<uint8_t>(effect.getBaseSignal());
  IOBinaryPrimitives::writeNBytes<uint8_t, 1>(baseSignal, file);
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(keyframeCount, file);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    auto myKeyframe = effect.getKeyframeAt(kfIndex);
    uint8_t valueMask = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      valueMask |= 0b0000'0001;
    }
    if (myKeyframe.getFrequencyModulation().has_value()) {
      valueMask |= 0b0000'0010;
    }
    IOBinaryPrimitives::writeNBytes<uint8_t, 1>(valueMask, file);

    if (myKeyframe.getAmplitudeModulation().has_value()) {
      auto amplitude = myKeyframe.getAmplitudeModulation().value();
      IOBinaryPrimitives::writeFloatNBytes<uint8_t, 1>(amplitude, file, -MAX_AMPLITUDE,
                                                       MAX_AMPLITUDE);
    }
    auto position = static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>(position, file);

    if (myKeyframe.getFrequencyModulation().has_value()) {
      auto frequency = static_cast<uint16_t>(myKeyframe.getFrequencyModulation().value());
      IOBinaryPrimitives::writeNBytes<uint16_t, 2>(frequency, file);
    }
  }

  return true;
}

auto IOBinaryBands::readWaveletEffect(types::Effect &effect, types::Band &band, std::ifstream &file)
    -> bool {
  spiht::Spiht_Dec dec;
  auto blocklength = band.getWindowLength() * band.getUpperFrequencyLimit() / S2MS;
  auto size = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);

  std::vector<unsigned char> instream;
  instream.resize(size);
  for (auto &b : instream) {
    b = IOBinaryPrimitives::readNBytes<unsigned char, 1>(file);
  }
  dec.decodeEffect(instream, effect, (int)blocklength);

  return true;
}

auto IOBinaryBands::writeWaveletEffect(types::Effect &effect, std::ofstream &file) -> bool {
  spiht::Spiht_Enc enc;
  std::vector<unsigned char> outstream;
  enc.encodeEffect(effect, outstream);
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>((uint16_t)outstream.size(), file);
  for (auto &b : outstream) {
    IOBinaryPrimitives::writeNBytes<unsigned char, 1>(b, file);
  }

  return true;
}

auto IOBinaryBands::readReferenceEffect(types::Effect &effect, std::ifstream &file) -> bool {
  auto id = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  effect.setId(id);
  return true;
}

auto IOBinaryBands::writeReferenceEffect(types::Effect &effect, std::ofstream &file) -> bool {
  int id = effect.getId();
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(id, file);
  return true;
}
auto IOBinaryBands::readTimelineEffect(types::Effect &effect, types::Band &band,
                                       std::ifstream &file) -> bool {
  auto timelineEffectCount = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    types::Effect myEffect;
    auto effectType =
        static_cast<types::EffectType>(IOBinaryPrimitives::readNBytes<uint8_t, 1>(file));
    myEffect.setEffectType(effectType);
    auto position = static_cast<int>(IOBinaryPrimitives::readNBytes<uint32_t, 4>(file));
    myEffect.setPosition(position);
    if (effectType == types::EffectType::Reference) {
      readReferenceEffect(myEffect, file);
    } else if (effectType == types::EffectType::Timeline) {
      readTimelineEffect(myEffect, band, file);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        IOBinaryBands::readTransientEffect(myEffect, file);
        break;
      case types::BandType::Curve:
        IOBinaryBands::readCurveEffect(myEffect, file);
        break;
      case types::BandType::VectorialWave:
        IOBinaryBands::readVectorialEffect(myEffect, file);
        break;
      case types::BandType::WaveletWave:
        IOBinaryBands::readWaveletEffect(myEffect, band, file);
        break;
      default:
        return false;
      }
    }
    effect.addTimelineEffect(myEffect);
  }
  return true;
}

auto IOBinaryBands::writeTimelineEffect(types::Effect &effect, types::Band &band,
                                        std::ofstream &file) -> bool {
  auto timelineEffectCount = static_cast<uint16_t>(effect.getTimelineSize());
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(timelineEffectCount, file);
  // for each library effect
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    auto timelineEffect = effect.getTimelineEffectAt(i);
    auto effectType = static_cast<uint8_t>(timelineEffect.getEffectType());
    IOBinaryPrimitives::writeNBytes<uint8_t, 1>(effectType, file);
    auto position = static_cast<uint32_t>(timelineEffect.getPosition());
    IOBinaryPrimitives::writeNBytes<uint32_t, 4>(position, file);

    if (effect.getEffectType() == types::EffectType::Reference) {
      writeReferenceEffect(effect, file);
    } else if (effect.getEffectType() == types::EffectType::Timeline) {
      writeTimelineEffect(effect, band, file);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        IOBinaryBands::writeTransientEffect(effect, file);
        break;
      case types::BandType::Curve:
        IOBinaryBands::writeCurveEffect(effect, file);
        break;
      case types::BandType::VectorialWave:
        IOBinaryBands::writeVectorialEffect(effect, file);
        break;
      case types::BandType::WaveletWave:
        IOBinaryBands::writeWaveletEffect(effect, file);
        break;
      default:
        return false;
      }
    }
  }
  return true;
}

} // namespace haptics::io
