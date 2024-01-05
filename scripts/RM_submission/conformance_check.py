# The copyright in this software is being made available under the BSD
# License, included below. This software may be subject to other third party
# and contributor rights, including patent rights, and no such rights are
# granted under this license.

# Copyright (c) 2010-2021, ISO/IEC
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of the ISO/IEC nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.

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

from datetime import datetime

import numpy as np
import argparse
import json
import os
import shutil
import subprocess
import csv
import matplotlib.pyplot as plt

from soundfile import read, write

RM_INSTALL_DIR = "RM_install_dir"
ENCODER_PATH_KEY = "encoder_path"
DECODER_PATH_KEY = "decoder_path"
SYNTHESIZER_PATH_KEY = "synthesizer_path"
REFERENCE_FILES_KEY = "reference_files"
TYPE_KEY = "type"
MODALITY_KEY = "modality"
EXTENSION_KEY = "extension"
NAME_KEY = "name"
HAPTIC_FILE_PATH_KEY = "haptic_file_path"
REFERENCE_FILE_KEY = "reference_file"
MAIN_FOLDER_KEY = "main_folder"
CONFORMANCE_FILES_KEY = "conformance_files"
CONFORMANCE_TEST_SET_KEYS = [
    "schemas_checks",
    "semantic_checks"
]

# implements PSNR metric (verified)
def psnr(x, y):
    mse = (np.square(np.subtract(x, y))).mean()
    if mse == 0:
        return 100
    # xmax and xmin values are respectively the maximum and minimum 
    # possible values of the signal. Here the haptic signal is normalized 
    # between -1 and 1
    xmax = 1
    xmin = -1
    max_val = xmax - xmin
    val = 10 * np.log10((max_val ** 2) / mse)
    return val


def psnr_two_files(original, decompressed, autopad=False):
    # read data
    data_original, sample_rate_original = read(original)
    data_decompressed, sample_rate_decompressed = read(decompressed)
    
    # check samplerate
    if(sample_rate_original != sample_rate_decompressed):
        sys.exit("[!] The sample rates of the input files are not the same.")

    # check number of channels
    num_chan_original = 1
    num_chan_decompressed = 1

    if len(data_original.shape) > 1:
        num_chan_original = data_original.shape[1]

    if len(data_decompressed.shape) > 1:
        num_chan_decompressed = data_decompressed.shape[1]

    if(num_chan_decompressed != num_chan_original):
        sys.exit("[!] The number of channels is not the same.")

    # check file length + channels
    if(data_original.shape != data_decompressed.shape):
        # zero-pad signals to the right (start is assumed to be aligned!)
        if autopad and (data_original.shape[0] != data_decompressed.shape[0]):
            padLen = data_decompressed.shape[0] - data_original.shape[0]
            if num_chan_original == 1:
                zPad = np.zeros( abs(padLen) )
            else:
                s = (abs(padLen), num_chan_original)
                zPad = np.zeros(s)
            
            # if the sample length difference is negative, the decompressed file is shorter
            if padLen < 0:
                # zero-pads the decompressed file
                data_decompressed = np.concatenate((data_decompressed, zPad))
            
            # if the sample length difference is positive, the original file is shorter
            elif padLen > 0:
                # zero-pads the original file
                data_original = np.concatenate((data_original, zPad))
            
        else:
            sys.exit("[!] The files have different sizes. Automatic padding is off. Exiting.")
        
    # calculate psnr (dB)
    # cast to python float to enable serialization
    return float(psnr(data_original, data_decompressed))


# bitrate kbps
def compute_bitrate(wav_file, encoded_file):
    data_original, sample_rate_original = read(wav_file)
    duration_s = len(data_original) / sample_rate_original
    num_channels = 1
    if len(data_original.shape) > 1:
        num_channels = data_original.shape[1]
    with open(encoded_file, 'rb') as f:
        bit_size = len(f.read()) * 8
    bit_rate = bit_size / duration_s / num_channels / 1000.0
    return bit_rate


def check_positive(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive int value" % value)
    return ivalue


def checkSoftwarePath(config: dict):
    def _check_software(dir_key: str,path_key: str):
        assert dir_key in config, f"{dir_key} should be configured"
        assert path_key in config, f"{path_key} should be configured"
        path = os.path.join(config[dir_key],config[path_key])
        assert os.path.isfile(path), f"{path_key} should be a file"
        assert os.path.exists(config_file), f"{path_key} should be an existing file"
        assert os.access(config_file, os.X_OK), f"{path_key} should be executable"

    _check_software(RM_INSTALL_DIR, ENCODER_PATH_KEY)
    _check_software(RM_INSTALL_DIR, DECODER_PATH_KEY)
    _check_software(RM_INSTALL_DIR, SYNTHESIZER_PATH_KEY)


def generateOutputFolderTree(folder: str):
    os.makedirs(folder)
    for testType in ["Test", "Training", "Evaluation"]:
        for testId in range(1, 4):
            test_directory = os.path.join(folder, rf"{testType}1_{testId}")
            os.mkdir(test_directory)
            os.mkdir(os.path.join(test_directory, rf"HMPG"))
            os.mkdir(os.path.join(test_directory, rf"HJIF"))
            os.mkdir(os.path.join(test_directory, rf"WAV_nopad"))
            os.mkdir(os.path.join(test_directory, rf"WAV_pad"))


def main():
    print(datetime.now().strftime("[ %Hh : %Mm : %Ss ] => START\n"))
    with open(config_file, 'r') as file_stream:
        config = json.load(file_stream)

    checkSoftwarePath(config)
    if os.path.exists(output_folder):
        shutil.rmtree(output_folder)
    generateOutputFolderTree(output_folder)


    for conformance_check_type in CONFORMANCE_TEST_SET_KEYS:
        print("\n****** ",conformance_check_type," ******")
        for conformance_check in config[CONFORMANCE_FILES_KEY][conformance_check_type]:
            input_file_path = conformance_check[HAPTIC_FILE_PATH_KEY]
            if MAIN_FOLDER_KEY in config[CONFORMANCE_FILES_KEY]:
                if(input_file_path == ""):
                    continue
                input_file_path = os.path.join(config[CONFORMANCE_FILES_KEY][MAIN_FOLDER_KEY], input_file_path)
                if(not os.path.exists(input_file_path)):
                    print(f"FILE NOT FOUND: {input_file_path}")
                    continue
            print("\n",conformance_check[NAME_KEY])
            print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Encoder on : {input_file_path}"))
            subprocess.run(f"{os.path.join(config[RM_INSTALL_DIR], config[ENCODER_PATH_KEY])} -f {input_file_path} -o test.hjif")

if __name__ == "__main__":
    DEFAULT_FILTER_BY_TYPE = ""

    parser = argparse.ArgumentParser()
    parser.add_argument("config_file", type=str, help="input config file in JSON format")
    args = parser.parse_args()

    config_file = args.config_file
    output_folder = os.path.abspath("./out")
    assert not config_file.isspace(), "config_file should be provided"
    assert os.path.isfile(config_file), "config_file should be a file"
    assert (os.path.splitext(config_file)[1] == ".json"), "config_file should be a json file"
    assert os.path.exists(config_file), "config_file should be an existing file"

    main()
