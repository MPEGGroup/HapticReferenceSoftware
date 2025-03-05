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

#ifndef BODYPARTTARGET_H
#define BODYPARTTARGET_H

#include <map>
#include <string>

namespace haptics::types {

enum class BodyPartTarget : uint8_t {
  Unknown = 0,
  All = 1,

  Top = 10,
  Down = 11,
  Right = 12,
  Left = 13,
  Front = 14,
  Back = 15,

  Arm = 20,
  Head = 21,
  Chest = 22,
  Waist = 23,
  Leg = 24,

  UpperArm = 30,
  Forearm = 31,
  Hand = 32,
  Crane = 33,
  Neck = 34,
  Thigh = 35,
  Calf = 36,
  Foot = 37,

  Palm = 40,
  Finger = 41,
  Sole = 42,
  Toe = 43,

  Thumb = 50,
  Index = 51,
  Middle = 52,
  Ring = 53,
  Pinky = 54,
  Hallux = 55,
  IndexToe = 56,
  MiddleToe = 57,
  RingToe = 58,
  PinkyToe = 59,

  FirstPhalanx = 60,
  SecondPhalanx = 61,
  ThirdPhalanx = 62,

  Minus = 254,
  Plus = 255
};

static const std::map<std::string, BodyPartTarget> stringToBodyPartTarget = {
    {"Unknown", BodyPartTarget::Unknown},
    {"All", BodyPartTarget::All},
    {"Top", BodyPartTarget::Top},
    {"Down", BodyPartTarget::Down},
    {"Right", BodyPartTarget::Right},
    {"Left", BodyPartTarget::Left},
    {"Front", BodyPartTarget::Front},
    {"Back", BodyPartTarget::Back},
    {"Arm", BodyPartTarget::Arm},
    {"Head", BodyPartTarget::Head},
    {"Chest", BodyPartTarget::Chest},
    {"Waist", BodyPartTarget::Waist},
    {"Leg", BodyPartTarget::Leg},
    {"Upper-arm", BodyPartTarget::UpperArm},
    {"Forearm", BodyPartTarget::Forearm},
    {"Hand", BodyPartTarget::Hand},
    {"Crane", BodyPartTarget::Crane},
    {"Neck", BodyPartTarget::Neck},
    {"Thigh", BodyPartTarget::Thigh},
    {"Calf", BodyPartTarget::Calf},
    {"Foot", BodyPartTarget::Foot},
    {"Palm", BodyPartTarget::Palm},
    {"Finger", BodyPartTarget::Finger},
    {"Sole", BodyPartTarget::Sole},
    {"Toe", BodyPartTarget::Toe},
    {"Thumb", BodyPartTarget::Thumb},
    {"Index", BodyPartTarget::Index},
    {"Middle", BodyPartTarget::Middle},
    {"Ring", BodyPartTarget::Ring},
    {"Pinky", BodyPartTarget::Pinky},
    {"Hallux", BodyPartTarget::Hallux},
    {"Index-toe", BodyPartTarget::IndexToe},
    {"Middle-toe", BodyPartTarget::MiddleToe},
    {"Ring-toe", BodyPartTarget::RingToe},
    {"Pinky-toe", BodyPartTarget::PinkyToe},
    {"First Phalanx", BodyPartTarget::FirstPhalanx},
    {"Second Phalanx", BodyPartTarget::SecondPhalanx},
    {"Third Phalanx", BodyPartTarget::ThirdPhalanx},
    {"Minus", BodyPartTarget::Minus},
    {"Plus", BodyPartTarget::Plus}};
static const std::map<BodyPartTarget, std::string> bodyPartTargetToString = {
    {BodyPartTarget::Unknown, "Unknown"},
    {BodyPartTarget::All, "All"},
    {BodyPartTarget::Top, "Top"},
    {BodyPartTarget::Down, "Down"},
    {BodyPartTarget::Right, "Right"},
    {BodyPartTarget::Left, "Left"},
    {BodyPartTarget::Front, "Front"},
    {BodyPartTarget::Back, "Back"},
    {BodyPartTarget::Arm, "Arm"},
    {BodyPartTarget::Head, "Head"},
    {BodyPartTarget::Chest, "Chest"},
    {BodyPartTarget::Waist, "Waist"},
    {BodyPartTarget::Leg, "Leg"},
    {BodyPartTarget::UpperArm, "Upper-arm"},
    {BodyPartTarget::Forearm, "Forearm"},
    {BodyPartTarget::Hand, "Hand"},
    {BodyPartTarget::Crane, "Crane"},
    {BodyPartTarget::Neck, "Neck"},
    {BodyPartTarget::Thigh, "Thigh"},
    {BodyPartTarget::Calf, "Calf"},
    {BodyPartTarget::Foot, "Foot"},
    {BodyPartTarget::Palm, "Palm"},
    {BodyPartTarget::Finger, "Finger"},
    {BodyPartTarget::Sole, "Sole"},
    {BodyPartTarget::Toe, "Toe"},
    {BodyPartTarget::Thumb, "Thumb"},
    {BodyPartTarget::Index, "Index"},
    {BodyPartTarget::Middle, "Middle"},
    {BodyPartTarget::Ring, "Ring"},
    {BodyPartTarget::Pinky, "Pinky"},
    {BodyPartTarget::Hallux, "Hallux"},
    {BodyPartTarget::IndexToe, "Index-toe"},
    {BodyPartTarget::MiddleToe, "Middle-toe"},
    {BodyPartTarget::RingToe, "Ring-toe"},
    {BodyPartTarget::PinkyToe, "Pinky-toe"},
    {BodyPartTarget::FirstPhalanx, "First Phalanx"},
    {BodyPartTarget::SecondPhalanx, "Second Phalanx"},
    {BodyPartTarget::ThirdPhalanx, "Third Phalanx"},
    {BodyPartTarget::Minus, "Minus"},
    {BodyPartTarget::Plus, "Plus"}};
} // namespace haptics::types

#endif // BODYPARTTARGET_H