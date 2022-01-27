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
  auto bandType = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);
  band.setBandType(static_cast<types::BandType>(bandType));

  auto encodingmodality =
      IOBinaryPrimitives::IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);
  band.setEncodingModality(static_cast<types::EncodingModality>(encodingmodality));

  if (band.getBandType() == types::BandType::Wave &&
      (band.getEncodingModality() == types::EncodingModality::Quantized ||
       band.getEncodingModality() == types::EncodingModality::Wavelet)) {
    auto windowLength = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    band.setWindowLength(static_cast<int>(windowLength));
  }

  auto lowerFrequencyLimit = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
  band.setLowerFrequencyLimit(static_cast<int>(lowerFrequencyLimit));

  auto upperFrequencyLimit = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
  band.setUpperFrequencyLimit(static_cast<int>(upperFrequencyLimit));

  if (band.getBandType() == types::BandType::Transient) {
    auto keyframeCount = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    for (unsigned int i = 0; i < keyframeCount; i++) {
      types::Effect e;
      band.addEffect(e);
    }
  } else if (band.getBandType() == types::BandType::Curve) {
    auto keyframeCount = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    types::Effect e;
    for (unsigned int i = 0; i < keyframeCount; i++) {
      types::Keyframe kf;
      e.addKeyframe(kf);
    }
    band.addEffect(e);
  } else if (band.getBandType() == types::BandType::Wave &&
             band.getEncodingModality() == types::EncodingModality::Vectorial) {
    auto keyframeCount = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    for (unsigned int i = 0; i < keyframeCount; i++) {
      types::Effect e;
      band.addEffect(e);
    }
  }

  return true;
}

auto IOBinaryBands::writeBandHeader(types::Band &band, std::ofstream &file) -> bool {
  types::BandType t = band.getBandType();
  auto bandType = static_cast<unsigned short>(t);
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(bandType, file);

  auto encodingmodality = static_cast<unsigned short>(band.getEncodingModality());
  IOBinaryPrimitives::writeNBytes<unsigned short, 2>(encodingmodality, file);

  if (band.getBandType() == types::BandType::Wave &&
      (band.getEncodingModality() == types::EncodingModality::Quantized ||
       band.getEncodingModality() == types::EncodingModality::Wavelet)) {
    auto windowLength = static_cast<unsigned int>(band.getWindowLength());
    IOBinaryPrimitives::writeNBytes<unsigned int, 4>(windowLength, file);
  }

  auto lowerFrequencyLimit = static_cast<unsigned int>(band.getLowerFrequencyLimit());
  IOBinaryPrimitives::writeNBytes<unsigned int, 4>(lowerFrequencyLimit, file);

  auto upperFrequencyLimit = static_cast<unsigned int>(band.getUpperFrequencyLimit());
  IOBinaryPrimitives::writeNBytes<unsigned int, 4>(upperFrequencyLimit, file);

  if (band.getBandType() == types::BandType::Transient ||
      band.getBandType() == types::BandType::Curve) {
    unsigned int keyframeCount = 0;
    types::Effect myEffect;
    for (int i = 0; i < static_cast<int>(band.getEffectsSize()); i++) {
      myEffect = band.getEffectAt(i);
      keyframeCount += static_cast<unsigned int>(myEffect.getKeyframesSize());
    }
    IOBinaryPrimitives::writeNBytes<unsigned int, 4>(keyframeCount, file);
  } else if (band.getBandType() == types::BandType::Wave &&
             band.getEncodingModality() == types::EncodingModality::Vectorial) {
    auto keyframeCount = static_cast<unsigned int>(band.getEffectsSize());
    IOBinaryPrimitives::writeNBytes<unsigned int, 4>(keyframeCount, file);
  }

  return true;
}

auto IOBinaryBands::readBandBody(types::Band &band, std::ifstream &file) -> bool {
  switch (band.getBandType()) {
  case types::BandType::Transient:
    return IOBinaryBands::readTransientBandBody(band, file);
  case types::BandType::Curve:
    return IOBinaryBands::readCurveBandBody(band, file);
  case types::BandType::Wave:
    switch (band.getEncodingModality()) {
    case types::EncodingModality::Vectorial:
      return IOBinaryBands::readVectorialBandBody(band, file);
    case types::EncodingModality::Quantized:
      return IOBinaryBands::readQuantizedBandBody(band, file);
    case types::EncodingModality::Wavelet:
      return IOBinaryBands::readWaveletBandBody(band, file);
    default:
      return true;
    }
  default:
    return true;
  }
}

