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

#include <Synthesizer/include/Helper.h>

using haptics::synthesizer::Helper;
using haptics::types::Haptics;

namespace haptics::synthesizer {

[[nodiscard]] auto Helper::getTimeLength(types::Haptics &haptic) -> double {
  types::Perception perception;
  types::Track track;
  types::Band band;
  types::Effect effect;
  double maxLength = 0;
  double currentLength = 0;
  for (int perceptionIndex = 0; perceptionIndex < haptic.getPerceptionsSize(); perceptionIndex++) {
    perception = haptic.getPerceptionAt(perceptionIndex);
    for (int trackIndex = 0; trackIndex < perception.getTracksSize(); trackIndex++) {
      track = perception.getTrackAt(trackIndex);
      for (int bandIndex = 0; bandIndex < track.getBandsSize(); bandIndex++) {
        band = track.getBandAt(bandIndex);
        if (band.getEffectsSize() == 0) {
          continue;
        }
        effect = band.getEffectAt(static_cast<int>(band.getEffectsSize()) - 1);
        currentLength = Helper::getEffectTimeLength(
            effect, band.getBandType(), band.getEncodingModality(), band.getWindowLength());
        if (currentLength > maxLength) {
          maxLength = currentLength;
        }
      }
    }
  }
  return maxLength;
}

[[nodiscard]] auto Helper::getEffectTimeLength(types::Effect &effect, types::BandType bandType,
                                               types::EncodingModality encodingModality,
                                               int windowLength) -> double {

  double length = effect.getPosition();
  if (effect.getKeyframesSize() == 0) {
    return length;
  }

  types::Keyframe lastKeyframe =
      effect.getKeyframeAt(static_cast<int>(effect.getKeyframesSize()) - 1);
  switch (bandType) {
  case types::BandType::Transient:
    length += lastKeyframe.getRelativePosition();
    break;
  case types::BandType::Curve:
    length += lastKeyframe.getRelativePosition();
    break;
  case types::BandType::Wave:
    switch (encodingModality) {
    case types::EncodingModality::Quantized:
      length += effect.getKeyframesSize() * windowLength;
      break;
    case types::EncodingModality::Vectorial:
      length += lastKeyframe.getRelativePosition();
      break;
    default:
      break;
    }
  default:
    break;
  }
  return length;
}

[[nodiscard]] auto Helper::playFile(types::Haptics &haptic, const double timeLength) -> bool {
  return false;
}

}