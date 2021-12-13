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

#include "../include/IvsEncoder.h"
#include <Effect.h>
#include <Keyframe.h>
#include <catch2/catch.hpp>

using haptics::encoder::IvsEncoder;

TEST_CASE("IvsEncoder::getLastModified without node", "[getLastModified][withoutNode]") {
  pugi::xml_document doc;
  std::string res = IvsEncoder::getLastModified(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLastModified without attribute", "[getLastModified][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("ivs-file");
  std::string res = IvsEncoder::getLastModified(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLastModified with wrong node", "[getLastModified][wrongNode]") {
  std::string testingValue("Sunday, April 11, 2021  10:39:06PM");
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("wrong ivs-file");
  node.append_attribute("last-modified") = testingValue.c_str();

  std::string res = IvsEncoder::getLastModified(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLastModified with wrong attribute", "[getLastModified][wrongAttribute]") {
  std::string testingValue("Sunday, April 11, 2021  10:39:06PM");
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("ivs-file");
  node.append_attribute("wrong last-modified") = testingValue.c_str();

  std::string res = IvsEncoder::getLastModified(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLastModified", "[getLastModified][correctParam]") {
  const std::vector<char *> testingValues = {"Hello World", "Sunday, April 11, 2021  10:39:06PM",
                                             "42", "magic string", ""};

  int16_t i = 0;
  for (char *testingCase : testingValues) {
    pugi::xml_document doc;
    pugi::xml_node node = doc.append_child("ivs-file");
    node.append_attribute("last-modified") = testingCase;

    std::string expected(testingCase);
    DYNAMIC_SECTION("getLastModified with configured date (TESTING CASE: " + std::to_string(i) +
                    ")") {
      std::string res = IvsEncoder::getLastModified(&doc);

      REQUIRE(expected == res);
    }

    i++;
  }
}


TEST_CASE("IvsEncoder::getBasisEffects without node", "[getLastModified][withoutNode]") {
  pugi::xml_document doc;
  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getBasisEffects(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getBasisEffects with wrong node", "[getLastModified][wrongNode]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("ivs-file").append_child("wrong effects").append_child("basis-effect");
  node.append_attribute("name") = "Hello World";
  node.append_attribute("duration") = "42";

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getBasisEffects(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getBasisEffects", "[getLastModified][correctParam]") {
  std::vector<std::vector<char *>> testingValues = {
      {"ShortTransitionRampUp100_1", "500", "sine"},
      {"Bump100_1", "50", "sine"},
      {"ShortBuzz100_1", "250", "square"},
  };

  pugi::xml_document doc;
  pugi::xml_node effectsNode = doc.append_child("ivs-file").append_child("effects");
  pugi::xml_node timelineNode = effectsNode.append_child("timeline-effect");
  pugi::xml_node launchNode;
  pugi::xml_node effectNode;
  std::vector<char *> testingEffectValue;
  int16_t i;
  for (i = 0; i < testingValues.size(); i++) {
    testingEffectValue = testingValues[i];

    launchNode = timelineNode.append_child("launch-effect");
    launchNode.append_attribute("effect") = testingEffectValue[0];
    launchNode.append_attribute("time") = i * 500;

    effectNode = effectsNode.append_child("basis-effect");
    effectNode.append_attribute("waveform") = testingEffectValue[2];
    effectNode.append_attribute("name") = testingEffectValue[0];
    effectNode.append_attribute("duration") = testingEffectValue[1];
  }

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getBasisEffects(&doc);

  i = 0;
  for (pugi::xml_node n : res) {
    testingEffectValue = testingValues[i];

    REQUIRE(std::string(n.name()) == "basis-effect");
    REQUIRE(std::string(n.attribute("name").value()) == testingEffectValue[0]);
    REQUIRE(std::string(n.attribute("duration").value()) == testingEffectValue[1]);
    REQUIRE(std::string(n.attribute("waveform").value()) == testingEffectValue[2]);

    i++;
  }
}


TEST_CASE("IvsEncoder::getLaunchEvents without node", "[getLaunchEvents][withoutNode]") {
  pugi::xml_document doc;
  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getLaunchEvents(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLaunchEvents with wrong node", "[getLaunchEvents][wrongNode]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("ivs-file")
                            .append_child("effects")
                            .append_child("wrong timeline-effect")
                            .append_child("launch-event");
  node.append_attribute("effect") = "Hello World";
  node.append_attribute("time") = "42";

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getLaunchEvents(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getLaunchEvents", "[getLaunchEvents][correctParam]") {
  std::vector<std::vector<char *>> testingValues = {
      {"ShortTransitionRampUp100_1", "500"},
      {"Bump100_1", "50"},
      {"ShortBuzz100_1", "250"},
  };

  pugi::xml_document doc;
  pugi::xml_node effectsNode = doc.append_child("ivs-file").append_child("effects");
  pugi::xml_node timelineNode = effectsNode.append_child("timeline-effect");
  pugi::xml_node launchNode;
  pugi::xml_node effectNode;
  std::vector<char *> testingEffectValue;
  int16_t i;
  for (i = 0; i < testingValues.size(); i++) {
    testingEffectValue = testingValues[i];

    launchNode = timelineNode.append_child("launch-effect");
    launchNode.append_attribute("effect") = testingEffectValue[0];
    launchNode.append_attribute("time") = testingEffectValue[1];

    effectNode = effectsNode.append_child("basis-effect");
    effectNode.append_attribute("waveform") = "sine";
    effectNode.append_attribute("name") = testingEffectValue[0];
    effectNode.append_attribute("duration") = 42 * i;
  }

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getLaunchEvents(&doc);

  i = 0;
  for (pugi::xml_node n : res) {
    testingEffectValue = testingValues[i];

    REQUIRE(std::string(n.name()) == "launch-effect");
    REQUIRE(std::string(n.attribute("effect").value()) == testingEffectValue[0]);
    REQUIRE(std::string(n.attribute("time").value()) == testingEffectValue[1]);

    i++;
  }
}


TEST_CASE("IvsEncoder::getRepeatEvents without node", "[getRepeatEvents][withoutNode]") {
  pugi::xml_document doc;
  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getLaunchEvents(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getRepeatEvents with wrong node", "[getRepeatEvents][wrongNode]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("ivs-file")
                            .append_child("effects")
                            .append_child("wrong timeline-effect")
                            .append_child("repeat-event");
  node.append_attribute("time") = 42;
  node.append_attribute("count") = 5;
  node.append_attribute("duration") = 1583;

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getRepeatEvents(&doc);

  REQUIRE(res.empty());
}

TEST_CASE("IvsEncoder::getRepeatEvents", "[getRepeatEvents][correctParam]") {
  std::vector<std::vector<int>> testingValues = {
      {43, 54321, 543},
      {0, 50, 0},
      {250, 543, 43210},
  };

  pugi::xml_document doc;
  pugi::xml_node node =
      doc.append_child("ivs-file").append_child("effects").append_child("timeline-effect");
  pugi::xml_node n;
  std::vector<int> testingEffectValue;
  int16_t i;
  for (i = 0; i < testingValues.size(); i++) {
    testingEffectValue = testingValues[i];

    n = node.append_child("repeat-effect");
    n.append_attribute("time") = testingEffectValue[0];
    n.append_attribute("count") = testingEffectValue[1];
    n.append_attribute("duration") = testingEffectValue[2];
  }

  pugi::xml_object_range<pugi::xml_named_node_iterator> res = IvsEncoder::getRepeatEvents(&doc);

  i = 0;
  for (pugi::xml_node n : res) {
    testingEffectValue = testingValues[i];

    REQUIRE(std::string(n.name()) == "launch-effect");
    REQUIRE(n.attribute("time").as_int() == testingEffectValue[0]);
    REQUIRE(n.attribute("count").as_int() == testingEffectValue[1]);
    REQUIRE(n.attribute("duration").as_int() == testingEffectValue[2]);

    i++;
  }
}


TEST_CASE("IvsEncoder::getLaunchedEffect without launch", "[getLaunchedEffect][withoutNode]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("duration") = "42";
  pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects = node.children("basis-effect");

  pugi::xml_node res = {};
  REQUIRE_FALSE(IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, res));
}

TEST_CASE("IvsEncoder::getLaunchedEffect without basis", "[getLaunchedEffect][withoutNode]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("effect") = "Hello World";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects = node.children("basis-effect");

  pugi::xml_node res = {};
  REQUIRE_FALSE(IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, res));
}

TEST_CASE("IvsEncoder::getLaunchedEffect without launch/basis effect matching",
          "[getLaunchedEffect][withoutMatching]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("effect") = "Hello World";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "World Hello";
  basisEffect.append_attribute("duration") = "42";
  pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects = node.children("basis-effect");

  pugi::xml_node res = {};
  REQUIRE_FALSE(IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, res));
}

TEST_CASE("IvsEncoder::getLaunchedEffect normal case",
          "[getLaunchedEffect][correctParam]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("effect") = "Hello World";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "World Hello";
  basisEffect.append_attribute("duration") = "42";
  basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("duration") = "24";
  pugi::xml_object_range<pugi::xml_named_node_iterator> basisEffects =
      node.children("basis-effect");

  pugi::xml_node res = {};
  REQUIRE(IvsEncoder::getLaunchedEffect(&basisEffects, &launchEvent, res));
  CHECK(std::string(res.name()) == "basis-effect");
  CHECK(std::string(launchEvent.attribute("effect").as_string()) == std::string(res.attribute("name").as_string()));
  CHECK(std::string(res.attribute("name").as_string()) == "Hello World");
  CHECK(res.attribute("duration").as_int() == 24);
}


TEST_CASE("IvsEncoder::getTime without value", "[getTime][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("repeat-event");

  int res = IvsEncoder::getTime(&repeatEvent);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getTime without override", "[getTime][withValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("basis-effect");
  repeatEvent.append_attribute("time") = 3;

  int res = IvsEncoder::getTime(&repeatEvent);

  REQUIRE(res == 3);
}


TEST_CASE("IvsEncoder::getCount without value", "[getCount][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("repeat-event");

  int res = IvsEncoder::getCount(&repeatEvent);

  REQUIRE(res == 0);
}

TEST_CASE("IvsEncoder::getCount without override", "[getCount][withValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("basis-effect");
  repeatEvent.append_attribute("count") = 3254;

  int res = IvsEncoder::getCount(&repeatEvent);

  REQUIRE(res == 3254);
}


TEST_CASE("IvsEncoder::getDuration(1 param) without value", "[getDuration][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("repeat-event");

  int res = IvsEncoder::getDuration(&repeatEvent);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getDuration(1 param) without override", "[getDuration][withValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node repeatEvent = node.append_child("repeat-event");
  repeatEvent.append_attribute("duration") = 3;

  int res = IvsEncoder::getDuration(&repeatEvent);

  REQUIRE(res == 3);
}

TEST_CASE("IvsEncoder::getDuration(2 params) without value", "[getDuration][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");

  int res = IvsEncoder::getDuration(&basisEffect, &launchEvent);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getDuration(2 params) without override", "[getDuration][withoutOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("duration") = "24";

  int res = IvsEncoder::getDuration(&basisEffect, &launchEvent);

  REQUIRE(res == 24);
}

TEST_CASE("IvsEncoder::getDuration(2 params) with override", "[getDuration][withOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("duration-override") = "42";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("duration") = "24";

  int res = IvsEncoder::getDuration(&basisEffect, &launchEvent);

  REQUIRE(res == 42);
}


TEST_CASE("IvsEncoder::getMagnitude without value", "[getMagnitude][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");

  int res = IvsEncoder::getMagnitude(&basisEffect, &launchEvent);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getMagnitude without override", "[getMagnitude][withoutOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("magnitude") = "24";

  int res = IvsEncoder::getMagnitude(&basisEffect, &launchEvent);

  REQUIRE(res == 24);
}

TEST_CASE("IvsEncoder::getMagnitude with override", "[getMagnitude][withOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("magnitude-override") = "42";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("magnitude") = "24";

  int res = IvsEncoder::getMagnitude(&basisEffect, &launchEvent);

  REQUIRE(res == 42);
}


TEST_CASE("IvsEncoder::getPeriod without value", "[getPeriod][withoutValue]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");

  int res = IvsEncoder::getPeriod(&basisEffect, &launchEvent);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getPeriod without override", "[getPeriod][withoutOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("period") = "24";

  int res = IvsEncoder::getPeriod(&basisEffect, &launchEvent);

  REQUIRE(res == 24);
}

TEST_CASE("IvsEncoder::getPeriod with override", "[getPeriod][withOverride]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("period-override") = "42";
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("period") = "24";

  int res = IvsEncoder::getPeriod(&basisEffect, &launchEvent);

  REQUIRE(res == 42);
}


TEST_CASE("IvsEncoder::getAttackTime without configured attribute",
          "[getAttackTime][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("wrong attack-time") = 42;

  int res = IvsEncoder::getAttackTime(&basisEffect);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getAttackTime with configured attribute", "[getAttackTime][withAttribute]") {
  std::vector<int> testingValues = {
    42,
    0,
    350
  };

  for (int i = 0; i < testingValues.size(); i++) {
    DYNAMIC_SECTION("IvsEncoder::getAttackTime with configured attribute (TESTING CASE: " +
                    std::to_string(i) + ")") {
      int testingCase = testingValues[i];
      pugi::xml_document doc;
      pugi::xml_node node = doc.append_child("root");
      pugi::xml_node basisEffect = node.append_child("basis-effect");
      basisEffect.append_attribute("attack-time") = testingCase;

      int res = IvsEncoder::getAttackTime(&basisEffect);

      REQUIRE(res == testingCase);
    }
  }
}


TEST_CASE("IvsEncoder::getAttackLevel without configured attribute",
          "[getAttackLevel][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("wrong attack-level") = 42;

  int res = IvsEncoder::getAttackLevel(&basisEffect);

  REQUIRE(res == 0);
}

TEST_CASE("IvsEncoder::getAttackLevel with configured attribute", "[getAttackLevel][withAttribute]") {
  std::vector<int> testingValues = {42, 0, 350};

  for (int i = 0; i < testingValues.size(); i++) {
    DYNAMIC_SECTION("IvsEncoder::getAttackLevel with configured attribute (TESTING CASE: " +
                    std::to_string(i) + ")") {
      int testingCase = testingValues[i];
      pugi::xml_document doc;
      pugi::xml_node node = doc.append_child("root");
      pugi::xml_node basisEffect = node.append_child("basis-effect");
      basisEffect.append_attribute("attack-level") = testingCase;

      int res = IvsEncoder::getAttackLevel(&basisEffect);

      REQUIRE(res == testingCase);
    }
  }
}


TEST_CASE("IvsEncoder::getFadeTime without configured attribute", "[getFadeTime][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("wrong fade-time") = 42;

  int res = IvsEncoder::getFadeTime(&basisEffect);

  REQUIRE(res == -1);
}

TEST_CASE("IvsEncoder::getFadeTime with configured attribute", "[getFadeTime][withAttribute]") {
  std::vector<int> testingValues = {42, 0, 350};

  for (int i = 0; i < testingValues.size(); i++) {
    DYNAMIC_SECTION("IvsEncoder::getFadeTime with configured attribute (TESTING CASE: " +
                    std::to_string(i) + ")") {
      int testingCase = testingValues[i];
      pugi::xml_document doc;
      pugi::xml_node node = doc.append_child("root");
      pugi::xml_node basisEffect = node.append_child("basis-effect");
      basisEffect.append_attribute("fade-time") = testingCase;

      int res = IvsEncoder::getFadeTime(&basisEffect);

      REQUIRE(res == testingCase);
    }
  }
}


TEST_CASE("IvsEncoder::getFadeLevel without configured attribute", "[getFadeLevel][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("wrong fade-level") = 42;

  int res = IvsEncoder::getFadeLevel(&basisEffect);

  REQUIRE(res == 0);
}

TEST_CASE("IvsEncoder::getFadeLevel with configured attribute", "[getFadeLevel][withAttribute]") {
  std::vector<int> testingValues = {42, 0, 350};

  for (int i = 0; i < testingValues.size(); i++) {
    DYNAMIC_SECTION("IvsEncoder::getFadeLevel with configured attribute (TESTING CASE: " +
                    std::to_string(i) + ")") {
      int testingCase = testingValues[i];
      pugi::xml_document doc;
      pugi::xml_node node = doc.append_child("root");
      pugi::xml_node basisEffect = node.append_child("basis-effect");
      basisEffect.append_attribute("fade-level") = testingCase;

      int res = IvsEncoder::getFadeLevel(&basisEffect);

      REQUIRE(res == testingCase);
    }
  }
}


TEST_CASE("IvsEncoder::getWaveform without configured attribute", "[getWaveform][withoutAttribute]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("wrong waveform") = "sine";

  haptics::types::BaseSignal res = IvsEncoder::getWaveform(&basisEffect);

  REQUIRE(res == haptics::types::BaseSignal(-1));
}

TEST_CASE("IvsEncoder::getWaveform with configured attribute", "[getWaveform][withAttribute]") {
  std::vector<char *> testingValues = {
    "something wrong",
    "sine",
    "square",
    "triangle",
    "sawtooth-up",
    "sawtooth-down"
  };

  for (int i = 0; i < testingValues.size(); i++) {
    DYNAMIC_SECTION("IvsEncoder::getWaveform with configured attribute (TESTING CASE: " +
      std::to_string(i) + ")") {
      char *testingCase = testingValues[i];
      pugi::xml_document doc;
      pugi::xml_node node = doc.append_child("root");
      pugi::xml_node basisEffect = node.append_child("basis-effect");
      basisEffect.append_attribute("waveform") = testingCase;

      haptics::types::BaseSignal res = IvsEncoder::getWaveform(&basisEffect);

      REQUIRE(res == haptics::types::BaseSignal(i - 1));
    }
  }
}


TEST_CASE("IVSEncoder::convertToEffect simple case", "[getWaveform][withoutAttack][withoutFade]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("time") = 42;
  launchEvent.append_attribute("effect") = "Hello World";

  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("type") = "periodic";
  basisEffect.append_attribute("duration") = 500;
  basisEffect.append_attribute("magnitude") = 5000;
  basisEffect.append_attribute("waveform") = "sawtooth-down";
  basisEffect.append_attribute("period") = 80;

  haptics::types::Effect res;
  REQUIRE(IvsEncoder::convertToEffect(&basisEffect, &launchEvent, &res));

  CHECK(res.getPosition() == 42);
  CHECK(res.getPhase() == Approx(0.0));
  CHECK(res.getBaseSignal() == haptics::types::BaseSignal::SawToothDown);
  CHECK(res.getKeyframesSize() == 2);

  haptics::types::Keyframe k = res.getKeyframeAt(0);
  CHECK(k.getRelativePosition() == 0);
  CHECK(k.getAmplitudeModulation() == Approx(0.5));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(1);
  CHECK(k.getRelativePosition() == 500);
  CHECK(k.getAmplitudeModulation() == Approx(0.5));
  CHECK(k.getFrequencyModulation() == 80);
}

TEST_CASE("IVSEncoder::convertToEffect with attack", "[getWaveform][withAttack][withoutFade]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("time") = 42;
  launchEvent.append_attribute("effect") = "Hello World";

  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("type") = "periodic";
  basisEffect.append_attribute("duration") = 500;
  basisEffect.append_attribute("magnitude") = 7500;
  basisEffect.append_attribute("waveform") = "sawtooth-down";
  basisEffect.append_attribute("period") = 80;
  basisEffect.append_attribute("attack-time") = 10;
  basisEffect.append_attribute("attack-level") = 1000;


  haptics::types::Effect res;
  REQUIRE(IvsEncoder::convertToEffect(&basisEffect, &launchEvent, &res));

  CHECK(res.getPosition() == 42);
  CHECK(res.getPhase() == Approx(0.0));
  CHECK(res.getBaseSignal() == haptics::types::BaseSignal::SawToothDown);
  CHECK(res.getKeyframesSize() == 3);

  haptics::types::Keyframe k = res.getKeyframeAt(0);
  CHECK(k.getRelativePosition() == 0);
  CHECK(k.getAmplitudeModulation() == Approx(0.1));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(1);
  CHECK(k.getRelativePosition() == 10);
  CHECK(k.getAmplitudeModulation() == Approx(0.75));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(2);
  CHECK(k.getRelativePosition() == 500);
  CHECK(k.getAmplitudeModulation() == Approx(0.75));
  CHECK(k.getFrequencyModulation() == 80);
}

TEST_CASE("IVSEncoder::convertToEffect with fade", "[getWaveform][withoutAttack][withFade]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("time") = 42;
  launchEvent.append_attribute("effect") = "Hello World";

  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("type") = "periodic";
  basisEffect.append_attribute("duration") = 500;
  basisEffect.append_attribute("magnitude") = 2500;
  basisEffect.append_attribute("waveform") = "sawtooth-down";
  basisEffect.append_attribute("period") = 80;
  basisEffect.append_attribute("fade-time") = 1;
  basisEffect.append_attribute("fade-level") = 10000;

  haptics::types::Effect res;
  REQUIRE(IvsEncoder::convertToEffect(&basisEffect, &launchEvent, &res));

  CHECK(res.getPosition() == 42);
  CHECK(res.getPhase() == Approx(0.0));
  CHECK(res.getBaseSignal() == haptics::types::BaseSignal::SawToothDown);
  CHECK(res.getKeyframesSize() == 3);

  haptics::types::Keyframe k = res.getKeyframeAt(0);
  CHECK(k.getRelativePosition() == 0);
  CHECK(k.getAmplitudeModulation() == Approx(0.25));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(1);
  CHECK(k.getRelativePosition() == 499);
  CHECK(k.getAmplitudeModulation() == Approx(0.25));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(2);
  CHECK(k.getRelativePosition() == 500);
  CHECK(k.getAmplitudeModulation() == Approx(1));
  CHECK(k.getFrequencyModulation() == 80);
}

TEST_CASE("IVSEncoder::convertToEffect with attack and fade",
          "[getWaveform][withAttack][withFade]") {
  pugi::xml_document doc;
  pugi::xml_node node = doc.append_child("root");
  pugi::xml_node launchEvent = node.append_child("launch-effect");
  launchEvent.append_attribute("time") = 42;
  launchEvent.append_attribute("effect") = "Hello World";

  pugi::xml_node basisEffect = node.append_child("basis-effect");
  basisEffect.append_attribute("name") = "Hello World";
  basisEffect.append_attribute("type") = "periodic";
  basisEffect.append_attribute("duration") = 500;
  basisEffect.append_attribute("magnitude") = 500;
  basisEffect.append_attribute("waveform") = "sawtooth-down";
  basisEffect.append_attribute("period") = 80;
  basisEffect.append_attribute("attack-time") = 1;
  basisEffect.append_attribute("attack-level") = 1000;
  basisEffect.append_attribute("fade-time") = 20;
  basisEffect.append_attribute("fade-level") = 10000;

  haptics::types::Effect res;
  REQUIRE(IvsEncoder::convertToEffect(&basisEffect, &launchEvent, &res));

  CHECK(res.getPosition() == 42);
  CHECK(res.getPhase() == Approx(0.0));
  CHECK(res.getBaseSignal() == haptics::types::BaseSignal::SawToothDown);
  CHECK(res.getKeyframesSize() == 4);

  haptics::types::Keyframe k = res.getKeyframeAt(0);
  CHECK(k.getRelativePosition() == 0);
  CHECK(k.getAmplitudeModulation() == Approx(0.1));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(1);
  CHECK(k.getRelativePosition() == 1);
  CHECK(k.getAmplitudeModulation() == Approx(0.05));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(2);
  CHECK(k.getRelativePosition() == 480);
  CHECK(k.getAmplitudeModulation() == Approx(0.05));
  CHECK(k.getFrequencyModulation() == 80);

  k = res.getKeyframeAt(3);
  CHECK(k.getRelativePosition() == 500);
  CHECK(k.getAmplitudeModulation() == Approx(1.0));
  CHECK(k.getFrequencyModulation() == 80);
}
