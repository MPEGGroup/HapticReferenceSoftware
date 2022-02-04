#!/usr/bin/env python3

# The copyright in this software is being made available under the BSD
# License, included below. This software may be subject to other third party
# and contributor rights, including patent rights, and no such rights are
# granted under this license.
#
# Copyright (c) 2010-2022, ISO/IEC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#  * Neither the name of the ISO/IEC nor the names of its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

import pytest
import subprocess
import tempfile
import os
from psnr import psnr_two_files
from bitrate import compute_bitrate
import time


def encoding(encoder, wav_file, temp_file_path_gmpg, record_property):
    print("--Encoding")
    t_start = time.time()
    subprocess.run([encoder, "-f", wav_file, '-o', temp_file_path_gmpg])
    duration = time.time() - t_start
    record_property("encoder_duration_s", duration)
    print("Enconder time: "+str(duration))


def synthesizing(synthesizer, temp_file_path_gmpg, temp_file_path_wav, record_property):
    print("--Synthesizing")
    t_start = time.time()
    subprocess.run([synthesizer, "-f", temp_file_path_gmpg, '-o', temp_file_path_wav])
    duration = time.time() - t_start
    record_property("synthesizer_duration_s", duration)
    print("Synthesizer time: "+str(duration))


def test_psnrs(wav_file_psnr, encoder, synthesizer, autopad, record_property):
    record_property("file", wav_file_psnr[0])
    tmpdirname = tempfile.TemporaryDirectory()
    tmp_wav_file = os.path.basename(wav_file_psnr[0])
    gmpg_file = tmp_wav_file.split('.')[0] + ".gmpg"
    temp_file_path_gmpg = os.path.join(tmpdirname.name, gmpg_file)
    temp_file_path_wav = os.path.join(tmpdirname.name, tmp_wav_file)

    encoding(encoder, wav_file_psnr[0], temp_file_path_gmpg, record_property)

    # ToDo : compute on binary file
    bit_rate = compute_bitrate(wav_file_psnr[0], temp_file_path_gmpg)
    record_property("bitrate_kbps", bit_rate)

    synthesizing(synthesizer, temp_file_path_gmpg, temp_file_path_wav, record_property)

    print("--PSNR")
    psnr = psnr_two_files(wav_file_psnr[0], temp_file_path_wav, autopad)
    psnr = round(psnr, 2)
    print("psnr:" + str(psnr))
    record_property("psnr", psnr)
    if wav_file_psnr[1] is not None:
        record_property("psnr_ref", wav_file_psnr[1])
        assert psnr >= wav_file_psnr[1]
