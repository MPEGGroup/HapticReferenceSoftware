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

#include <Types/include/Avatar.h>
#include <catch2/catch.hpp>

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("haptics::tools::Avatar") {

  using haptics::types::Avatar;

  Avatar avatar(1, 2, haptics::types::AvatarType::Vibration);
  SECTION("Checking getId", "[getId]") {
    auto checkId = avatar.getId();
    CHECK(checkId == 1);
  }
  SECTION("Checking getLod", "[getLod]") {
    auto checkLod = avatar.getLod();
    CHECK(checkLod == 2);
  }
  SECTION("Checking getType", "[getType]") {
    auto checkType = avatar.getType();
    CHECK(checkType == haptics::types::AvatarType::Vibration);
  }
  SECTION("Checking getMesh", "[getMesh]") {
    auto checkMesh = avatar.getMesh();
    CHECK(!checkMesh.has_value());
  }
  SECTION("Checking setId", "[setId]") {
    avatar.setId(2);
    auto checkId = avatar.getId();
    CHECK(checkId == 2);
  }
  SECTION("Checking setLod", "[setLod]") {
    avatar.setLod(3);
    auto checkLod = avatar.getLod();
    CHECK(checkLod == 3);
  }
  SECTION("Checking setType", "[setType]") {
    avatar.setType(haptics::types::AvatarType::Pressure);
    auto checkType = avatar.getType();
    CHECK(checkType == haptics::types::AvatarType::Pressure);
  }
  SECTION("Checking setMesh", "[setMesh]") {
    std::string mesh = "SomeMesh.obj";
    avatar.setMesh(mesh);
    auto checkMesh = avatar.getMesh();
    CHECK(checkMesh == mesh);
  }
}
