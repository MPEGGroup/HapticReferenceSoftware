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

#ifndef IOBINARYFIELDS_H
#define IOBINARYFIELDS_H

#include <IOHaptics/include/IOBinaryPrimitives.h>
#include <Types/include/Haptics.h>
#include <bitset>
#include <string>
#include <vector>

namespace haptics::io {

static constexpr int LEVEL = 8;

static constexpr int UNIT_TYPE = 6;
static constexpr int UNIT_SYNC = 2;
static constexpr int UNIT_LAYER = 4;
static constexpr int UNIT_DURATION = 24;
static constexpr int UNIT_LENGTH = 32;
static constexpr int UNIT_RESERVED = 4;

static constexpr int H_NBITS = 24;
static constexpr int H_MIHS_PACKET_TYPE = 6;
static constexpr int H_PAYLOAD_LENGTH = 17;
static constexpr int H_RESERVED = 1;

static constexpr int TIMING_TIME = 32;

static constexpr int INITTIMING_TIMESCALE = 32;
static constexpr int INITTIMING_NOMINALDURATION = 24;
static constexpr int INITTIMING_DURATIONDEVIATION = 24;
static constexpr int INITTIMING_OVERLAPPING = 1;

static constexpr int MDEXP_VERSION = 8;
static constexpr int MDEXP_PROFILE_SIZE = 8;
static constexpr int MDEXP_LEVEL = 8;
static constexpr int MDEXP_DATE = 8;
static constexpr int MDEXP_DESC_SIZE = 8;

static constexpr int MDEXP_PERC_COUNT = 8;
static constexpr int MDEXP_AVATAR_COUNT = 8;

static constexpr int AVATAR_ID = 8;
static constexpr int AVATAR_LOD = 8;
static constexpr int AVATAR_TYPE = 8;

static constexpr int AVATAR_MESH_COUNT = 8;

static constexpr int MDPERCE_ID = 8;
static constexpr int MDPERCE_PRIORITY = 8;
static constexpr int MDPERCE_DESC_SIZE = 8;
static constexpr int MDPERCE_MODALITY = 8;
static constexpr int MDPERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_PERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_LIBRARY_COUNT = 16;
static constexpr int MDPERCE_FLAG_SEMANTIC = 1;
static constexpr int MDPERCE_SCHEME_LENGTH = 8;
static constexpr int MDPERCE_SCHEME_CHAR = 8;
static constexpr int MDPERCE_REFDEVICE_COUNT = 8;
static constexpr int MDPERCE_CHANNEL_COUNT = 16;

static constexpr int REFDEV_ID = 8;

static constexpr int REFDEV_NAME_LENGTH = 8;

static constexpr int REFDEV_BODY_PART_MASK = 32;
static constexpr int REFDEV_OPT_FIELDS = 12;
static constexpr int REFDEV_MAX_FREQ = 32;
static constexpr int REFDEV_MIN_FREQ = 32;
static constexpr int REFDEV_RES_FREQ = 32;
static constexpr int REFDEV_MAX_AMP = 32;
static constexpr int REFDEV_IMPEDANCE = 32;
static constexpr int REFDEV_MAX_VOLT = 32;
static constexpr int REFDEV_MAX_CURR = 32;
static constexpr int REFDEV_MAX_DISP = 32;
static constexpr int REFDEV_WEIGHT = 32;
static constexpr int REFDEV_SIZE = 32;
static constexpr int REFDEV_CUSTOM = 32;
static constexpr int REFDEV_TYPE = 4;
static constexpr int REFDEV_MAX_ID = 255;

static constexpr int MDCHANNEL_ID = 16;
static constexpr int MDCHANNEL_PRIORITY = 8;
static constexpr int MDCHANNEL_DESC_LENGTH = 8;
static constexpr int MDCHANNEL_DEVICE_ID = 8;
static constexpr int MDCHANNEL_GAIN = 32;
static constexpr int MDCHANNEL_MIXING_WEIGHT = 32;
static constexpr int MDCHANNEL_BODY_PART_MASK = 32;
static constexpr int MDCHANNEL_OPT_FIELDS = 8;
static constexpr int MDCHANNEL_FREQ_SAMPLING = 32;
static constexpr int MDCHANNEL_SAMPLE_COUNT = 32;
static constexpr int MDCHANNEL_DIRECTION_MASK = 1;
static constexpr int MDCHANNEL_DIRECTION_AXIS = 8;
static constexpr int MDCHANNEL_VERT_COUNT = 16;
static constexpr int MDCHANNEL_VERT = 32;
static constexpr int MDCHANNEL_BANDS_COUNT = 8;
static constexpr int MDCHANNEL_BODY_PART_TARGET_COUNT = 8;
static constexpr int MDCHANNEL_ACTUATOR_TARGET_COUNT = 8;
static constexpr int MDCHANNEL_BODY_PART_TARGET = 8;

static constexpr int MDBAND_ID = 8;
static constexpr int MDBAND_PRIORITY = 8;
static constexpr int MDBAND_BAND_TYPE = 3;
static constexpr int MDBAND_CURVE_TYPE = 4;
static constexpr int MDBAND_WIN_LEN = 8;
static constexpr int MDBAND_BLK_LEN = 8;
static constexpr int MDBAND_LOW_FREQ = 16;
static constexpr int MDBAND_UP_FREQ = 16;
static constexpr int MDBAND_EFFECT_COUNT = 16;

static constexpr int EFFECT_ID = 16;
static constexpr int EFFECT_POSITION = 25;
static constexpr int EFFECT_FLAG_SEMANTIC = 1;
static constexpr int EFFECT_SEMANTIC_LAYER_1 = 4;
static constexpr int EFFECT_SEMANTIC_LAYER_2 = 8;
static constexpr int EFFECT_POSITION_STREAMING = 25;
static constexpr int EFFECT_PHASE = 16;
static constexpr int EFFECT_BASE_SIGNAL = 4;
static constexpr int EFFECT_TYPE = 2;
static constexpr int EFFECT_KEYFRAME_COUNT = 16;
static constexpr int EFFECT_TIMELINE_COUNT = 16;
static constexpr int EFFECT_WAVELET_SIZE = 16;

static constexpr int KEYFRAME_MASK = 3;

static constexpr int KEYFRAME_VECTORIAL_MASK = 2;

static constexpr int KEYFRAME_POSITION = 16;
static constexpr int KEYFRAME_AMPLITUDE = 8;
static constexpr int KEYFRAME_FREQUENCY = 16;

static constexpr int DB_AU_TYPE = 1;
static constexpr int DB_DURATION = 32;
static constexpr int DB_EFFECT_COUNT = 16;

static constexpr int CRC32_NB_BITS = 32;
static constexpr int CRC16_NB_BITS = 16;
static constexpr int GCRC_NB_PACKET = 8;

} // namespace haptics::io
#endif // IOBINARYFIELDS_H
