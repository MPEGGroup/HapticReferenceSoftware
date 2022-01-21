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

#include <Types/include/ReferenceDevice.h>

namespace haptics::types {

[[nodiscard]] auto ReferenceDevice::getId() const -> int { return id; }
auto ReferenceDevice::setId(int newId) -> void { id = newId; }

[[nodiscard]] auto ReferenceDevice::getName() const -> std::string { return name; }

auto ReferenceDevice::setName(std::string &newName) -> void { name = newName; }

[[nodiscard]] auto ReferenceDevice::getBodyPartMask() const -> std::optional<uint32_t> {
  return bodyPartMask;
}

auto ReferenceDevice::setBodyPartMask(uint32_t newBodyPartMask) -> void {
  bodyPartMask = newBodyPartMask;
}

[[nodiscard]] auto ReferenceDevice::getMaximumFrequency() const -> std::optional<float> {
  return maximumFrequency;
}

auto ReferenceDevice::setMaximumFrequency(float newMaximumFrequency) -> void {
  maximumFrequency = newMaximumFrequency;
}

[[nodiscard]] auto ReferenceDevice::getMinimumFrequency() const -> std::optional<float> {
  return minimumFrequency;
}
auto ReferenceDevice::setMinimumFrequency(float newMinimumFrequency) -> void {
  minimumFrequency = newMinimumFrequency;
}
[[nodiscard]] auto ReferenceDevice::getResonanceFrequency() const -> std::optional<float> {
  return resonanceFrequency;
}
auto ReferenceDevice::setResonanceFrequency(float newResonanceFrequency) -> void {
  resonanceFrequency = newResonanceFrequency;
}

[[nodiscard]] auto ReferenceDevice::getMaximumAmplitude() const -> std::optional<float> {
  return maximumAmplitude;
}

auto ReferenceDevice::setMaximumAmplitude(float newMaximumAmplitude) -> void {
  maximumAmplitude = newMaximumAmplitude;
}

[[nodiscard]] auto ReferenceDevice::getImpedance() const -> std::optional<float> {
  return impedance;
}

auto ReferenceDevice::setImpedance(float newImpedance) -> void { impedance = newImpedance; }

[[nodiscard]] auto ReferenceDevice::getMaximumVoltage() const -> std::optional<float> {
  return maximumVoltage;
}

auto ReferenceDevice::setMaximumVoltage(float newMaximumVoltage) -> void {
  maximumVoltage = newMaximumVoltage;
}

[[nodiscard]] auto ReferenceDevice::getMaximumCurrent() const -> std::optional<float> {
  return maximumCurrent;
}

auto ReferenceDevice::setMaximumCurrent(float newMaximumCurrent) -> void {
  maximumCurrent = newMaximumCurrent;
}

[[nodiscard]] auto ReferenceDevice::getMaximumDisplacement() const -> std::optional<float> {
  return maximumDisplacement;
}

auto ReferenceDevice::setMaximumDisplacement(float newMaximumDisplacement) -> void {
  maximumDisplacement = newMaximumDisplacement;
}

[[nodiscard]] auto ReferenceDevice::getWeight() const -> std::optional<float> { return weight; }

auto ReferenceDevice::setWeight(float newWeight) -> void { weight = newWeight; }

[[nodiscard]] auto ReferenceDevice::getSize() const -> std::optional<float> { return size; }

auto ReferenceDevice::setSize(float newSize) -> void { size = newSize; }

[[nodiscard]] auto ReferenceDevice::getCustom() const -> std::optional<float> { return custom; }

auto ReferenceDevice::setCustom(float newCustom) -> void { custom = newCustom; }

[[nodiscard]] auto ReferenceDevice::getType() const -> std::optional<ActuatorType> { return type; }

auto ReferenceDevice::setType(ActuatorType newType) -> void { type = newType; }

} // namespace haptics::types