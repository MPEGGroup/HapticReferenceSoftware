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
EXPECTED_OUTPUT_KEY = "expected_output"
REFERENCE_FILE_KEY = "reference_file"
MAIN_FOLDER_KEY = "main_folder"
CONFORMANCE_FILES_KEY = "conformance_files"
CONFORMANCE_TEST_SET_KEYS = [
    "schemas_checks",
    "semantic_checks"
]

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


def main():
    print(datetime.now().strftime("[ %Hh : %Mm : %Ss ] => START\n"))
    with open(config_file, 'r') as file_stream:
        config = json.load(file_stream)

    checkSoftwarePath(config)
    if os.path.exists(output_folder):
        shutil.rmtree(output_folder)


    #text_file = open("testlog.txt", "w")
    nb_tests = 0
    nb_success = 0
    check_fails = []
    for conformance_check_type in CONFORMANCE_TEST_SET_KEYS:
        print("\n****** ",conformance_check_type," ******")
        test_number = 1;
        #text_file.write("****** "+conformance_check_type+" ******\n")
        for conformance_check in config[CONFORMANCE_FILES_KEY][conformance_check_type]:
            input_file_path = conformance_check[HAPTIC_FILE_PATH_KEY]
            if MAIN_FOLDER_KEY in config[CONFORMANCE_FILES_KEY]:
                if(input_file_path == ""):
                    print("Test #",test_number,":\tTO DO\t|\t",conformance_check[NAME_KEY])
                    nb_tests+=1
                    test_number+=1
                    continue
                input_file_path = os.path.join(config[CONFORMANCE_FILES_KEY][MAIN_FOLDER_KEY], input_file_path)
                if(not os.path.exists(input_file_path)):
                    print(f"FILE NOT FOUND: {input_file_path}")
                    continue
            #print("\n",conformance_check[NAME_KEY])
            #print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Encoder on : {input_file_path}"))
            result = subprocess.run(f"{os.path.join(config[RM_INSTALL_DIR], config[ENCODER_PATH_KEY])} -f {input_file_path} -o test.hjif",shell=True, capture_output=True, text=True)
            valid = result.stderr.splitlines()==conformance_check[EXPECTED_OUTPUT_KEY].splitlines()
            if(not valid):
                check_fails.append("*********\n"+conformance_check_type +" #"+str(test_number)+" failed: "+conformance_check[NAME_KEY]+"\n")
                check_fails.append("- Output : \n"+result.stderr)
                check_fails.append("- Expected output: \n"+conformance_check[EXPECTED_OUTPUT_KEY])
            print("Test #",test_number,":\t",valid,"\t|\t",conformance_check[NAME_KEY])
            nb_tests+=1
            test_number+=1
            if(valid):
                nb_success+=1
            #text_file.write('\\n'.join(result.stderr.replace("\\","\\\\").splitlines())+"\n\n")
    #text_file.close()
    print("####### Conformance Results")
    if(nb_success == nb_tests):
        print("SUCCESS: ",nb_success,"/",nb_tests," valid tests")
    else:
        print("FAIL: ",nb_success,"/",nb_tests," valid tests")
        print("The following test failed:")
        print("\n".join(check_fails))

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