auto IOBinaryBands::writeBandBody(types::Band &band, std::ofstream &file) -> bool {
  switch (band.getBandType()) {
  case types::BandType::Transient:
    return IOBinaryBands::writeTransientBandBody(band, file);
  case types::BandType::Curve:
    return IOBinaryBands::writeCurveBandBody(band, file);
  case types::BandType::Wave:
    switch (band.getEncodingModality()) {
    case types::EncodingModality::Vectorial:
      return IOBinaryBands::writeVectorialBandBody(band, file);
    case types::EncodingModality::Quantized:
      return IOBinaryBands::writeQuantizedBandBody(band, file);
    case types::EncodingModality::Wavelet:
      return IOBinaryBands::writeWaveletBandBody(band, file);
    default:
      return true;
    }
  default:
    return true;
  }
}

auto IOBinaryBands::readTransientBandBody(types::Band &band, std::ifstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    float amplitude = IOBinaryPrimitives::readFloat(file);
    auto position = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    auto frequency = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);

    myKeyframe = types::Keyframe(0, amplitude, static_cast<int>(frequency));
    myEffect = types::Effect(static_cast<int>(position), 0, types::BaseSignal::Sine);
    myEffect.addKeyframe(myKeyframe);
    band.replaceEffectAt(effectIndex, myEffect);
  }

  return true;
}

auto IOBinaryBands::writeTransientBandBody(types::Band &band, std::ofstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    myEffect = band.getEffectAt(effectIndex);
    for (int kfIndex = 0; kfIndex < static_cast<int>(myEffect.getKeyframesSize()); kfIndex++) {
      myKeyframe = myEffect.getKeyframeAt(kfIndex);

      float amplitude = 0;
      if (myKeyframe.getAmplitudeModulation().has_value()) {
        amplitude = myKeyframe.getAmplitudeModulation().value();
      }
      IOBinaryPrimitives::writeFloat(amplitude, file);

      auto position = static_cast<unsigned int>(myEffect.getPosition());
      if (myKeyframe.getRelativePosition().has_value()) {
        position += static_cast<unsigned int>(myKeyframe.getRelativePosition().value());
      }
      IOBinaryPrimitives::writeNBytes<unsigned int, 4>(position, file);

      unsigned int frequency = 0;
      if (myKeyframe.getFrequencyModulation().has_value()) {
        frequency = static_cast<unsigned int>(myKeyframe.getFrequencyModulation().value());
      }
      IOBinaryPrimitives::writeNBytes<unsigned int, 4>(frequency, file);
    }
  }

  return true;
}

auto IOBinaryBands::readCurveBandBody(types::Band &band, std::ifstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  unsigned int effectPosition = 0;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    myEffect = band.getEffectAt(effectIndex);
    myEffect.setPhase(0);
    myEffect.setBaseSignal(types::BaseSignal::Sine);
    effectPosition = 0;
    for (int keyframeIndex = 0; keyframeIndex < static_cast<int>(myEffect.getKeyframesSize());
         keyframeIndex++) {
      float amplitude = IOBinaryPrimitives::readFloat(file);
      auto position = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);

      if (keyframeIndex == 0) {
        effectPosition = position;
        position = 0;
      } else {
        position -= effectPosition;
      }

      myKeyframe = types::Keyframe(static_cast<int>(position), amplitude, std::nullopt);
      myEffect.replaceKeyframeAt(keyframeIndex, myKeyframe);
    }

    band.replaceEffectAt(effectIndex, myEffect);
  }

  return true;
}

auto IOBinaryBands::writeCurveBandBody(types::Band &band, std::ofstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    myEffect = band.getEffectAt(effectIndex);
    for (int kfIndex = 0; kfIndex < static_cast<int>(myEffect.getKeyframesSize()); kfIndex++) {
      myKeyframe = myEffect.getKeyframeAt(kfIndex);

      float amplitude = 0;
      if (myKeyframe.getAmplitudeModulation().has_value()) {
        amplitude = myKeyframe.getAmplitudeModulation().value();
      }
      IOBinaryPrimitives::writeFloat(amplitude, file);

      auto position = static_cast<unsigned int>(myEffect.getPosition());
      if (myKeyframe.getRelativePosition().has_value()) {
        position += static_cast<unsigned int>(myKeyframe.getRelativePosition().value());
      }
      IOBinaryPrimitives::writeNBytes<unsigned int, 4>(position, file);
    }
  }

  return true;
}

