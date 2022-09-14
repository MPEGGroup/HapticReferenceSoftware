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
#include <IOHaptics/include/IOBinaryFields.h>
#include <limits>

namespace haptics::io {

auto IOBinaryBands::readBandHeader(types::Band &band, std::ifstream &file,
                                   std::vector<bool> &unusedBits) -> bool {
  auto bandType = IOBinaryPrimitives::readNBits<uint8_t, MDBAND_BAND_TYPE>(file, unusedBits);
  band.setBandType(static_cast<types::BandType>(bandType));
  if (band.getBandType() == types::BandType::Curve) {
    auto curveType = IOBinaryPrimitives::readNBits<uint8_t, MDBAND_CURVE_TYPE>(file, unusedBits);
    band.setCurveType(static_cast<types::CurveType>(curveType));
  } else if (band.getBandType() == types::BandType::WaveletWave) {
    auto windowLength =
        IOBinaryPrimitives::readNBits<uint16_t, MDBAND_WIN_LEN>(file, unusedBits);
    band.setWindowLength(static_cast<int>(windowLength));
  }

  auto lowerFrequencyLimit =
      IOBinaryPrimitives::readNBits<uint16_t, MDBAND_LOW_FREQ>(file, unusedBits);
  band.setLowerFrequencyLimit(static_cast<int>(lowerFrequencyLimit));

  auto upperFrequencyLimit =
      IOBinaryPrimitives::readNBits<uint16_t, MDBAND_UP_FREQ>(file, unusedBits);
  band.setUpperFrequencyLimit(static_cast<int>(upperFrequencyLimit));

  auto effectCount = IOBinaryPrimitives::readNBits<uint16_t, MDBAND_EFFECT_COUNT>(file, unusedBits);
  for (unsigned int i = 0; i < effectCount; i++) {
    types::Effect e;
    band.addEffect(e);
  }

  return true;
}

auto IOBinaryBands::writeBandHeader(types::Band &band, std::vector<bool> &output) -> bool {
  types::BandType t = band.getBandType();
  auto bandType = static_cast<unsigned short>(t);
  IOBinaryPrimitives::writeNBits<uint8_t, MDBAND_BAND_TYPE>(bandType, output);

  if (band.getBandType() == types::BandType::Curve) {
    auto curveType = static_cast<uint8_t>(band.getCurveType());
    IOBinaryPrimitives::writeNBits<uint8_t, MDBAND_CURVE_TYPE>(curveType, output);
  } else if (band.getBandType() == types::BandType::WaveletWave) {
    auto windowLength = static_cast<uint16_t>(band.getWindowLength());
    IOBinaryPrimitives::writeNBits<uint16_t, MDBAND_WIN_LEN>(windowLength, output);
  }

  auto lowerFrequencyLimit = static_cast<unsigned int>(band.getLowerFrequencyLimit());
  IOBinaryPrimitives::writeNBits<uint16_t, MDBAND_LOW_FREQ>(lowerFrequencyLimit, output);

  auto upperFrequencyLimit = static_cast<unsigned int>(band.getUpperFrequencyLimit());
  IOBinaryPrimitives::writeNBits<uint16_t, MDBAND_UP_FREQ>(upperFrequencyLimit, output);

  auto effectCount = static_cast<unsigned int>(band.getEffectsSize());
  IOBinaryPrimitives::writeNBits<uint16_t, MDBAND_EFFECT_COUNT>(effectCount, output);

  return true;
}

auto IOBinaryBands::readBandBody(types::Band &band, std::ifstream &file,
                                 std::vector<bool> &unusedBits) -> bool {
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    auto myEffect = band.getEffectAt(effectIndex);
    auto effectType = static_cast<types::EffectType>(
        IOBinaryPrimitives::readNBits<uint8_t, EFFECT_TYPE>(file, unusedBits));
    myEffect.setEffectType(effectType);
    auto position = 0;
    if ((myEffect.getEffectType() == types::EffectType::Basis &&
         band.getBandType() == types::BandType::WaveletWave)) {
      position = effectIndex * band.getWindowLength();
    } else {
      position = static_cast<int>(IOBinaryPrimitives::readNBits<uint32_t, EFFECT_POSITION>(file, unusedBits));
    }
    myEffect.setPosition(position);
    if (effectType == types::EffectType::Reference) {
      readReferenceEffect(myEffect, file, unusedBits);
    } else if (effectType == types::EffectType::Timeline) {
      readTimelineEffect(myEffect, band, file, unusedBits);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        if (!IOBinaryBands::readTransientEffect(myEffect, file, unusedBits)) {
          return false;
        }
        break;
      case types::BandType::Curve:
        if (!IOBinaryBands::readCurveEffect(myEffect, file, unusedBits)) {
          return false;
        }
        break;
      case types::BandType::VectorialWave:
        if (!IOBinaryBands::readVectorialEffect(myEffect, file, unusedBits)) {
          return false;
        }
        break;
      case types::BandType::WaveletWave:
        if (!IOBinaryBands::readWaveletEffect(myEffect, band, file, unusedBits)) {
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

auto IOBinaryBands::writeBandBody(types::Band &band, std::vector<bool> &output) -> bool {
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    auto myEffect = band.getEffectAt(effectIndex);
    auto effectType = static_cast<uint8_t>(myEffect.getEffectType());
    IOBinaryPrimitives::writeNBits<uint8_t, EFFECT_TYPE>(effectType, output);
    if ((myEffect.getEffectType() != types::EffectType::Basis ||
         band.getBandType() != types::BandType::WaveletWave)) {
      auto position = static_cast<uint32_t>(myEffect.getPosition());
      IOBinaryPrimitives::writeNBits<uint32_t, EFFECT_POSITION>(position, output);
    }
    if (myEffect.getEffectType() == types::EffectType::Reference) {
      writeReferenceEffect(myEffect, output);
    } else if (myEffect.getEffectType() == types::EffectType::Timeline) {
      writeTimelineEffect(myEffect, band, output);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        if (!IOBinaryBands::writeTransientEffect(myEffect, output)) {
          return false;
        }
        break;
      case types::BandType::Curve:
        if (!IOBinaryBands::writeCurveEffect(myEffect, output)) {
          return false;
        }
        break;
      case types::BandType::VectorialWave:
        if (!IOBinaryBands::writeVectorialEffect(myEffect, output)) {
          return false;
        }
        break;
      case types::BandType::WaveletWave:
        if (!IOBinaryBands::writeWaveletEffect(myEffect, output)) {
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

auto IOBinaryBands::readTransientEffect(types::Effect &effect, std::ifstream &file,
                                        std::vector<bool> &unusedBits) -> bool {

  auto keyframeCount =
      IOBinaryPrimitives::readNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(file, unusedBits);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    float amplitude = IOBinaryPrimitives::readFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
        file, -MAX_AMPLITUDE, MAX_AMPLITUDE, unusedBits);
    auto position = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_POSITION>(file, unusedBits);
    auto frequency = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_FREQUENCY>(file, unusedBits);

    auto myKeyframe = types::Keyframe(position, amplitude, frequency);
    effect.addKeyframe(myKeyframe);
  }

  return true;
}

auto IOBinaryBands::writeTransientEffect(types::Effect &effect, std::vector<bool> &output) -> bool {
  types::Keyframe myKeyframe;
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(keyframeCount, output);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    myKeyframe = effect.getKeyframeAt(kfIndex);
    float amplitude = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      amplitude = myKeyframe.getAmplitudeModulation().value();
    }
    IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(amplitude, output,
                                                                     -MAX_AMPLITUDE,
                                                            MAX_AMPLITUDE);

    uint16_t position = 0;
    if (myKeyframe.getRelativePosition().has_value()) {
      position += static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    }
    IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_POSITION>(position, output);

    unsigned int frequency = 0;
    if (myKeyframe.getFrequencyModulation().has_value()) {
      frequency = static_cast<unsigned int>(myKeyframe.getFrequencyModulation().value());
    }
    IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_FREQUENCY>(frequency, output);
  }

  return true;
}

auto IOBinaryBands::readCurveEffect(types::Effect &effect, std::ifstream &file,
                                    std::vector<bool> &unusedBits) -> bool {
  auto keyframeCount =
      IOBinaryPrimitives::readNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(file, unusedBits);
  for (unsigned short keyframeIndex = 0; keyframeIndex < keyframeCount; keyframeIndex++) {
    float amplitude = IOBinaryPrimitives::readFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
        file, -MAX_AMPLITUDE, MAX_AMPLITUDE, unusedBits);
    auto position = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_POSITION>(file, unusedBits);
    auto myKeyframe = types::Keyframe(static_cast<int>(position), amplitude, std::nullopt);
    effect.addKeyframe(myKeyframe);
  }
  return true;
}

auto IOBinaryBands::writeCurveEffect(types::Effect &effect, std::vector<bool> &output) -> bool {
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(keyframeCount, output);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    auto myKeyframe = effect.getKeyframeAt(kfIndex);
    float amplitude = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      amplitude = myKeyframe.getAmplitudeModulation().value();
    }
    IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(amplitude, output,
                                                                     -MAX_AMPLITUDE,
                                                            MAX_AMPLITUDE);

    uint16_t position = 0;
    if (myKeyframe.getRelativePosition().has_value()) {
      position += static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    }
    IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_POSITION>(static_cast<uint16_t>(position),
                                                            output);
  }

