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

#include <WavParser.h>
#include <algorithm>
#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace haptics::tools {

auto WavParser::loadFile(const std::string &filename) -> bool {

  drwav wav;
  if (!(bool)drwav_init_file(&wav, filename.c_str(), nullptr)) {
    return false;
  }

  numSamples = (size_t)wav.totalPCMFrameCount;
  numChannels = (int)wav.channels;
  auto samplesPerChannel = numSamples / (size_t)numChannels;
  buffer.clear();
  buffer.reserve(numChannels);
  for (int c = 0; c < numChannels; c++) {
    std::vector<float> b;
    b.resize(numSamples);
    drwav_read_pcm_frames_f32(&wav, samplesPerChannel, b.data());
    std::vector<double> b_double;
    b_double.resize(samplesPerChannel);
    std::transform(b.begin(), b.end(), b_double.begin(),
                   [](float v) -> double { return (double)v; });
    buffer.push_back(b_double);
  }

  return true;
}

auto WavParser::saveFile(std::string &filename, std::vector<double> &buffer, int sampleRate)
    -> bool {

  drwav wav;
  drwav_data_format format;
  format.container = drwav_container_riff;
  format.format = DR_WAVE_FORMAT_PCM;
  format.channels = 1;
  format.sampleRate = sampleRate;
  format.bitsPerSample = BITS_PER_SAMPLE;
  drwav_init_file_write(&wav, filename.c_str(), &format, nullptr);

  drwav_write_pcm_frames(&wav, buffer.size(), buffer.data());

  return true;
}

auto WavParser::saveFile(std::string &filename, std::vector<std::vector<double>> &buffer,
                         int sampleRate) -> bool {
  drwav wav;
  drwav_data_format format;
  format.container = drwav_container_riff;
  format.format = DR_WAVE_FORMAT_PCM;
  format.channels = buffer.size();
  format.sampleRate = sampleRate;
  format.bitsPerSample = BITS_PER_SAMPLE;
  drwav_init_file_write(&wav, filename.c_str(), &format, nullptr);

  for (auto &b : buffer) {
    drwav_write_pcm_frames(&wav, b.size(), b.data());
  }

  return true;
}

auto WavParser::getSamplerate() const -> uint32_t { return sampleRate; }

auto WavParser::getNumChannels() const -> size_t { return numChannels; }

auto WavParser::getNumSamples() const -> size_t { return numSamples; }

auto WavParser::getSamplesChannel(size_t channel) const -> std::vector<double> {
  return buffer.at(channel);
}

auto WavParser::getAllSamples() const -> std::vector<std::vector<double>> { return buffer; }

} // namespace haptics::tools
