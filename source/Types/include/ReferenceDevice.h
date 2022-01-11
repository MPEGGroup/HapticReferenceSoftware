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

#ifndef REFERENCEDEVICE_H
#define REFERENCEDEVICE_H

#include <optional>
#include <string>

namespace haptics::types {

class ReferenceDevice {
public:
  explicit ReferenceDevice() = default;
  explicit ReferenceDevice(int newId, std::string newName) : id(newId), name(std::move(newName)){};

  [[nodiscard]] auto getId() const -> int;
  auto setId(int newId) -> void;
  [[nodiscard]] auto getName() const -> std::string;
  auto setName(std::string &newName) -> void;
  [[nodiscard]] auto getBodyPartMask() const -> std::optional<uint32_t>;
  auto setBodyPartMask(uint32_t newBodyPartMask) -> void;
  [[nodiscard]] auto getMaximumFrequency() const -> std::optional<float>;
  auto setMaximumFrequency(float newMaximumFrequency) -> void;
  [[nodiscard]] auto getMinimumFrequency() const -> std::optional<float>;
  auto setMinimumFrequency(float newMinimumFrequency) -> void;
  [[nodiscard]] auto getResonanceFrequency() const -> std::optional<float>;
  auto setResonanceFrequency(float newResonanceFrequency) -> void;
  [[nodiscard]] auto getMaximumAmplitude() const -> std::optional<float>;
  auto setMaximumAmplitude(float newMaximumAmplitude) -> void;
  [[nodiscard]] auto getImpedance() const -> std::optional<float>;
  auto setImpedance(float newImpedance) -> void;
  [[nodiscard]] auto getMaximumVoltage() const -> std::optional<float>;
  auto setMaximumVoltage(float newMaximumVoltage) -> void;
  [[nodiscard]] auto getMaximumCurrent() const -> std::optional<float>;
  auto setMaximumCurrent(float newMaximumCurrent) -> void;
  [[nodiscard]] auto getMaximumDisplacement() const -> std::optional<float>;
  auto setMaximumDisplacement(float newMaximumDisplacement) -> void;
  [[nodiscard]] auto getWeight() const -> std::optional<float>;
  auto setWeight(float newWeight) -> void;
  [[nodiscard]] auto getSize() const -> std::optional<float>;
  auto setSize(float newSize) -> void;

private:
  int id = -1;
  std::string name;
  std::optional<uint32_t> bodyPartMask;
  std::optional<float> maximumFrequency;
  std::optional<float> minimumFrequency;
  std::optional<float> resonanceFrequency;
  std::optional<float> maximumAmplitude;
  std::optional<float> impedance;
  std::optional<float> maximumVoltage;
  std::optional<float> maximumCurrent;
  std::optional<float> maximumDisplacement;
  std::optional<float> weight;
  std::optional<float> size;
};
} // namespace haptics::types
#endif // REFERENCEDEVICE_H