  return true;
}

auto IOBinaryBands::readVectorialEffect(types::Effect &effect, std::ifstream &file,
                                        std::vector<bool> &unusedBits) -> bool {
  float phase = IOBinaryPrimitives::readFloatNBits<uint16_t, EFFECT_PHASE>(file, -MAX_PHASE,
                                                                            MAX_PHASE, unusedBits);
  effect.setPhase(phase);
  auto baseSignal = IOBinaryPrimitives::readNBits<uint8_t, EFFECT_BASE_SIGNAL>(file, unusedBits);
  effect.setBaseSignal(static_cast<types::BaseSignal>(baseSignal));
  auto keyframeCount =
      IOBinaryPrimitives::readNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(file, unusedBits);
  for (unsigned short keyframeIndex = 0; keyframeIndex < keyframeCount; keyframeIndex++) {
    auto amplitudeFrequencyMask =
        IOBinaryPrimitives::readNBits<uint8_t, KEYFRAME_MASK>(file, unusedBits);
    auto myKeyframe = types::Keyframe(std::nullopt, std::nullopt, std::nullopt);
    if ((amplitudeFrequencyMask & 0b0000'0001) != 0) {
      float amplitude = IOBinaryPrimitives::readFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
          file, -MAX_AMPLITUDE, MAX_AMPLITUDE, unusedBits);
      myKeyframe.setAmplitudeModulation(amplitude);
    }
    auto position = IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_POSITION>(file, unusedBits);
    myKeyframe.setRelativePosition(position);
    if ((amplitudeFrequencyMask & 0b0000'0010) != 0) {
      auto frequency =
          IOBinaryPrimitives::readNBits<uint16_t, KEYFRAME_FREQUENCY>(file, unusedBits);
      myKeyframe.setFrequencyModulation(static_cast<int>(frequency));
    }
    effect.addKeyframe(myKeyframe);
  }

  return true;
}

auto IOBinaryBands::writeVectorialEffect(types::Effect &effect, std::vector<bool> &output) -> bool {

  float phase = effect.getPhase();
  IOBinaryPrimitives::writeFloatNBits<uint16_t, EFFECT_PHASE>(phase, output, -MAX_PHASE,
                                                               MAX_PHASE);
  auto baseSignal = static_cast<uint8_t>(effect.getBaseSignal());
  IOBinaryPrimitives::writeNBits<uint8_t, EFFECT_BASE_SIGNAL>(baseSignal, output);
  auto keyframeCount = static_cast<uint16_t>(effect.getKeyframesSize());
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_KEYFRAME_COUNT>(keyframeCount, output);
  for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
    auto myKeyframe = effect.getKeyframeAt(kfIndex);
    uint8_t valueMask = 0;
    if (myKeyframe.getAmplitudeModulation().has_value()) {
      valueMask |= 0b0000'0001;
    }
    if (myKeyframe.getFrequencyModulation().has_value()) {
      valueMask |= 0b0000'0010;
    }
    IOBinaryPrimitives::writeNBits<uint8_t, KEYFRAME_MASK>(valueMask, output);

    if (myKeyframe.getAmplitudeModulation().has_value()) {
      auto amplitude = myKeyframe.getAmplitudeModulation().value();
      IOBinaryPrimitives::writeFloatNBits<uint8_t, KEYFRAME_AMPLITUDE>(
          amplitude, output, -MAX_AMPLITUDE,
                                                              MAX_AMPLITUDE);
    }
    auto position = static_cast<uint16_t>(myKeyframe.getRelativePosition().value());
    IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_POSITION>(position, output);

    if (myKeyframe.getFrequencyModulation().has_value()) {
      auto frequency = static_cast<uint16_t>(myKeyframe.getFrequencyModulation().value());
      IOBinaryPrimitives::writeNBits<uint16_t, KEYFRAME_FREQUENCY>(frequency, output);
    }
  }

  return true;
}

