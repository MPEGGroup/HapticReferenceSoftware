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
#include <Tools/include/Tools.h>
#include <Tools/include/WavParser.h>
#include <WaveletDecoder/include/WaveletDecoder.h>

using haptics::synthesizer::Helper;
using haptics::types::Haptics;
using haptics::waveletdecoder::WaveletDecoder;

namespace haptics::synthesizer {

[[nodiscard]] auto Helper::getTimeLength(types::Haptics &haptic) -> double {
  types::Perception perception;
  types::Track track;
  types::Band band;
  types::Effect effect;
  double maxLength = 0;
  double currentLength = 0;
  for (uint32_t perceptionIndex = 0; perceptionIndex < haptic.getPerceptionsSize();
       perceptionIndex++) {
    perception = haptic.getPerceptionAt((int)perceptionIndex);
    for (uint32_t trackIndex = 0; trackIndex < perception.getTracksSize(); trackIndex++) {
      track = perception.getTrackAt((int)trackIndex);
      if (track.getFrequencySampling().has_value() && track.getSampleCount().has_value()) {
        currentLength = S_2_MS * (static_cast<double>(track.getSampleCount().value()) /
                                  track.getFrequencySampling().value());
        if (currentLength > maxLength) {
          maxLength = currentLength;
        }
      } else {
        for (uint32_t bandIndex = 0; bandIndex < track.getBandsSize(); bandIndex++) {
          band = track.getBandAt((int)bandIndex);
          currentLength = band.getBandTimeLength();
          if (currentLength > maxLength) {
            maxLength = currentLength;
          }
        }
      }
    }
  }
  return maxLength;
}

[[nodiscard]] auto Helper::playFile(types::Haptics &haptic, const double timeLength, const int fs,
                                    const int pad, std::string &filename) -> bool {

  // Apply preprocessing on wavelet bands
  for (uint32_t i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (uint32_t j = 0; j < haptic.getPerceptionAt((int)i).getTracksSize(); j++) {
      for (uint32_t k = 0; k < haptic.getPerceptionAt((int)i).getTrackAt((int)j).getBandsSize();
           k++) {
        types::Band band = haptic.getPerceptionAt((int)i).getTrackAt((int)j).getBandAt((int)k);
        if (band.getBandType() == types::BandType::Wave) {
          if (band.getEncodingModality() == types::EncodingModality::Wavelet) {
            WaveletDecoder::transformBand(band);
            haptic.getPerceptionAt((int)i).getTrackAt((int)j).replaceBandAt((int)k, band);
          }
        }
      }
    }
  }
  std::vector<std::vector<double>> amplitudes;

  for (uint32_t i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (uint32_t j = 0; j < haptic.getPerceptionAt((int)i).getTracksSize(); j++) {
      types::Track myTrack;
      auto sampleCount = static_cast<uint32_t>(std::round(fs * MS_2_S * (timeLength + 2 * pad)));
      std::vector<double> trackAmp(sampleCount);
      myTrack = haptic.getPerceptionAt((int)i).getTrackAt((int)j);
      for (uint32_t k = 0; k < sampleCount; k++) {
        trackAmp[k] = myTrack.EvaluateTrack(sampleCount, fs, pad)[k] * myTrack.getGain();
      }
      amplitudes.push_back(trackAmp);
    }
  }
  return haptics::tools::WavParser::saveFile(filename, amplitudes, fs);
}
} // namespace haptics::synthesizer
