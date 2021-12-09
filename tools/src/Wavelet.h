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

#ifndef _WAVELET_H_
#define _WAVELET_H_

#include <iostream>
#include <vector>
#include <math.h>

namespace haptics::tools {

class Wavelet {
public:

    void DWT(std::vector<double> &in, std::vector<double> &out, int levels);
    void inv_DWT(std::vector<double> &in, std::vector<double> &out, int levels);

    static void symconv1D(std::vector<double> &in, std::vector<double> &h, std::vector<double> &out);
    static void symconv1DAdd(std::vector<double> &in, std::vector<double> &h, std::vector<double> &out);
    static void conv1D(std::vector<double> &in, std::vector<double> &h, std::vector<double> &out);

private:
    std::vector<double> lp = {0.037828455506995,-0.023849465019380,-0.110624404418423,0.377402855612654,0.852698679009404,0.377402855612654,-0.110624404418423,-0.023849465019380,0.037828455506995};
    std::vector<double> hp = {-0.064538882628938,0.040689417609559,0.418092273222212,-0.788485616405665,0.418092273222212,0.040689417609559,-0.064538882628938};
    std::vector<double> lpr = {-0.0645388826289385,-0.0406894176095585,0.418092273222212,0.788485616405665,0.418092273222212,-0.0406894176095585,-0.0645388826289385};
    std::vector<double> hpr = {-0.0378284555069954,-0.0238494650193800,0.110624404418423,0.377402855612654,-0.852698679009404,0.377402855612654,0.110624404418423,-0.0238494650193800,-0.0378284555069954};
};
} // namespace haptics::tools
#endif //_WAVELET_H_