auto IOBinaryBands::readWaveletEffect(types::Effect &effect, types::Band &band, std::ifstream &file,
                                      std::vector<bool> &unusedBits) -> bool {
  spiht::Spiht_Dec dec;
  auto blocklength = band.getWindowLength() * band.getUpperFrequencyLimit() / S2MS;
  auto size = IOBinaryPrimitives::readNBits<uint16_t, EFFECT_WAVELET_SIZE>(file, unusedBits);

  std::vector<unsigned char> instream;
  instream.resize(size);
  for (auto &b : instream) {
    b = IOBinaryPrimitives::readNBits<unsigned char, BYTE_SIZE>(file, unusedBits);
  }
  dec.decodeEffect(instream, effect, (int)blocklength);

  return true;
}

auto IOBinaryBands::writeWaveletEffect(types::Effect &effect, std::vector<bool> &output) -> bool {
  spiht::Spiht_Enc enc;
  std::vector<unsigned char> outstream;
  enc.encodeEffect(effect, outstream);
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_WAVELET_SIZE>((uint16_t)outstream.size(), output);
  for (auto &b : outstream) {
    IOBinaryPrimitives::writeNBits<unsigned char, BYTE_SIZE>(b, output);
  }

  return true;
}

auto IOBinaryBands::readReferenceEffect(types::Effect &effect, std::ifstream &file,
                                        std::vector<bool> &unusedBits) -> bool {
  auto id = IOBinaryPrimitives::readNBits<uint16_t, EFFECT_ID>(file, unusedBits);
  effect.setId(id);
  return true;
}

