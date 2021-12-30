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

#include <catch2/catch.hpp>

#include <iostream>
#include <vector>

#include "../include/WaveletEncoder.h"

constexpr int val = 3;
constexpr int bits = 3;
constexpr double positive = 3;
constexpr double negative = -3;
constexpr size_t pos = 5;
constexpr size_t size = 10;

constexpr double unquantized = 0.7499;
constexpr double quantized = 0.75000000;

constexpr size_t bl_test = 512;
constexpr int fs_test = 8000;
constexpr size_t book_size = 8;

TEST_CASE("haptics::encoder::WaveletEncoder,1") {

    using haptics::encoder::WaveletEncoder;

    SECTION("Encoder tools") {

        std::vector<unsigned char> outstream(1,'0');
        WaveletEncoder::de2bi(val,outstream,bits);
        CHECK(outstream.size() == bits+1);
        CHECK(outstream[1] == 1);
        CHECK(outstream[2] == 1);
        CHECK(outstream[3] == 0);

        auto sign = WaveletEncoder::sgn(positive);
        CHECK(sign == 1);
        sign = WaveletEncoder::sgn(negative);
        CHECK(sign == -1);

    }

}

TEST_CASE("haptics::encoder::WaveletEncoder,2") {

    using haptics::encoder::WaveletEncoder;

    SECTION("Encoder tools") {

        std::vector<double> data(size,1);
        data[pos] = negative;
        size_t pos_found = WaveletEncoder::findMinInd(data);
        CHECK(pos_found == pos);

        std::vector<double> data2(size,1);
        data2[pos] = positive;
        double max = WaveletEncoder::findMax(data2);
        CHECK(max == positive);

        double quant = WaveletEncoder::maxQuant(unquantized,0,FRACTIONBITS_0);
        CHECK(quant == quantized);
        quant = WaveletEncoder::maxQuant(unquantized+1,3,4);
        CHECK(quant == quantized+1);

        std::vector<double> v_unquantized(3,unquantized);
        std::vector<double> v_quantized(3,0);
        WaveletEncoder::uniformQuant(v_unquantized,v_quantized,1,1,1,bits);
        CHECK(v_quantized[0] == 0);
        CHECK(v_quantized[1] == quantized);
        CHECK(v_quantized[2] == 0);

    }

}

TEST_CASE("haptics::encoder::WaveletEncoder,3") {

    using haptics::encoder::WaveletEncoder;

    SECTION("Encoder tools") {

        std::vector<double> v_unquantized(3,unquantized);
        double qwavmax = 0;
        std::vector<unsigned char> bitwavmax;
        std::vector<unsigned char> bitwavmax_compare = {0,0,0,0,0,0,1,1};
        WaveletEncoder::maximumWaveletCoefficient(v_unquantized,qwavmax,bitwavmax);
        CHECK(qwavmax == quantized);
        CHECK(std::equal(bitwavmax.begin(),bitwavmax.end(),bitwavmax_compare.begin()));

        /*for(size_t i=0; i<WAVMAXLENGTH; i++){
            std::cout << (int)bitwavmax[i] << std::endl;
        }*/

    }

    SECTION("Encoder") {

        std::vector<double> data_time(bl_test,0);
        data_time[0] = 1;
        WaveletEncoder waveletEncoder(bl_test,fs_test);
        std::vector<double> data_quant = waveletEncoder.encodeBlock(data_time,1);

    }

}