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

#ifndef WAVELETENCODER_H
#define WAVELETENCODER_H

#include <vector>
#include <cmath>

#include "PsychohapticModel/include/PsychohapticModel.h"
#include "FilterBank/include/Wavelet.h"
#include "Types/include/Band.h"
#include "Types/include/Effect.h"
#include "Types/include/Keyframe.h"

constexpr size_t WAVMAXLENGTH = 8;
constexpr int MAXBITS = 15;
constexpr int FRACTIONBITS_0 = 7;
constexpr double LOGFACTOR = 10;
constexpr double MAXQUANTFACTOR = 0.999;
constexpr double QUANT_ADD = 0.5;

namespace haptics::encoder {

struct quantMode {
  int integerbits;
  int fractionbits;
};

class WaveletEncoder {
public:
    WaveletEncoder(int bl_new, int fs_new);

    void encodeSignal(std::vector<double> &sig_time, int bitbudget);
    auto encodeBlock(std::vector<double> &block_time, int bitbudget) -> std::vector<double>;
    static void maximumWaveletCoefficient(std::vector<double> &sig, double &qwavmax,
                                          std::vector<unsigned char> &bitwavmax);
    void updateNoise(std::vector<double> &bandenergy, std::vector<double> &noiseenergy,
                     std::vector<double> &SNR, std::vector<double> &MNR, std::vector<double> &SMR);

    static void uniformQuant(std::vector<double> &in, size_t start, double max, int bits,
                             size_t length, std::vector<double> &out);
    static auto maxQuant(double in, quantMode m) -> double;
    template <class T>
    static auto findMax(std::vector<T> &data) -> T;
    static auto findMinInd(std::vector<double> &data) -> size_t;

    static auto sgn(double val) -> double;
    static void de2bi(int val, std::vector<unsigned char> &outstream, int length);

private:

    int bl;
    int fs;
    int dwtlevel;
    std::vector<int> book;
    std::vector<int> book_cumulative;

    tools::PsychohapticModel pm;

};
} // namespace haptics::encoder
#endif //WAVELETENCODER_H
