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

#include <Types/include/Track.h>

namespace haptics::types {

[[nodiscard]] auto Track::getId() const -> int { return id; }

auto Track::setId(int newId) -> void { id = newId; }

[[nodiscard]] auto Track::getDescription() const -> std::string { return description; }

auto Track::setDescription(std::string &newDescription) -> void { description = newDescription; }

[[nodiscard]] auto Track::getGain() const -> float { return gain; }

auto Track::setGain(float newGain) -> void { gain = newGain; }

[[nodiscard]] auto Track::getMixingWeight() const -> float { return mixingWeight; }

auto Track::setMixingWeight(float newMixingWeight) -> void { mixingWeight = newMixingWeight; }

[[nodiscard]] auto Track::getBodyPartMask() const -> uint32_t { return bodyPartMask; }

auto Track::setBodyPartMask(uint32_t newBodyPartMask) -> void { bodyPartMask = newBodyPartMask; }

[[nodiscard]] auto Track::getReferenceDeviceId() const -> std::optional<int> {
  return referenceDeviceId;
}
auto Track::setReferenceDeviceId(int newReferenceDeviceId) -> void {
  referenceDeviceId = newReferenceDeviceId;
}

auto Track::getVerticesSize() -> size_t { return vertices.size(); }

auto Track::getVertexAt(int index) -> int & { return vertices.at(index); }

auto Track::addVertex(int &newVertice) -> void { vertices.push_back(newVertice); }

auto Track::getBandsSize() -> size_t { return bands.size(); }

auto Track::getBandAt(int index) -> haptics::types::Band & { return bands.at(index); }

auto Track::addBand(haptics::types::Band &newBand) -> void { bands.push_back(newBand); }

auto Track::replaceBandAt(int index, haptics::types::Band &newBand) -> bool {
  if (index < 0 || index >= (int)this->getBandsSize()) {
    return false;
  }
  this->bands[index] = newBand;
  return true;
}

auto Track::generateBand() -> haptics::types::Band * {
  Band newBand;
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Track::generateBand(BandType bandType, CurveType curveType, EncodingModality encodingModality,
                         int windowLength, int lowerFrequencyLimit, int upperFrequencyLimit)
    -> haptics::types::Band * {
  Band newBand(bandType, curveType, encodingModality, windowLength, lowerFrequencyLimit,
               upperFrequencyLimit);
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Track::findBandAvailable(const int position, const int duration,
                              const types::BandType bandType,
                              const types::EncodingModality encodingModality)
    -> haptics::types::Band * {
  haptics::types::Effect e;
  bool bandIsAvailable = true;
  for (haptics::types::Band &b : bands) {
    if (b.getBandType() != bandType || b.getEncodingModality() != encodingModality) {
      continue;
    }

    bandIsAvailable = true;
    for (uint32_t i = 0; i < b.getEffectsSize(); i++) {
      e = b.getEffectAt((int)i);
      if (b.isOverlapping(e, position, position + duration)) {
        bandIsAvailable = false;
        break;
      }
    }
    if (bandIsAvailable) {
      return &b;
    }
  }

  return nullptr;
}

auto Track::Evaluate(double position) -> double {

  double res = 0;


  for (haptics::types::Band &b : bands) {
    res += b.Evaluate(position, b.getLowerFrequencyLimit(), b.getUpperFrequencyLimit());
  }

  if (res < -1) {
    return -1;
  }
  if (res > 1) {
    return 1;
  }
  return res;
}


auto Track::EvaluateTrack(uint32_t sampleCount, const int fs, const int pad) -> std::vector<double> {
  std::vector<double> trackAmp(sampleCount, 0); // intialiser à 0?
  for (haptics::types::Band &b : bands) {
    std::vector<double> bandAmp = b.EvaluationBand(sampleCount, fs, pad, b.getLowerFrequencyLimit(), b.getUpperFrequencyLimit());
    for (uint32_t i = 0; i < bandAmp.size(); i++){
      trackAmp[i] += bandAmp[i];
      if (trackAmp[i] < -1) {
        trackAmp[i] = -1;
      }
      if (trackAmp[i] > 1) {
        trackAmp[i] = 1;
      }
    }
  }
  return trackAmp;
}

[[nodiscard]] auto Track::getFrequencySampling() const -> std::optional<uint32_t> {
  return frequencySampling;
}

auto Track::setFrequencySampling(std::optional<uint32_t> newFrequencySampling) -> void {
  frequencySampling = newFrequencySampling;
}

[[nodiscard]] auto Track::getSampleCount() const -> std::optional<uint32_t> { return sampleCount; }

auto Track::setSampleCount(std::optional<uint32_t> newSampleCount) -> void {
  sampleCount = newSampleCount;
}

} // namespace haptics::types