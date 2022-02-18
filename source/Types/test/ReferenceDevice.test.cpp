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

#include <Types/include/ReferenceDevice.h>
#include <catch2/catch.hpp>

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("haptics::types::ReferenceDevice getters") {
  using haptics::types::ReferenceDevice;
  ReferenceDevice device(0, "Test device");

  SECTION("Checking getId", "[getId]") { CHECK(device.getId() == 0); }

  SECTION("Checking getName", "[getName]") { CHECK(device.getName() == "Test device"); }

  SECTION("Checking getBodyPartMask", "[getBodyPartMask]") {
    CHECK(!device.getBodyPartMask().has_value());
  }
  SECTION("Checking getMaximumFrequency", "[getMaximumFrequency]") {
    CHECK(!device.getMaximumFrequency().has_value());
  }
  SECTION("Checking getMinimumFrequency", "[getMinimumFrequency]") {
    CHECK(!device.getMinimumFrequency().has_value());
  }
  SECTION("Checking getResonanceFrequency", "[getResonanceFrequency]") {
    CHECK(!device.getResonanceFrequency().has_value());
  }
  SECTION("Checking getMaximumAmplitude", "[getMaximumAmplitude]") {
    CHECK(!device.getMaximumAmplitude().has_value());
  }
  SECTION("Checking getImpedance", "[getImpedance]") { CHECK(!device.getImpedance().has_value()); }
  SECTION("Checking getMaximumVoltage", "[getMaximumVoltage]") {
    CHECK(!device.getMaximumVoltage().has_value());
  }
  SECTION("Checking getMaximumCurrent", "[getMaximumCurrent]") {
    CHECK(!device.getMaximumCurrent().has_value());
  }
  SECTION("Checking getMaximumDisplacement", "[getMaximumDisplacement]") {
    CHECK(!device.getMaximumDisplacement().has_value());
  }
  SECTION("Checking getWeight", "[getWeight]") { CHECK(!device.getWeight().has_value()); }
  SECTION("Checking getSize", "[getSize]") { CHECK(!device.getSize().has_value()); }
  SECTION("Checking getCustom", "[getCustom]") { CHECK(!device.getCustom().has_value()); }
  SECTION("Checking getType", "[getType]") { CHECK(!device.getType().has_value()); }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)
TEST_CASE("haptics::types::ReferenceDevice setters") {
  using haptics::types::ReferenceDevice;
  ReferenceDevice device(0, "Test device");

  SECTION("Checking setId", "[setId]") {
    device.setId(2);
    CHECK(device.getId() == 2);
  }

  SECTION("Checking setName", "[setName]") {
    std::string newName = "Test device 2";
    device.setName(newName);
    CHECK(device.getName() == newName);
  }

  SECTION("Checking setBodyPartMask", "[setBodyPartMask]") {
    device.setBodyPartMask(0);
    REQUIRE(device.getBodyPartMask().has_value());
    CHECK(device.getBodyPartMask().value() == 0);
  }

  SECTION("Checking setMaximumFrequency", "[setMaximumFrequency]") {
    const float maxFrequency = 1000;
    device.setMaximumFrequency(maxFrequency);
    REQUIRE(device.getMaximumFrequency().has_value());
    CHECK(device.getMaximumFrequency().value() == 1000);
  }

  SECTION("Checking setMinimumFrequency", "[setMinimumFrequency]") {
    const float minFrequency = 100;
    device.setMinimumFrequency(minFrequency);
    REQUIRE(device.getMinimumFrequency().has_value());
    CHECK(device.getMinimumFrequency().value() == minFrequency);
  }

  SECTION("Checking setResonanceFrequency", "[setResonanceFrequency]") {
    const float resonanceFrequency = 200;
    device.setResonanceFrequency(resonanceFrequency);
    REQUIRE(device.getResonanceFrequency().has_value());
    CHECK(device.getResonanceFrequency().value() == resonanceFrequency);
  }

  SECTION("Checking setMaximumAmplitude", "[setMaximumAmplitude]") {
    const float maximumAmplitude = 1;
    device.setMaximumAmplitude(maximumAmplitude);
    REQUIRE(device.getMaximumAmplitude().has_value());
    CHECK(device.getMaximumAmplitude().value() == maximumAmplitude);
  }

  SECTION("Checking setImpedance", "[setImpedance]") {
    const float impedance = 10;
    device.setImpedance(impedance);
    REQUIRE(!device.getImpedance().has_value());
    CHECK(device.getImpedance().value() == impedance);
  }

  SECTION("Checking setMaximumVoltage", "[setMaximumVoltage]") {
    const float maximumVoltage = 3;
    device.setMaximumVoltage(maximumVoltage);
    REQUIRE(!device.getMaximumVoltage().has_value());
    CHECK(device.getMaximumVoltage().value() == maximumVoltage);
  }

  SECTION("Checking setMaximumCurrent", "[setMaximumCurrent]") {
    const float maximumCurrent = 1;
    device.setMaximumCurrent(maximumCurrent);
    REQUIRE(!device.getMaximumCurrent().has_value());
    CHECK(device.getMaximumCurrent().value() == maximumCurrent);
  }

  SECTION("Checking setMaximumDisplacement", "[setMaximumDisplacement]") {
    const float maximumDisplacement = 10;
    device.setMaximumDisplacement(maximumDisplacement);
    REQUIRE(!device.getMaximumDisplacement().has_value());
    CHECK(device.getMaximumDisplacement().value() == maximumDisplacement);
  }

  SECTION("Checking setWeight", "[setWeight]") {
    const float weight = 10;
    device.setWeight(weight);
    REQUIRE(!device.getWeight().has_value());
    CHECK(device.getWeight().value() == weight);
  }

  SECTION("Checking setSize", "[setSize]") {
    const float size = 2;
    device.setSize(size);
    REQUIRE(!device.getSize().has_value());
    CHECK(device.getSize().value() == size);
  }

  SECTION("Checking setCustom", "[setCustom]") {
    const float custom = 10;
    device.setCustom(custom);
    REQUIRE(!device.getCustom().has_value());
    CHECK(device.getCustom().value() == custom);
  }

  SECTION("Checking setType", "[setType]") {
    device.setType(haptics::types::ActuatorType::ERM);
    REQUIRE(!device.getType().has_value());
    CHECK(device.getType().value() == haptics::types::ActuatorType::ERM);
  }
}
