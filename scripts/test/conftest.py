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

import os
from pathlib import Path
import sys

def pytest_addoption(parser):
    parser.addoption('--autopad', help='turns on automatic padding, \
                                        else PSNR won\'t be computed with \
                                        unequal signal lengths',
                     action="store_true")
    parser.addoption("--install_dir", help="RM0 installation directory", required=True)
    parser.addoption("--data_dir", help="Data directory (from mpegcontent repo)", required=True)

def pytest_generate_tests(metafunc):
    install_dir = metafunc.config.getoption("install_dir")
    data_dir = metafunc.config.getoption("data_dir")

    if not os.path.exists(install_dir):
        sys.exit("Installation directory " + install_dir+" does not exist.")
    if not os.path.exists(data_dir):
        sys.exit("Data directory " + data_dir+" does not exist.")

    if "autopad" in metafunc.fixturenames:
        if metafunc.config.getoption("autopad"):
            metafunc.parametrize("autopad", [True])
        else:
            metafunc.parametrize("autopad", [False])

    encoder_path = os.path.join(install_dir, 'bin', 'Encoder')
    synthesizer_path = os.path.join(install_dir, 'bin', 'Synthesizer')

    list_wav_files = []
    for path in Path(data_dir).rglob('*.wav'):
        # cast to str because Path object is not serializable
        list_wav_files.append(str(path))

    if "encoder" in metafunc.fixturenames:
        metafunc.parametrize("encoder", [encoder_path])

    if "synthesizer" in metafunc.fixturenames:
        metafunc.parametrize("synthesizer", [synthesizer_path])

    if "wav_file" in metafunc.fixturenames:
        metafunc.parametrize("wav_file", list_wav_files)
