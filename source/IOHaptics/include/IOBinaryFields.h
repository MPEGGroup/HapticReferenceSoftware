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

// static constexpr int H_NBITS = 16;
// static constexpr int H_NALU_TYPE = 3;
// static constexpr int H_LEVEL = 2;
// static constexpr int H_PAYLOAD_LENGTH = 8;

// static constexpr int MDEXP_VERSION = 8;
// static constexpr int MDEXP_DATE = 8;
// static constexpr int MDEXP_DESC_SIZE = 8;
static constexpr int MDEXP_PERC_COUNT = 8;
static constexpr int MDEXP_AVATAR_COUNT = 8;

static constexpr int AVATAR_ID = 8;
static constexpr int AVATAR_LOD = 8;
static constexpr int AVATAR_TYPE = 3;
// static constexpr int AVATAR_MESH_COUNT = 8;
// static constexpr int AVATAR_MESH = 32;

static constexpr int MDPERCE_ID = 8;
// static constexpr int MDPERCE_DESC_SIZE = 8;
static constexpr int MDPERCE_MODALITY = 4;
static constexpr int MDPERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_PERCE_UNIT_EXP = 8;
static constexpr int MDPERCE_LIBRARY_COUNT = 16;
static constexpr int MDPERCE_REFDEVICE_COUNT = 8;
static constexpr int MDPERCE_TRACK_COUNT = 8;

static constexpr int REFDEV_ID = 8;
// static constexpr int REFDEV_NAME_LENGTH = 16;
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
static constexpr int REFDEV_TYPE = 3;

static constexpr int MDTRACK_ID = 8;
static constexpr int MDTRACK_DEVICE_ID = 8;
static constexpr int MDTRACK_GAIN = 32;
static constexpr int MDTRACK_MIXING_WEIGHT = 32;
static constexpr int MDTRACK_BODY_PART_MASK = 32;
static constexpr int MDTRACK_OPT_FIELDS = 3;
static constexpr int MDTRACK_FREQ_SAMPLING = 32;
static constexpr int MDTRACK_SAMPLE_COUNT = 32;
static constexpr int MDTRACK_DIRECTION_AXIS = 8;
static constexpr int MDTRACK_VERT_COUNT = 16;
static constexpr int MDTRACK_VERT = 32;
static constexpr int MDTRACK_BANDS_COUNT = 16;

// static constexpr int MDBAND_ID = 8;
static constexpr int MDBAND_BAND_TYPE = 2;
static constexpr int MDBAND_CURVE_TYPE = 3;
static constexpr int MDBAND_WIN_LEN = 16;
static constexpr int MDBAND_LOW_FREQ = 16;
static constexpr int MDBAND_UP_FREQ = 16;
static constexpr int MDBAND_EFFECT_COUNT = 16;

static constexpr int EFFECT_ID = 16;
static constexpr int EFFECT_POSITION = 24;
static constexpr int EFFECT_PHASE = 16;
static constexpr int EFFECT_BASE_SIGNAL = 4;
static constexpr int EFFECT_TYPE = 2;
static constexpr int EFFECT_KEYFRAME_COUNT = 16;
static constexpr int EFFECT_TIMELINE_COUNT = 16;
static constexpr int EFFECT_WAVELET_SIZE = 16;

static constexpr int KEYFRAME_MASK = 3;
static constexpr int KEYFRAME_POSITION = 16;
static constexpr int KEYFRAME_AMPLITUDE = 8;
static constexpr int KEYFRAME_FREQUENCY = 16;

// static constexpr int DB_AU_TYPE = 1;
// static constexpr int DB_TIMESTAMP = 32;
// static constexpr int DB_FX_COUNT = 16;
// static constexpr int FX_ID = 16;

} // namespace haptics::io
#endif // IOBINARYFIELDS_H
