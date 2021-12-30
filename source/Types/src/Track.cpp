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

  [[nodiscard]] auto Track::getId() const -> int {
    return id;
  }

  auto Track::setId(int newId) -> void {
    id = newId;
  }

  [[nodiscard]] auto Track::getDescription() const -> std::string {
    return description;
  }

  auto Track::setDescription(std::string &newDescription) -> void {
    description = newDescription;
  }

  [[nodiscard]] auto Track::getGain() const -> float {
    return gain;
  }

  auto Track::setGain(float newGain) -> void {
    gain = newGain;
  }

  [[nodiscard]] auto Track::getMixingWeight() const -> float {
    return mixingWeight;
  }

  auto Track::setMixingWeight(float newMixingWeight) -> void {
    mixingWeight = newMixingWeight;
  }

  [[nodiscard]] auto Track::getBodyPartMask() const -> uint32_t {
    return bodyPartMask;
  }

  auto Track::setBodyPartMask(uint32_t newBodyPartMask) -> void {
    bodyPartMask = newBodyPartMask;
  }

  auto Track::getVerticesSize() -> size_t {
    return vertices.size();
  }

  auto Track::getVertexAt(int index) -> int& {
    return vertices.at(index);
  }

  auto Track::addVertex(int& newVertice) -> void {
    vertices.push_back(newVertice);
  }

  auto Track::getBandsSize() -> size_t {
    return bands.size();
  }

  auto Track::getBandAt(int index) -> haptics::types::Band& {
    return bands.at(index);
  }

  auto Track::addBand(haptics::types::Band& newBand) -> void {
    bands.push_back(newBand);
  }

  auto Track::findWaveBandAvailable(const int position, const int duration) -> haptics::types::Band * {
    haptics::types::Effect e;
    int start = 0;
    int stop = 0;
    bool bandIsAvailable = true;
    for (haptics::types::Band &b : bands) {
      if (b.getBandType() != haptics::types::BandType::Wave) {
        continue;
      }

      bandIsAvailable = true;
      for (int i = 0; i < b.getEffectsSize(); i++) {
        e = b.getEffectAt(i);
        if (b.isOverlapping(e, position, position+duration)) {
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

    for (haptics::types::Band b : bands) {
      res += b.Evaluate(position);
    }

    return res;
  }

} // namespace haptics::types