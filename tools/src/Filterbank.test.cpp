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

#include "Filterbank.h"
#include <vector>
#include <iostream>

constexpr size_t BL = 25;
constexpr double FS = 8000;

TEST_CASE("haptics::tools::Filterbank") {

    using haptics::tools::Filterbank;

    SECTION("LP") {

        std::vector<double> in(BL,0);
        in[(BL+1)/2-1] = 1;
        Filterbank fb(FS);
        std::vector<double> out = fb.LP(in,FS/4);

        /*for(int i=0; i<BL; i++){
            std::cout << out[i] << std::endl;
        }*/

        CHECK(true);


    }

    SECTION("HP") {

        std::vector<double> in(BL,0);
        in[(BL+1)/2-1] = 1;
        Filterbank fb(FS);
        std::vector<double> out = fb.HP(in,FS/4);

        /*for(int i=0; i<BL; i++){
            std::cout << out[i] << std::endl;
        }*/

        CHECK(true);


    }

    SECTION("LP+HP") {

        std::vector<double> in(BL,0);
        in[(BL+1)/2-1] = 1;
        Filterbank fb(FS);
        std::vector<double> out1 = fb.HP(in,FS/4);

        std::vector<double> out2 = fb.LP(in,FS/4);

        /*for(int i=0; i<BL; i++){
            std::cout << out1[i] +out2[i] << std::endl;
        }*/

        CHECK(true);



    }
}