auto IOBinaryBands::readVectorialBandBody(types::Band &band, std::ifstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    auto keyframeCount = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);
    for (int keyframeIndex = 0; keyframeIndex < static_cast<int>(keyframeCount); keyframeIndex++) {
      auto amplitudeFrequencyMask = IOBinaryPrimitives::readNBytes<unsigned char, 1>(file);

      myKeyframe = types::Keyframe(std::nullopt, std::nullopt, std::nullopt);
      if ((amplitudeFrequencyMask & 0b0000'0001) != 0) {
        float amplitude = IOBinaryPrimitives::readFloat(file);
        myKeyframe.setAmplitudeModulation(amplitude);
      }
      auto position = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
      if ((amplitudeFrequencyMask & 0b0000'0010) != 0) {
        auto frequency = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
        myKeyframe.setFrequencyModulation(static_cast<int>(frequency));
      }

      if (keyframeIndex == 0) {
        float phase = IOBinaryPrimitives::readFloat(file);
        auto baseSignal = IOBinaryPrimitives::readNBytes<unsigned short, 2>(file);

        myEffect = types::Effect(static_cast<int>(position), phase,
                                 static_cast<types::BaseSignal>(baseSignal));
        myKeyframe.setRelativePosition(0);
      } else {
        int effectPosition = myEffect.getPosition();
        myKeyframe.setRelativePosition(static_cast<int>(position - effectPosition));
      }

      myEffect.addKeyframe(myKeyframe);
    }
    band.replaceEffectAt(effectIndex, myEffect);
  }

  return true;
}

auto IOBinaryBands::writeVectorialBandBody(types::Band &band, std::ofstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    myEffect = band.getEffectAt(effectIndex);
    auto keyframeCount = static_cast<unsigned short>(myEffect.getKeyframesSize());
    IOBinaryPrimitives::writeNBytes<unsigned short, 2>(keyframeCount, file);

    for (unsigned short kfIndex = 0; kfIndex < keyframeCount; kfIndex++) {
      myKeyframe = myEffect.getKeyframeAt(kfIndex);

      unsigned char valueMask = 0;
      if (myKeyframe.getAmplitudeModulation().has_value()) {
        valueMask |= 0b0000'0001;
      }
      if (myKeyframe.getFrequencyModulation().has_value()) {
        valueMask |= 0b0000'0010;
      }
      IOBinaryPrimitives::writeNBytes<unsigned char, 1>(valueMask, file);

      float amplitude = 0;
      if (myKeyframe.getAmplitudeModulation().has_value()) {
        amplitude = myKeyframe.getAmplitudeModulation().value();
        IOBinaryPrimitives::writeFloat(amplitude, file);
      }

      auto position = static_cast<unsigned int>(myEffect.getPosition());
      if (myKeyframe.getRelativePosition().has_value()) {
        position += static_cast<unsigned int>(myKeyframe.getRelativePosition().value());
      }
      IOBinaryPrimitives::writeNBytes<unsigned int, 4>(position, file);

      unsigned int frequency = 0;
      if (myKeyframe.getFrequencyModulation().has_value()) {
        frequency = static_cast<unsigned int>(myKeyframe.getFrequencyModulation().value());
        IOBinaryPrimitives::writeNBytes<unsigned int, 4>(frequency, file);
      }

      if (kfIndex == 0) {
        float phase = myEffect.getPhase();
        IOBinaryPrimitives::writeFloat(phase, file);

        auto baseSignal = static_cast<unsigned short>(myEffect.getBaseSignal());
        IOBinaryPrimitives::writeNBytes<unsigned short, 2>(baseSignal, file);
      }
    }
  }

  return true;
}

auto IOBinaryBands::readQuantizedBandBody(types::Band &band, std::ifstream &file) -> bool {
  types::Effect myEffect(0, 0, types::BaseSignal::Sine);
  types::Keyframe myKeyframe;
  bool isEOB = false;
  bool wasEOE = true;

  while (!isEOB) {
    float amplitude = IOBinaryPrimitives::readFloat(file);
    unsigned int position = 0;
    if (amplitude == 0) {
      if (!wasEOE) {
        band.addEffect(myEffect);
        wasEOE = true;
        continue;
      }

      position = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
      isEOB = position == std::numeric_limits<unsigned int>::max();
      if (isEOB) {
        break;
      }
    } else if (wasEOE) {
      position = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    }

    auto frequency = IOBinaryPrimitives::readNBytes<unsigned int, 4>(file);
    if (wasEOE) {
      float phase = IOBinaryPrimitives::readFloat(file);
      myEffect = types::Effect(static_cast<int>(position), phase, types::BaseSignal::Sine);
    }

    myKeyframe = types::Keyframe(std::nullopt, amplitude, frequency);
    myEffect.addKeyframe(myKeyframe);
    wasEOE = false;
  }

  return true;
}

