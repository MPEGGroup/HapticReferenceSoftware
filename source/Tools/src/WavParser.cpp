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

#include <Tools/include/WavParser.h>
#include <algorithm>
#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace haptics::tools {

auto WavParser::loadFile(const std::string &filename) -> bool {

  drwav wav;
  if (!(bool)drwav_init_file(&wav, filename.c_str(), nullptr)) {
    return false;
  }

  auto samplesPerChannel = (size_t)wav.totalPCMFrameCount;
  numChannels = (size_t)wav.channels;

  numSamples = samplesPerChannel * numChannels;
  sampleRate = static_cast<int>(wav.sampleRate);
  buffer.clear();
  buffer.reserve(numChannels);
  std::vector<float> b;
  b.resize(numSamples);
  drwav_read_pcm_frames_f32(&wav, samplesPerChannel, b.data());
  



  for (size_t c = 0; c < numChannels; c++) {
    std::vector<double> b_double;
    b_double.reserve(samplesPerChannel);
    buffer.push_back(b_double);
  }

  for (auto it = b.begin(); it < b.end(); it++) {
    auto diff = it - b.begin();
    auto c = diff % (long)numChannels;
    buffer.at(c).push_back(*it);
  }

  return true;
}

auto WavParser::saveFile(std::string &filename, std::vector<double> &buff, int sampleRate) -> bool {
  drwav wav;
  drwav_data_format format;
  format.container = drwav_container_riff;
  format.format = DR_WAVE_FORMAT_PCM;
  format.channels = 1;
  format.sampleRate = sampleRate;
  format.bitsPerSample = BITS_PER_SAMPLE;
  drwav_init_file_write(&wav, filename.c_str(), &format, nullptr);
  std::vector<uint16_t> b_int;
  b_int.resize(buff.size());
  std::transform(buff.begin(), buff.end(), b_int.begin(),
                 [](double v) -> uint16_t { return (uint16_t)(round((v)*SCALING)); });
  drwav_write_pcm_frames(&wav, b_int.size(), b_int.data());
  drwav_uninit(&wav);
  return true;
}

auto WavParser::saveFile(std::string &filename, std::vector<std::vector<double>> &buff,
                         int sampleRate) -> bool {
  size_t s = buff.at(0).size();
  bool sameSize = true;
  for (int i = 1; i < buff.size(); i++) {
    if (buff.at(i).size() != s) {
      std::cout << "Output Tracks are of different size" << std::endl;
      return false;
    }
  }
  drwav wav;
  drwav_data_format format;
  format.container = drwav_container_riff;
  format.format = DR_WAVE_FORMAT_PCM;
  format.channels = buff.size();
  format.sampleRate = sampleRate;
  format.bitsPerSample = BITS_PER_SAMPLE;
  drwav_init_file_write(&wav, filename.c_str(), &format, nullptr);

  std::vector<uint16_t> b_int;
  b_int.resize(buff.size() * buff.at(0).size());
  long c = 0;
  for (auto &b : buff) {
    for (int i = 0; i<b.size(); i++) {
      b_int.at((i * buff.size()) + c) = (uint16_t)(round((b.at(i) * SCALING)));
    }
    c++;
  }

  drwav_write_pcm_frames(&wav, b_int.size() / buff.size(), b_int.data());
  drwav_uninit(&wav);
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
