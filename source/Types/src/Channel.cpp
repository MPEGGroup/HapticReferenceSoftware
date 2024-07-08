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

#include <Types/include/Channel.h>
#include <iostream>

namespace haptics::types {

[[nodiscard]] auto Channel::getId() const -> int { return id; }

auto Channel::setId(int newId) -> void { id = newId; }

[[nodiscard]] auto Channel::getDescription() const -> std::string { return description; }

auto Channel::setDescription(std::string &newDescription) -> void { description = newDescription; }

auto Channel::getPriority() const -> std::optional<int> { return priority; }
auto Channel::getPriorityOrDefault() const -> int {
  if (priority.has_value()) {
    return priority.value();
  }
  return 0;
}
auto Channel::setPriority(int newPriority) -> void { priority = newPriority; }

[[nodiscard]] auto Channel::getGain() const -> float { return gain; }

auto Channel::setGain(float newGain) -> void { gain = newGain; }

[[nodiscard]] auto Channel::getMixingWeight() const -> float { return mixingWeight; }

auto Channel::setMixingWeight(float newMixingWeight) -> void { mixingWeight = newMixingWeight; }

[[nodiscard]] auto Channel::getBodyPartMask() const -> uint32_t { return bodyPartMask; }

auto Channel::setBodyPartMask(uint32_t newBodyPartMask) -> void { bodyPartMask = newBodyPartMask; }

[[nodiscard]] auto Channel::getReferenceDeviceId() const -> std::optional<int> {
  return referenceDeviceId;
}
auto Channel::setReferenceDeviceId(int newReferenceDeviceId) -> void {
  referenceDeviceId = newReferenceDeviceId;
}

auto Channel::getVerticesSize() -> size_t { return vertices.size(); }

auto Channel::getVertexAt(int index) -> int & { return vertices.at(index); }

auto Channel::addVertex(int &newVertice) -> void { vertices.push_back(newVertice); }

auto Channel::getBandsSize() -> size_t { return bands.size(); }

auto Channel::getBandAt(int index) -> haptics::types::Band & { return bands.at(index); }

auto Channel::addBand(haptics::types::Band &newBand) -> void { bands.push_back(newBand); }

auto Channel::replaceBandAt(int index, haptics::types::Band &newBand) -> bool {
  if (index < 0 || index >= (int)this->getBandsSize()) {
    return false;
  }
  bands[index] = newBand;
  return true;
}

auto Channel::replaceBandMetadataAt(int index, haptics::types::Band &newBand) -> bool {
  if (index < 0 || index >= (int)this->getBandsSize()) {
    return false;
  }
  bands[index].setBandType(newBand.getBandType());
  if (bands[index].getBandType() == BandType::Curve) {
    bands[index].setCurveType(newBand.getCurveTypeOrDefault());
  }
  if (bands[index].getBandType() == BandType::WaveletWave) {
    bands[index].setBlockLength(newBand.getBlockLengthOrDefault());
  }
  bands[index].setLowerFrequencyLimit(newBand.getLowerFrequencyLimit());
  bands[index].setUpperFrequencyLimit(newBand.getUpperFrequencyLimit());
  // bands[index].setTimescale(newBand.getTimescale());
  return true;
}

auto Channel::removeBandAt(int index) -> bool {
  if (index < 0 || index >= (int)this->getBandsSize()) {
    return false;
  }
  this->bands.erase(this->bands.begin() + index);
  return true;
}

auto Channel::generateBand() -> haptics::types::Band * {
  Band newBand;
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Channel::generateBand(BandType bandType, int lowerFrequencyLimit, int upperFrequencyLimit)
    -> haptics::types::Band * {
  Band newBand(bandType, lowerFrequencyLimit, upperFrequencyLimit);
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Channel::generateBand(BandType bandType, CurveType curveType, int lowerFrequencyLimit,
                           int upperFrequencyLimit) -> haptics::types::Band * {
  Band newBand(bandType, curveType, lowerFrequencyLimit, upperFrequencyLimit);
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Channel::generateBand(BandType bandType, int blockLength, int lowerFrequencyLimit,
                           int upperFrequencyLimit) -> haptics::types::Band * {
  Band newBand(bandType, blockLength, lowerFrequencyLimit, upperFrequencyLimit);
  this->bands.push_back(newBand);
  return &this->bands.back();
}

auto Channel::findBandAvailable(const int position, const int duration,
                                const types::BandType bandType) -> haptics::types::Band * {
  haptics::types::Effect e;
  bool bandIsAvailable = true;
  for (haptics::types::Band &b : bands) {
    if (b.getBandType() != bandType) {
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

auto Channel::Evaluate(double position, unsigned int timescale) -> double {

  double res = 0;

  for (haptics::types::Band &b : bands) {
    res += b.Evaluate(position, b.getLowerFrequencyLimit(), b.getUpperFrequencyLimit(), timescale);
  }

  if (res < -1) {
    return -1;
  }
  if (res > 1) {
    return 1;
  }
  return res;
}

auto Channel::EvaluateChannel(uint32_t sampleCount, int fs, int pad, unsigned int timescale)
    -> std::vector<double> {
  std::vector<double> channelAmp(sampleCount, 0); // intialiser � 0?
  for (haptics::types::Band &b : bands) {
    std::vector<double> bandAmp = b.EvaluationBand(sampleCount, fs, pad, timescale);
    for (uint32_t i = 0; i < bandAmp.size(); i++) {
      channelAmp[i] += bandAmp[i];
      if (channelAmp[i] < -1) {
        channelAmp[i] = -1;
      }
      if (channelAmp[i] > 1) {
        channelAmp[i] = 1;
      }
    }
  }
  return channelAmp;
}

[[nodiscard]] auto Channel::getFrequencySampling() const -> std::optional<uint32_t> {
  return frequencySampling;
}

auto Channel::setFrequencySampling(std::optional<uint32_t> newFrequencySampling) -> void {
  frequencySampling = newFrequencySampling;
}

[[nodiscard]] auto Channel::getSampleCount() const -> std::optional<uint32_t> {
  return sampleCount;
}

auto Channel::setSampleCount(std::optional<uint32_t> newSampleCount) -> void {
  sampleCount = newSampleCount;
}

[[nodiscard]] auto Channel::getDirection() const -> std::optional<Vector> { return direction; }

auto Channel::setDirection(std::optional<Vector> newDirection) -> void { direction = newDirection; }

[[nodiscard]] auto Channel::getActuatorResolution() const -> std::optional<Vector> {
  return actuatorResolution;
}

auto Channel::setActuatorResolution(std::optional<Vector> newChannelResolution) -> void {
  actuatorResolution = newChannelResolution;
}

[[nodiscard]] auto Channel::getBodyPartTarget() const
    -> std::optional<std::vector<BodyPartTarget>> {
  return bodyPartTarget;
}

auto Channel::setBodyPartTarget(std::optional<std::vector<BodyPartTarget>> newBodyPartTarget)
    -> void {
  bodyPartTarget = std::move(newBodyPartTarget);
}

[[nodiscard]] auto Channel::getActuatorTarget() const -> std::optional<std::vector<Vector>> {
  return actuatorTarget;
}

auto Channel::setActuatorTarget(std::optional<std::vector<Vector>> newActuatorTarget) -> void {
  actuatorTarget = std::move(newActuatorTarget);
}

auto Channel::equals(const Channel &channel) const -> bool {
  if (id != channel.getId()) {
    std::cerr << "Channel id fields are different" << std::endl;
    return false;
  }
  if (description != channel.getDescription()) {
    std::cerr << "Description fields are different" << std::endl;
    return false;
  }
  if (gain != channel.getGain()) {
    std::cerr << "Gain fields are different" << std::endl;
    return false;
  }
  if (bodyPartMask != channel.getBodyPartMask()) {
    std::cerr << "channel id fields are different" << std::endl;
    return false;
  }
  if (priority != channel.getPriority()) {
    std::cerr << "bodyPartMask fields are different" << std::endl;
    return false;
  }
  if (referenceDeviceId != channel.getReferenceDeviceId()) {
    std::cerr << "Reference device id fields are different" << std::endl;
    return false;
  }
  if (frequencySampling != channel.getFrequencySampling()) {
    std::cerr << "Frequency Sampling fields are different" << std::endl;
    return false;
  }
  if (sampleCount != channel.getSampleCount()) {
    std::cerr << "Sample Count fields are different" << std::endl;
    return false;
  }
  if (direction != channel.getDirection()) {
    std::cerr << "Direction fields are different" << std::endl;
    return false;
  }
  if (actuatorResolution != channel.getActuatorResolution()) {
    std::cerr << "Actuator Resolution fields are different" << std::endl;
    return false;
  }
  if (vertices.size() != channel.vertices.size()) {
    std::cerr << "The number of vertices in channel " << id << " is different" << std::endl;
    return false;
  }
  if (bands.size() != channel.bands.size()) {
    std::cerr << "The number of bands in channel " << id << " is different" << std::endl;
    return false;
  }
  if (bodyPartTarget.has_value() != channel.bodyPartTarget.has_value()) {
    std::cerr << "bodyPartTarget fields are different" << std::endl;
    return false;
  }
  if (bodyPartTarget.has_value() &&
      (bodyPartTarget.value().size() != channel.bodyPartTarget.value().size())) {
    std::cerr << "bodyPartTarget fields are different" << std::endl;
    return false;
  }
  if (actuatorTarget.has_value() != channel.actuatorTarget.has_value()) {
    std::cerr << "actuatorTarget fields are different" << std::endl;
    return false;
  }
  if (actuatorTarget.has_value() &&
      (actuatorTarget.value().size() != channel.actuatorTarget.value().size())) {
    std::cerr << "actuatorTarget fields are different" << std::endl;
    return false;
  }

  bool isEqual = true;
  for (int i = 0; i < static_cast<int> (vertices.size())-*; i++) {
    isEqual = isEqual && (vertices.at(i) == channel.vertices.at(i));
  }
  if (!isEqual) {
    std::cerr << "vertices fields are different" << std::endl;
    return false;
  }

  for (int i = 0; i < static_cast<int> (bands.size()); i++) {
    const auto band1 = bands.at(i);
    const auto band2 = channel.bands.at(i);
    isEqual = isEqual && (band1.equals(band2));
  }
  if (!isEqual) {
    return false;
  }
  if (bodyPartTarget.has_value()) {
    for (int i = 0; i < static_cast<int> (bodyPartTarget.value().size()); i++) {
      const auto bodyPartTarget1 = bodyPartTarget.value().at(i);
      const auto bodyPartTarget2 = channel.bodyPartTarget.value().at(i);
      isEqual = isEqual && (bodyPartTarget1 == bodyPartTarget2);
    }
    if (!isEqual) {
      std::cerr << "bodyPartTarget fields are different" << std::endl;
      return false;
    }
  }

  if (actuatorTarget.has_value()) {
    for (int i = 0; i < static_cast<int>(actuatorTarget.value().size()); i++) {
      const auto actuatorTarget1 = actuatorTarget.value().at(i);
      const auto actuatorTarget2 = channel.actuatorTarget.value().at(i);
      isEqual = isEqual && (actuatorTarget1 == actuatorTarget2);
    }
    if (!isEqual) {
      std::cerr << "actuatorTarget fields are different" << std::endl;
      return false;
    }
  }
  return true;
}

} // namespace haptics::types
