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

#include "../include/Note.h"

namespace haptics::types {

[[nodiscard]] auto Note::getPosition() const -> int {
  return position;
}

auto Note::setPosition(int newPosition) -> void {
  position = newPosition;
}

[[nodiscard]] auto Note::getPhase() const -> float {
  return phase;
}

auto Note::setPhase(float newPhase) -> void {
  phase = newPhase;
}

[[nodiscard]] auto Note::getBaseSignal() const -> BaseSignal {
  return baseSignal;
}

auto Note::setBaseSignal(BaseSignal newBaseSignal) -> void {
  baseSignal = newBaseSignal;
}

auto Note::getKeyframesSize() -> size_t {
  return keyframes.size();
}

auto Note::getKeyframeAt(int index) -> haptics::types::Keyframe& {
  return keyframes.at(index);
}

auto Note::addKeyframe(haptics::types::Keyframe& newKeyframe) -> void {
  keyframes.push_back(newKeyframe);
}

} // namespace haptics::types