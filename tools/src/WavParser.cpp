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

#include "WavParser.h"

namespace haptics::tools {

WavParser::WavParser() {
    file.shouldLogErrorsToConsole(false);
}

bool WavParser::loadFile(std::string filename) {
    return file.load(filename);
}

bool WavParser::saveFile(std::string filename,std::vector<double> &buffer, int sampleRate) {
    file.setSampleRate(sampleRate);
    file.setAudioBufferSize(1,buffer.size());
    std::copy(file.samples.at(0).begin(),file.samples.at(0).end(),buffer.begin());
    return file.save(filename);
}

bool WavParser::saveFile(std::string filename,std::vector<std::vector<double>> &buffer, int sampleRate) {
    file.setSampleRate(sampleRate);
    file.setAudioBufferSize(buffer.size(),buffer.at(0).size());
    for(int i=0; i<buffer.size(); i++) {
        std::copy(file.samples.at(i).begin(),file.samples.at(i).end(),buffer.at(i).begin());
    }
    return file.save(filename);
}


uint32_t WavParser::getSamplerate() {
    return file.getSampleRate();
}

int WavParser::getNumChannels() {
    return file.getNumChannels();
}

int WavParser::getNumSamples() {
    return file.getNumSamplesPerChannel();
}

std::vector<double> WavParser::getSamples() {
    std::vector<double> buffer;
    buffer.resize(file.getNumSamplesPerChannel());
    std::copy(file.samples.at(0).begin(),file.samples.at(0).end(),buffer.begin());
    return buffer;
}

} // namespace haptics::tools