auto IOBinaryBands::writeQuantizedBandBody(types::Band &band, std::ofstream &file) -> bool {
  types::Effect myEffect;
  types::Keyframe myKeyframe;
  bool lastValueWasNull = true;
  for (int effectIndex = 0; effectIndex < static_cast<int>(band.getEffectsSize()); effectIndex++) {
    myEffect = band.getEffectAt(effectIndex);
    auto startingPosition = static_cast<unsigned int>(myEffect.getPosition());
    for (int kfIndex = 0; kfIndex < static_cast<int>(myEffect.getKeyframesSize()); kfIndex++) {
      myKeyframe = myEffect.getKeyframeAt(kfIndex);

      float amplitude = 0;
      if (myKeyframe.getAmplitudeModulation().has_value()) {
        amplitude = myKeyframe.getAmplitudeModulation().value();
      }
      if (amplitude == 0) {
        if (!lastValueWasNull) {
          IOBinaryPrimitives::writeFloat(amplitude, file);
          lastValueWasNull = true;
        }
        continue;
      }
      IOBinaryPrimitives::writeFloat(amplitude, file);

      if (lastValueWasNull) {
        auto position = startingPosition + band.getWindowLength() * kfIndex;
        IOBinaryPrimitives::writeNBytes<unsigned int, 4>(position, file);
      }

      unsigned int frequency = 0;
      if (myKeyframe.getFrequencyModulation().has_value()) {
        frequency = static_cast<unsigned int>(myKeyframe.getFrequencyModulation().value());
      }
      IOBinaryPrimitives::writeNBytes<unsigned int, 4>(frequency, file);

      if (lastValueWasNull) {
        float phase = myEffect.getPhase();
        IOBinaryPrimitives::writeNBytes<float, 4>(phase, file);
      }

      lastValueWasNull = false;
    }

    if (!lastValueWasNull) {
      IOBinaryPrimitives::writeNBytes<float, 4>(0x0000, file);
      lastValueWasNull = true;
    }
  }
  IOBinaryPrimitives::writeNBytes<float, 4>(0x0000, file);
  IOBinaryPrimitives::writeNBytes<unsigned int, 4>(std::numeric_limits<unsigned int>::max(), file);

  return true;
}

auto IOBinaryBands::readWaveletBandBody(types::Band &band, std::ifstream &file) -> bool {
  spiht::Spiht_Dec dec;
  auto effects_size = IOBinaryPrimitives::readNBytes<uint16_t, 2>(file);
  auto blocklength = IOBinaryPrimitives::readNBytes<uint16_t, 1>(file);
  band.setWindowLength(blocklength);
  for (uint16_t i = 0; i < effects_size; i++) {
    std::vector<unsigned char> instream;
    instream.resize(IOBinaryPrimitives::readNBytes<uint16_t, 2>(file));
    for (auto &b : instream) {
      b = IOBinaryPrimitives::readNBytes<unsigned char, 1>(file);
    }
    types::Effect effect;
    dec.decodeEffect(instream, effect, (int)blocklength);
    band.addEffect(effect);
  }
  return true;
}

auto IOBinaryBands::writeWaveletBandBody(types::Band &band, std::ofstream &file) -> bool {
  spiht::Spiht_Enc enc;
  auto effects_size = (uint16_t)band.getEffectsSize();
  IOBinaryPrimitives::writeNBytes<uint16_t, 2>(effects_size, file);
  int blocklength = band.getWindowLength();
  IOBinaryPrimitives::writeNBytes<int, 1>(blocklength, file);
  for (uint16_t i = 0; i < (uint16_t)band.getEffectsSize(); i++) {
    std::vector<unsigned char> outstream;
    enc.encodeEffect(band.getEffectAt(i), outstream);
    IOBinaryPrimitives::writeNBytes<uint16_t, 2>((uint16_t)outstream.size(), file);
    for (auto &b : outstream) {
      IOBinaryPrimitives::writeNBytes<unsigned char, 1>(b, file);
    }
  }
  return true;
}
} // namespace haptics::io
