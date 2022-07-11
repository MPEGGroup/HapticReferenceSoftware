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

#ifndef TRACK_H
#define TRACK_H

#include <Types/include/Band.h>
#include <fstream>
#include <vector>

namespace haptics::types {

class Track {
public:
  explicit Track() = default;
  explicit Track(int newId, std::string newDescription, float newGain, float newMixingWeight,
                 uint32_t newBodyPartMask)
      : id(newId)
      , description(std::move(newDescription))
      , gain(newGain)
      , mixingWeight(newMixingWeight)
      , bodyPartMask(newBodyPartMask)
      , vertices({})
      , bands({})
      , frequencySampling(std::nullopt)
      , sampleCount(std::nullopt){};

  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getDescription() const -> std::string;
  auto setDescription(std::string &newDescription) -> void;
  [[nodiscard]] auto getGain() const -> float;
  auto setGain(float newGain) -> void;
  [[nodiscard]] auto getMixingWeight() const -> float;
  auto setMixingWeight(float newMixingWeight) -> void;
  [[nodiscard]] auto getBodyPartMask() const -> uint32_t;
  auto setBodyPartMask(uint32_t newBodyPartMask) -> void;
  [[nodiscard]] auto getReferenceDeviceId() const -> std::optional<int>;
  auto setReferenceDeviceId(int newReferenceDeviceId) -> void;
  auto getVerticesSize() -> size_t;
  auto getVertexAt(int index) -> int &;
  auto addVertex(int &newVertice) -> void;
  auto getBandsSize() -> size_t;
  auto getBandAt(int index) -> haptics::types::Band &;
  auto replaceBandAt(int index, haptics::types::Band &newBand) -> bool;
  auto addBand(haptics::types::Band &newBand) -> void;
  auto generateBand() -> haptics::types::Band *;
  auto generateBand(BandType bandType, CurveType curveType, EncodingModality encodingModality,
                    int windowLength, int lowerFrequencyLimit, int upperFrequencyLimit)
      -> haptics::types::Band *;
  auto findBandAvailable(int position, int duration, types::BandType bandType,
                         types::EncodingModality encodingModality) -> haptics::types::Band *;
  auto Evaluate(double position) -> double;
  auto EvaluateTrack(uint32_t sampleCount, const int fs, const int pad) -> std::vector<double>;
  [[nodiscard]] auto getFrequencySampling() const -> std::optional<uint32_t>;
  auto setFrequencySampling(std::optional<uint32_t> newFrequencySampling) -> void;
  [[nodiscard]] auto getSampleCount() const -> std::optional<uint32_t>;
  auto setSampleCount(std::optional<uint32_t> newSampleCount) -> void;

private:
  int id = -1;
  std::string description;
  float gain = 1;
  float mixingWeight = 1;
  uint32_t bodyPartMask = 0;
  std::vector<int> vertices = {};
  std::vector<Band> bands = {};
  std::optional<int> referenceDeviceId;
  std::optional<uint32_t> frequencySampling = std::nullopt;
  std::optional<uint32_t> sampleCount = std::nullopt;
};
} // namespace haptics::types
#endif // TRACK_H