auto IOBinaryBands::writeReferenceEffect(types::Effect &effect, std::vector<bool> &output) -> bool {
  int id = effect.getId();
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_ID>(id, output);
  return true;
}
auto IOBinaryBands::readTimelineEffect(types::Effect &effect, types::Band &band,
                                       std::ifstream &file, std::vector<bool> &unusedBits) -> bool {
  auto timelineEffectCount =
      IOBinaryPrimitives::readNBits<uint16_t, EFFECT_TIMELINE_COUNT>(file, unusedBits);
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    types::Effect myEffect;
    auto effectType = static_cast<types::EffectType>(
        IOBinaryPrimitives::readNBits<uint8_t, EFFECT_TYPE>(file, unusedBits));
    myEffect.setEffectType(effectType);
    auto position = static_cast<int>(IOBinaryPrimitives::readNBits<uint32_t, EFFECT_POSITION>(file, unusedBits));
    myEffect.setPosition(position);
    if (effectType == types::EffectType::Reference) {
      readReferenceEffect(myEffect, file, unusedBits);
    } else if (effectType == types::EffectType::Timeline) {
      readTimelineEffect(myEffect, band, file, unusedBits);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        IOBinaryBands::readTransientEffect(myEffect, file, unusedBits);
        break;
      case types::BandType::Curve:
        IOBinaryBands::readCurveEffect(myEffect, file, unusedBits);
        break;
      case types::BandType::VectorialWave:
        IOBinaryBands::readVectorialEffect(myEffect, file, unusedBits);
        break;
      case types::BandType::WaveletWave:
        IOBinaryBands::readWaveletEffect(myEffect, band, file, unusedBits);
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
                                        std::vector<bool> &output) -> bool {
  auto timelineEffectCount = static_cast<uint16_t>(effect.getTimelineSize());
  IOBinaryPrimitives::writeNBits<uint16_t, EFFECT_TIMELINE_COUNT>(timelineEffectCount, output);
  // for each library effect
  for (unsigned short i = 0; i < timelineEffectCount; i++) {
    auto timelineEffect = effect.getTimelineEffectAt(i);
    auto effectType = static_cast<uint8_t>(timelineEffect.getEffectType());
    IOBinaryPrimitives::writeNBits<uint8_t, EFFECT_TYPE>(effectType, output);
    auto position = static_cast<uint16_t>(timelineEffect.getPosition());
    IOBinaryPrimitives::writeNBits<uint32_t, EFFECT_POSITION>(position, output);

    if (effect.getEffectType() == types::EffectType::Reference) {
      writeReferenceEffect(effect, output);
    } else if (effect.getEffectType() == types::EffectType::Timeline) {
      writeTimelineEffect(effect, band, output);
    } else {
      switch (band.getBandType()) {
      case types::BandType::Transient:
        IOBinaryBands::writeTransientEffect(effect, output);
        break;
      case types::BandType::Curve:
        IOBinaryBands::writeCurveEffect(effect, output);
        break;
      case types::BandType::VectorialWave:
        IOBinaryBands::writeVectorialEffect(effect, output);
        break;
      case types::BandType::WaveletWave:
        IOBinaryBands::writeWaveletEffect(effect, output);
        break;
      default:
        return false;
      }
    }
  }
  return true;
}

} // namespace haptics::io
