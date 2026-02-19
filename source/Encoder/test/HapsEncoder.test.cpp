/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2022, ISO/IEC
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

#include <Encoder/include/HapsEncoder.h>
#include <catch2/catch.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using haptics::encoder::HapsEncoder;
const unsigned int timescale = 1000;

// --- extractVibration ---
TEST_CASE("haptics::encoder::HapsEncoder::extractVibration succeeds on empty object",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractVibration fails if mute is not bool",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("mute", "not_bool", doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractVibration sets gain to 0 if mute is true",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("mute", true, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_SUCCESS);
  REQUIRE(channel.getGain() == 0.0F);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractVibration fails if frequency_range is not object",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("frequency_range", 123, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractVibration fails if transients is not array",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("transients", 123, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractVibration fails if melodies is not array",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("melodies", 123, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_FAILURE);
}

// --- extractTransients ---
TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if position is missing",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  doc.PushBack(rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if position is negative",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", -1.0, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if amplitude is not double",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("amplitude", "not_double", doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if amplitude is < 0",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("amplitude", -0.1, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if amplitude is > 1",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("amplitude", 1.1, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if pitch is not double",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("pitch", "not_double", doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if pitch is < 0",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("pitch", -0.1, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients fails if pitch is > 1",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("pitch", 1.1, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients succeeds on minimal valid input",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("amplitude", 0.5, doc.GetAllocator());
  t.AddMember("pitch", 0.5, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_SUCCESS);
}

// --- extractMelodies ---
TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies fails if gain is not double",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  m.AddMember("gain", "not_double", doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies fails if gain is negative",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  m.AddMember("gain", -1.0, doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies fails if mute is not bool",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  m.AddMember("mute", "not_bool", doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies fails if notes is not array",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  m.AddMember("notes", 123, doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies fails if note is not object",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  rapidjson::Value notes(rapidjson::kArrayType);
  notes.PushBack(123, doc.GetAllocator());
  m.AddMember("notes", notes, doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies succeeds on minimal valid input",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value m(rapidjson::kObjectType);
  rapidjson::Value notes(rapidjson::kArrayType);
  rapidjson::Value n(rapidjson::kObjectType);
  n.AddMember("position", 0.1, doc.GetAllocator());
  n.AddMember("length", 3, doc.GetAllocator());
  notes.PushBack(n, doc.GetAllocator());
  m.AddMember("notes", notes, doc.GetAllocator());
  doc.PushBack(m, doc.GetAllocator());
  haptics::types::Channel channel(0, "", 1.0F, 1, 0);
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_SUCCESS);
}

// --- extractNote ---
TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if position is missing",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if position is negative",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", -1.0, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if gain is not double",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("gain", "not_double", doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if gain is negative", "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("gain", -1.0, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if phase is not float",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("phase", "not_float", doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if phase is not normalized",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("phase", 2.0, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if waveform is not string",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("waveform", 123, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote fails if waveform is unknown",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("waveform", "NotARealWaveform", doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote succeeds on minimal valid input",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("length", 4, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

// --- extractFrequencyRange ---
TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange succeeds on valid frequency_range",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 10.0, doc.GetAllocator());
  freqRange.AddMember("max", 100.0, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_SUCCESS);
}

TEST_CASE(
    "haptics::encoder::HapsEncoder::extractFrequencyRange fails if frequency_range is not object",
    "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("frequency_range", 123, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange fails if min is not double",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", "not_double", doc.GetAllocator());
  freqRange.AddMember("max", 100.0, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange fails if min is negative",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", -1.0, doc.GetAllocator());
  freqRange.AddMember("max", 100.0, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange fails if max is not double",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 10.0, doc.GetAllocator());
  freqRange.AddMember("max", "not_double", doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange fails if max < min",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 100.0, doc.GetAllocator());
  freqRange.AddMember("max", 50.0, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_FAILURE);
}

// --- getWaveform ---
TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for Sine",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("Sine", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::Sine);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for Square",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("Square", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::Square);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for Triangle",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("Triangle", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::Triangle);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for SawToothUp",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("SawToothUp", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::SawToothUp);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for SawToothDown",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("SawToothDown", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::SawToothDown);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for Constant",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("Constant", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::Constant);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform returns correct enum for Unknown",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("Unknown", out) == EXIT_SUCCESS);
  REQUIRE(out == HapsEncoder::Waveform::Unknown);
}

TEST_CASE("haptics::encoder::HapsEncoder::getWaveform fails for invalid string", "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("NotARealWaveform", out) == EXIT_FAILURE);
}

// --- extractTransients: multiple and boundary values ---
TEST_CASE("haptics::encoder::HapsEncoder::extractTransients handles multiple valid transients",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  for (double pos : {0.0, 0.5, 1.0}) {
    rapidjson::Value t(rapidjson::kObjectType);
    t.AddMember("position", pos, doc.GetAllocator());
    t.AddMember("amplitude", 1.0, doc.GetAllocator());
    t.AddMember("pitch", 0.0, doc.GetAllocator());
    doc.PushBack(t, doc.GetAllocator());
  }
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractTransients succeeds with missing optional "
          "amplitude and pitch",
          "[extractTransients]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.2, doc.GetAllocator());
  doc.PushBack(t, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_SUCCESS);
}

// --- extractMelodies: multiple melodies and notes, with mute/gain ---
TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies handles multiple melodies and notes",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  for (int i = 0; i < 2; ++i) {
    rapidjson::Value melody(rapidjson::kObjectType);
    melody.AddMember("gain", 1.0, doc.GetAllocator());
    melody.AddMember("mute", false, doc.GetAllocator());
    rapidjson::Value notes(rapidjson::kArrayType);
    for (int j = 0; j < 2; ++j) {
      rapidjson::Value n(rapidjson::kObjectType);
      n.AddMember("position", 0.1 * (j + 1), doc.GetAllocator());
      n.AddMember("length", 0.04 * (j + 1), doc.GetAllocator());
      notes.PushBack(n, doc.GetAllocator());
    }
    melody.AddMember("notes", notes, doc.GetAllocator());
    doc.PushBack(melody, doc.GetAllocator());
  }
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies handles melody with mute true",
          "[extractMelodies]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value melody(rapidjson::kObjectType);
  melody.AddMember("mute", true, doc.GetAllocator());
  rapidjson::Value notes(rapidjson::kArrayType);
  rapidjson::Value n(rapidjson::kObjectType);
  n.AddMember("position", 0.1, doc.GetAllocator());
  n.AddMember("length", 0.2, doc.GetAllocator());
  notes.PushBack(n, doc.GetAllocator());
  melody.AddMember("notes", notes, doc.GetAllocator());
  doc.PushBack(melody, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_SUCCESS);
}

// --- extractNote: all optional fields, boundary and compound values ---
TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles all optional fields at boundaries",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.0, doc.GetAllocator());
  doc.AddMember("length", 0.8, doc.GetAllocator());
  doc.AddMember("gain", 1.0, doc.GetAllocator());
  doc.AddMember("phase", 1.0F, doc.GetAllocator());
  doc.AddMember("waveform", "Triangle", doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles extra/unknown fields gracefully",
          "[extractNote]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.5, doc.GetAllocator());
  doc.AddMember("length", 0.5, doc.GetAllocator());
  doc.AddMember("unknown_field", 123, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

// --- extractFrequencyRange: min == max, extra fields ---
TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange succeeds when min equals max",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 50.0, doc.GetAllocator());
  freqRange.AddMember("max", 50.0, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange ignores extra fields",
          "[extractFrequencyRange]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 10.0, doc.GetAllocator());
  freqRange.AddMember("max", 100.0, doc.GetAllocator());
  freqRange.AddMember("extra", 999, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_SUCCESS);
}

// --- getWaveform: case sensitivity and whitespace ---
TEST_CASE("haptics::encoder::HapsEncoder::getWaveform fails for lowercase string",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform("sine", out) == EXIT_FAILURE);
}
TEST_CASE("haptics::encoder::HapsEncoder::getWaveform fails for string with whitespace",
          "[getWaveform]") {
  HapsEncoder::Waveform out;
  REQUIRE(HapsEncoder::getWaveform(" Sine ", out) == EXIT_FAILURE);
}

// --- Integration: extractVibration with transients and melodies together ---
TEST_CASE("haptics::encoder::HapsEncoder::extractVibration handles both transients and melodies",
          "[extractVibration]") {
  rapidjson::Document doc(rapidjson::kObjectType);

  rapidjson::Value transients(rapidjson::kArrayType);
  rapidjson::Value t(rapidjson::kObjectType);
  t.AddMember("position", 0.1, doc.GetAllocator());
  t.AddMember("amplitude", 0.5, doc.GetAllocator());
  t.AddMember("pitch", 0.5, doc.GetAllocator());
  transients.PushBack(t, doc.GetAllocator());
  doc.AddMember("transients", transients, doc.GetAllocator());

  rapidjson::Value melodies(rapidjson::kArrayType);
  rapidjson::Value melody(rapidjson::kObjectType);
  rapidjson::Value notes(rapidjson::kArrayType);
  rapidjson::Value n(rapidjson::kObjectType);
  n.AddMember("position", 0.2, doc.GetAllocator());
  n.AddMember("length", 0.2, doc.GetAllocator());
  notes.PushBack(n, doc.GetAllocator());
  melody.AddMember("notes", notes, doc.GetAllocator());
  melodies.PushBack(melody, doc.GetAllocator());
  doc.AddMember("melodies", melodies, doc.GetAllocator());

  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_SUCCESS);
}

// --- extractNote: amplitude as periodic signal ---
TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles amplitude as periodic signal fails "
          "because no length is given",
          "[extractNote][periodic]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  rapidjson::Value amplitude(rapidjson::kObjectType);
  amplitude.AddMember("amplitude", 0.5, doc.GetAllocator());
  amplitude.AddMember("period_length", 0.2, doc.GetAllocator());
  amplitude.AddMember("phase", 0.0, doc.GetAllocator());
  amplitude.AddMember("vertical_offset", 0.0, doc.GetAllocator());
  amplitude.AddMember("waveform", "Sine", doc.GetAllocator());
  doc.AddMember("amplitude", amplitude, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_FAILURE);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles amplitude as periodic signal",
          "[extractNote][periodic]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  doc.AddMember("length", 1, doc.GetAllocator());
  rapidjson::Value amplitude(rapidjson::kObjectType);
  amplitude.AddMember("amplitude", 0.5, doc.GetAllocator());
  amplitude.AddMember("period_length", 0.2, doc.GetAllocator());
  amplitude.AddMember("phase", 0.0, doc.GetAllocator());
  amplitude.AddMember("vertical_offset", 0.0, doc.GetAllocator());
  amplitude.AddMember("waveform", "Sine", doc.GetAllocator());
  doc.AddMember("amplitude", amplitude, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles amplitude as interpolation curve",
          "[extractNote][curve]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  rapidjson::Value amplitude(rapidjson::kObjectType);
  rapidjson::Value keyframes(rapidjson::kArrayType);
  for (double t : {0.0, 0.1, 0.2}) {
    rapidjson::Value kf(rapidjson::kObjectType);
    kf.AddMember("position", t, doc.GetAllocator());
    kf.AddMember("value", 0.5, doc.GetAllocator());
    keyframes.PushBack(kf, doc.GetAllocator());
  }
  amplitude.AddMember("keyframes", keyframes, doc.GetAllocator());
  doc.AddMember("amplitude", amplitude, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

TEST_CASE("haptics::encoder::HapsEncoder::extractNote handles pitch as interpolation curve",
          "[extractNote][curve]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  doc.AddMember("position", 0.1, doc.GetAllocator());
  rapidjson::Value pitch(rapidjson::kObjectType);
  rapidjson::Value keyframes(rapidjson::kArrayType);
  for (double t : {0.0, 0.1, 0.2}) {
    rapidjson::Value kf(rapidjson::kObjectType);
    kf.AddMember("position", t, doc.GetAllocator());
    kf.AddMember("value", 0.5, doc.GetAllocator());
    keyframes.PushBack(kf, doc.GetAllocator());
  }
  pitch.AddMember("keyframes", keyframes, doc.GetAllocator());
  doc.AddMember("pitch", pitch, doc.GetAllocator());
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractNote(doc.GetObject(), &band, 1.0, timescale) == EXIT_SUCCESS);
}

// --- extractMelodies: melody with mixed valid/invalid notes ---
TEST_CASE(
    "haptics::encoder::HapsEncoder::extractMelodies skips invalid notes but encodes valid ones",
    "[extractMelodies][mixed]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  rapidjson::Value melody(rapidjson::kObjectType);
  rapidjson::Value notes(rapidjson::kArrayType);

  rapidjson::Value n1(rapidjson::kObjectType);
  n1.AddMember("position", 0.1, doc.GetAllocator());
  notes.PushBack(n1, doc.GetAllocator());

  rapidjson::Value n2(rapidjson::kObjectType);
  n2.AddMember("gain", 1.0, doc.GetAllocator());
  notes.PushBack(n2, doc.GetAllocator());

  melody.AddMember("notes", notes, doc.GetAllocator());
  doc.PushBack(melody, doc.GetAllocator());
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_FAILURE);
}

// --- extractTransients: empty array ---
TEST_CASE("haptics::encoder::HapsEncoder::extractTransients succeeds on empty array",
          "[extractTransients][empty]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  haptics::types::Band band;
  REQUIRE(HapsEncoder::extractTransients(doc.GetArray(), &band, timescale) == EXIT_SUCCESS);
}

// --- extractMelodies: empty array ---
TEST_CASE("haptics::encoder::HapsEncoder::extractMelodies succeeds on empty array",
          "[extractMelodies][empty]") {
  rapidjson::Document doc(rapidjson::kArrayType);
  haptics::types::Channel channel;
  REQUIRE(HapsEncoder::extractMelodies(doc.GetArray(), channel, 0, 100, timescale) == EXIT_SUCCESS);
}

// --- extractFrequencyRange: floating-point boundaries ---
TEST_CASE("haptics::encoder::HapsEncoder::extractFrequencyRange handles floating-point min/max",
          "[extractFrequencyRange][float]") {
  rapidjson::Document doc(rapidjson::kObjectType);
  rapidjson::Value freqRange(rapidjson::kObjectType);
  freqRange.AddMember("min", 10.7, doc.GetAllocator());
  freqRange.AddMember("max", 99.3, doc.GetAllocator());
  doc.AddMember("frequency_range", freqRange, doc.GetAllocator());
  int min = 0, max = 0;
  REQUIRE(HapsEncoder::extractFrequencyRange(doc.GetObject(), min, max) == EXIT_SUCCESS);
  REQUIRE(min == 10);
  REQUIRE(max == 100);
}

// --- Integration: overlapping events and edge-case timings ---
TEST_CASE(
    "haptics::encoder::HapsEncoder::extractVibration handles overlapping transients and melodies",
    "[extractVibration][overlap]") {
  rapidjson::Document doc(rapidjson::kObjectType);

  rapidjson::Value transients(rapidjson::kArrayType);
  for (double pos : {0.1, 0.1, 0.2}) {
    rapidjson::Value t(rapidjson::kObjectType);
    t.AddMember("position", pos, doc.GetAllocator());
    t.AddMember("amplitude", 0.5, doc.GetAllocator());
    t.AddMember("pitch", 0.5, doc.GetAllocator());
    transients.PushBack(t, doc.GetAllocator());
  }
  doc.AddMember("transients", transients, doc.GetAllocator());

  rapidjson::Value melodies(rapidjson::kArrayType);
  for (int i = 0; i < 2; ++i) {
    rapidjson::Value melody(rapidjson::kObjectType);
    rapidjson::Value notes(rapidjson::kArrayType);
    for (double pos : {0.1, 0.2}) {
      rapidjson::Value n(rapidjson::kObjectType);
      n.AddMember("position", pos, doc.GetAllocator());
      n.AddMember("length", pos / 5, doc.GetAllocator());
      notes.PushBack(n, doc.GetAllocator());
    }
    melody.AddMember("notes", notes, doc.GetAllocator());
    melodies.PushBack(melody, doc.GetAllocator());
  }
  doc.AddMember("melodies", melodies, doc.GetAllocator());

  haptics::types::Channel channel;
  std::string json = [](const rapidjson::Document &d) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    d.Accept(w);
    return std::string(sb.GetString());
  }(doc);
  REQUIRE(HapsEncoder::extractVibration(doc.GetObject(), channel, timescale) == EXIT_SUCCESS);
}
