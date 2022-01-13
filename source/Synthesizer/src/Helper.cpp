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
  for (int perceptionIndex = 0; perceptionIndex < haptic.getPerceptionsSize(); perceptionIndex++) {
    perception = haptic.getPerceptionAt(perceptionIndex);
    for (int trackIndex = 0; trackIndex < perception.getTracksSize(); trackIndex++) {
      track = perception.getTrackAt(trackIndex);
      for (int bandIndex = 0; bandIndex < track.getBandsSize(); bandIndex++) {
        band = track.getBandAt(bandIndex);
        currentLength = band.getBandTimeLength();
        if (currentLength > maxLength) {
          maxLength = currentLength;
        }
      }
    }
  }
  return maxLength;
}

[[nodiscard]] auto Helper::playFile(types::Haptics &haptic, const double timeLength, const int fs,
                                    const int pad, std::string &filename) -> bool {

  // Apply preprocessing on wavelet bands
  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (int j = 0; j < haptic.getPerceptionAt(i).getTracksSize(); j++) {
      for (int k = 0; k < haptic.getPerceptionAt(i).getTrackAt(j).getBandsSize(); k++) {
        types::Band band = haptic.getPerceptionAt(i).getTrackAt(j).getBandAt(k);
        if (band.getBandType() == types::BandType::Wave) {
          if (band.getEncodingModality() == types::EncodingModality::Wavelet) {
            WaveletDecoder::transformBand(band);
            haptic.getPerceptionAt(i).getTrackAt(j).replaceBandAt(k, band);
          }
        }
      }
    }
  }
  std::vector<std::vector<double>> amplitudes;

  int index = 0;

  for (int i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (int j = 0; j < haptic.getPerceptionAt(i).getTracksSize(); j++) {
      double t = 0 - pad * MS_2_S;
      std::vector<double> trackAmp;
      while (t < ((timeLength + pad) * MS_2_S)) {
        double amp = haptic.getPerceptionAt(i).getTrackAt(j).Evaluate(t * S_2_MS);
        trackAmp.push_back(amp);
        t += 1.0 / static_cast<double>(fs);
      }
      amplitudes.push_back(trackAmp);
    }
  }

  return haptics::tools::WavParser::saveFile(filename, amplitudes, fs);
}
} // namespace haptics::synthesizer
