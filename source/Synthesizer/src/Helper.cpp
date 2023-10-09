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
  types::Channel channel;
  types::Band band;
  types::Effect effect;
  double maxLength = 0;
  double currentLength = 0;
  unsigned int timescale = haptic.getTimescale().value();
  for (uint32_t perceptionIndex = 0; perceptionIndex < haptic.getPerceptionsSize();
       perceptionIndex++) {
    perception = haptic.getPerceptionAt((int)perceptionIndex);
    for (uint32_t channelIndex = 0; channelIndex < perception.getChannelsSize(); channelIndex++) {
      channel = perception.getChannelAt((int)channelIndex);
      if (channel.getFrequencySampling().has_value() && channel.getSampleCount().has_value()) {
        currentLength = static_cast<double>(timescale) *
                        (static_cast<double>(channel.getSampleCount().value()) /
                         channel.getFrequencySampling().value());
        if (currentLength > maxLength) {
          maxLength = currentLength;
        }
      } else {
        for (uint32_t bandIndex = 0; bandIndex < channel.getBandsSize(); bandIndex++) {
          band = channel.getBandAt((int)bandIndex);
          currentLength = band.getBandTimeLength(timescale);
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
    for (uint32_t j = 0; j < haptic.getPerceptionAt((int)i).getChannelsSize(); j++) {
      for (uint32_t k = 0; k < haptic.getPerceptionAt((int)i).getChannelAt((int)j).getBandsSize();
           k++) {
        types::Band band = haptic.getPerceptionAt((int)i).getChannelAt((int)j).getBandAt((int)k);
        if (band.getBandType() == types::BandType::WaveletWave) {
          WaveletDecoder::transformBand(band, haptic.getTimescaleOrDefault());
          haptic.getPerceptionAt((int)i).getChannelAt((int)j).replaceBandAt((int)k, band);
        }
      }
    }
  }
  std::vector<std::vector<double>> amplitudes;

  unsigned int timescale = haptic.getTimescale().value();
  for (uint32_t i = 0; i < haptic.getPerceptionsSize(); i++) {
    for (uint32_t j = 0; j < haptic.getPerceptionAt((int)i).getChannelsSize(); j++) {
      types::Channel myChannel;
      auto sampleCount = static_cast<uint32_t>(std::round(fs * (timeLength + 2 * pad) / timescale));
      myChannel = haptic.getPerceptionAt((int)i).getChannelAt((int)j);
      std::vector<double> channelAmp = myChannel.EvaluateChannel(sampleCount, fs, pad, timescale);
      const double perceptionUnitFactor =
          std::pow(10.0, haptic.getPerceptionAt((int)i).getPerceptionUnitExponentOrDefault());
      for (uint32_t k = 0; k < sampleCount; k++) {
        channelAmp[k] = channelAmp[k] * myChannel.getGain() * perceptionUnitFactor;
      }
      amplitudes.push_back(channelAmp);
    }
  }
  return haptics::tools::WavParser::saveFile(filename, amplitudes, fs);
}
} // namespace haptics::synthesizer
