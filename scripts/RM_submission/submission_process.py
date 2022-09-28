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
TEST_EFFECT_KEYS = [
    "short_effects",
    "long_effects",
    "kinesthetic_effects"
]
TYPE_KEY = "type"
EXTENSION_KEY = "extension"
NAME_KEY = "name"
HAPTIC_FILE_PATH_KEY = "haptic_file_path"
REFERENCE_FILE = "reference_file"
MAIN_FOLDER_KEY = "main_folder"
COMPARISON_DATA_KEY = "comparison_data"
REFERENCE_BITRATEPSNR_KEY = "reference_bitratePSNR"

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

def addPadding(input_wav_file, output_wav_file, padding_duration):
    data, sample_rate = read(input_wav_file)
    if not sample_rate == 8000:
        print("Sample rate should be 8kHz")
    if data.ndim > 1:
        padding = np.zeros((sample_rate * padding_duration, data.shape[1]))
        data = np.vstack((padding, data, padding))
    else:
        padding = np.zeros(sample_rate * padding_duration)
        data = np.concatenate((padding, data, padding))
    write(output_wav_file, data, sample_rate, format='WAV', subtype='PCM_16')

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


def bjontegaard(rateRef, psnrRef, rateTest, psnrTest):
    if rateRef == rateTest:
        return [0,0]

    else:
        lrateRef = np.log(rateRef)
        lrateTest = np.log(rateTest)
    
        pRef0 = np.polyfit(lrateRef, psnrRef, 2)
        pTest0 = np.polyfit(lrateTest, psnrTest, 2)
    
        min_int0 = max(min(lrateRef), min(lrateTest))
        max_int0 = min(max(lrateRef), max(lrateTest))
    
        p_intRef0 = np.polyint(pRef0)
        p_intTest0 = np.polyint(pTest0)
    
        intRef0 = np.polyval(p_intRef0, max_int0) - np.polyval(p_intRef0, min_int0)
        intTest0 = np.polyval(p_intTest0, max_int0) - np.polyval(p_intTest0, min_int0)

        avg_diffPSNR = (intTest0 - intRef0)/(max_int0 - min_int0)

        pRef1 = np.polyfit(psnrRef, lrateRef, 2)
        pTest1 = np.polyfit(psnrTest, lrateTest, 2)
    
        min_int1 = max(min(psnrRef), min(psnrTest))
        max_int1 = min(max(psnrRef), max(psnrTest))
    
        p_intRef1 = np.polyint(pRef1)
        p_intTest1 = np.polyint(pTest1)
    
        intRef1 = np.polyval(p_intRef1, max_int1) - np.polyval(p_intRef1, min_int1)
        intTest1 = np.polyval(p_intTest1, max_int1) - np.polyval(p_intTest1, min_int1)
    
        avg_exp_diff1 = (intTest1 - intRef1)/(max_int1 - min_int1)
        avg_diffBitrate = (np.exp(avg_exp_diff1) - 1)*100

        return [avg_diffPSNR, avg_diffBitrate]


def main():
    print(datetime.now().strftime("[ %Hh : %Mm : %Ss ] => START\n"))
    with open(config_file, 'r') as file_stream:
        config = json.load(file_stream)

    checkSoftwarePath(config)
    if os.path.exists(output_folder):
        shutil.rmtree(output_folder)
    generateOutputFolderTree(output_folder)

    if REFERENCE_FILES_KEY not in config:
        return

    with open("logs.txt", 'w') as log_file:
        with open('bitratePSNR.csv', 'w', newline='') as csvFile:
            writer = csv.writer(csvFile)
            firstRow = ["File", "Test set", "Type"]
            for current_bitrate in bitrates:
                firstRow.append(str(current_bitrate)+"kbps bitrate")
                firstRow.append(str(current_bitrate)+"kbps psnr")
            writer.writerow(firstRow)
            for testId in range(1, 4):
                effects_key = TEST_EFFECT_KEYS[testId-1]
                if effects_key not in config[REFERENCE_FILES_KEY]:
                    continue
                for my_effect in config[REFERENCE_FILES_KEY][effects_key]:
                    if TYPE_KEY not in my_effect or NAME_KEY not in my_effect or HAPTIC_FILE_PATH_KEY not in my_effect:
                        continue
                    if filter_by_type and filter_by_type != my_effect[TYPE_KEY]:
                        continue
                    csvRow = [f"{my_effect[NAME_KEY]}", f"{my_effect[TYPE_KEY]}1_{testId}", f"{my_effect[EXTENSION_KEY]}"]
                    for current_bitrate in bitrates:
                        formatted_output_name = f"{my_effect[TYPE_KEY]}1_{testId}fvt_{CRM_version}_{current_bitrate}_{my_effect[NAME_KEY]}"
                        input_file_path = my_effect[HAPTIC_FILE_PATH_KEY]
                        reference_file_path = my_effect[REFERENCE_FILE]
                        if MAIN_FOLDER_KEY in config[REFERENCE_FILES_KEY]:
                            input_file_path = os.path.join(config[REFERENCE_FILES_KEY][MAIN_FOLDER_KEY], input_file_path)
                            reference_file_path = os.path.join(config[REFERENCE_FILES_KEY][MAIN_FOLDER_KEY], reference_file_path)
                        if(not os.path.exists(input_file_path)):
                            print(f"FILE NOT FOUND: {input_file_path}")
                            continue
                        hmpg_file_path = os.path.join(output_folder, rf"{my_effect[TYPE_KEY]}1_{testId}/HMPG/{formatted_output_name}.hmpg")
                        hjif_file_path = os.path.join(output_folder, rf"{my_effect[TYPE_KEY]}1_{testId}/HJIF/{formatted_output_name}.hjif")
                        nopad_file_path = os.path.join(output_folder, rf"{my_effect[TYPE_KEY]}1_{testId}/WAV_nopad/{formatted_output_name}_nopad.wav")
                        pad_file_path = os.path.join(output_folder, rf"{my_effect[TYPE_KEY]}1_{testId}/WAV_pad/{formatted_output_name}_pad.wav")

                        print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Encoder ({current_bitrate}kbs) on : {my_effect[NAME_KEY]}"))
                        encoding_command = f"{os.path.join(config[RM_INSTALL_DIR], config[ENCODER_PATH_KEY])} -f {input_file_path} -o {hmpg_file_path} -kb {current_bitrate} -cf {cutoff} --binary --refactor"
                        if(disable_wavelet):
                            encoding_command += " --disable-wavelet"
                        elif(disable_vectorial):
                            encoding_command += " --disable-vectorial"
                        print(encoding_command)
                        subprocess.run(encoding_command, stdout=log_file)
                        print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Decoder ({current_bitrate}kbs) on : {my_effect[NAME_KEY]}"))
                        subprocess.run(f"{os.path.join(config[RM_INSTALL_DIR], config[DECODER_PATH_KEY])} -f {hmpg_file_path} -o {hjif_file_path}", stdout=log_file)
                        print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Synthesizer (nopad | {current_bitrate}kbs) on : {my_effect[NAME_KEY]}"))
                        subprocess.run(f"{os.path.join(config[RM_INSTALL_DIR], config[SYNTHESIZER_PATH_KEY])} -f {hjif_file_path} -o {nopad_file_path} --generate_ohm", stdout=log_file)
                        if padding:
                            print(datetime.now().strftime(f"[ %Hh : %Mm : %Ss ] => Padding (pad {padding}s| {current_bitrate}kbs) on : {my_effect[NAME_KEY]}"))
                            addPadding(nopad_file_path, pad_file_path, padding)
                        bitrate = compute_bitrate(reference_file_path, hmpg_file_path)
                        psnr = psnr_two_files(nopad_file_path, reference_file_path, True)
                        csvRow.append(bitrate)
                        csvRow.append(psnr)
                    writer.writerow(csvRow)

    
    if compute_bjontegaard:
        try:
            os.makedirs('Bjontegaard/Plots')
        except FileExistsError:
            pass
        
        with open('bitratePSNR.csv') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader)
            bitrate_bjontegaard = []
            psnr_bjontegaard = []
            for row in reader:
                bitrate_temp = []
                psnr_temp = []
                for i in range(2, len(row),2):
                    bitrate_temp.append(float(row[i]))
                    psnr_temp.append(float(row[i+1]))
                bitrate_bjontegaard.append(bitrate_temp)
                psnr_bjontegaard.append(psnr_temp)
            csvfile.close()
            
        with open(os.path.join(config[REFERENCE_FILES_KEY][MAIN_FOLDER_KEY],config[REFERENCE_FILES_KEY][REFERENCE_BITRATEPSNR_KEY])) as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader)
            signal_name = []
            signal_type = []
            bitrate_ref = []
            psnr_ref = []
            for row in reader:
                signal_name.append(row[0])
                signal_type.append(row[1])
                bitrate_temp = []
                psnr_temp = []
                for i in range(2, len(row),2):
                    bitrate_temp.append(float(row[i]))
                    psnr_temp.append(float(row[i+1]))
                bitrate_ref.append(bitrate_temp)
                psnr_ref.append(psnr_temp)
            csvfile.close()
            
        dataWrite = []
        for i in range(len(bitrate_ref)):
            dataWrite.append([signal_name[i], signal_type[i],
                              bjontegaard(bitrate_ref[i], psnr_ref[i], bitrate_bjontegaard[i], psnr_bjontegaard[i])[0],
                              bjontegaard(bitrate_ref[i], psnr_ref[i], bitrate_bjontegaard[i], psnr_bjontegaard[i])[1]])

            plt.plot(bitrate_ref[i], psnr_ref[i])
            plt.plot(bitrate_bjontegaard[i], psnr_bjontegaard[i])
            plt.xlabel('bitrate')
            plt.ylabel('psnr')
            plt.legend(['Reference','Residual'])
            plt.title(signal_name[i])
            plt.savefig('Bjontegaard/Plots/'+signal_name[i]+'.png')
            plt.close()

        with open('Bjontegaard/bjontegaard.csv', 'w+', newline='') as csvFile:
            writer = csv.writer(csvFile)
            writer.writerow(["File", "Type", "PSNR difference", "bitrate savings (%)"])
            writer.writerows(dataWrite)
            csvfile.close()

        print(datetime.now().strftime("\n[ %Hh : %Mm : %Ss ] => FINISH"))


if __name__ == "__main__":
    DEFAULT_OUTPUT = "./out"
    DEFAULT_BITRATES = [2, 8, 16, 64]
    DEFAULT_PAD = 1
    DEFAULT_CUTOFF_FREQUENCY = 72.5
    DEFAULT_DISABLE_WAVELET = False
    DEFAULT_DISABLE_VECTORIAL = False
    DEFAULT_BJONTEGAARD = False
    DEFAULT_FILTER_BY_TYPE = ""

    parser = argparse.ArgumentParser()
    parser.add_argument("config_file", type=str, help="input config file in JSON format")
    parser.add_argument("CRM_version", type=str, help="version of the CRM format")
    parser.add_argument("--cutoff", type=float, default=DEFAULT_CUTOFF_FREQUENCY, help="Cutoff frequency. Default is 72.5")
    parser.add_argument("-o", "--output", type=str, default=DEFAULT_OUTPUT, help=f"output folder (default is `{DEFAULT_OUTPUT}`)")
    parser.add_argument("-b", "--bitrates", type=check_positive, nargs='+', default=DEFAULT_BITRATES, help=f"bitrates used for the encoding (default is `[2, 16, 64]`)")
    parser.add_argument("--padding", type=check_positive, default=DEFAULT_PAD, help=f"pad in seconds used for the syntheziser")
    parser.add_argument("--filter_by_type", type=str, help=f"Process input files matching with this type (if not set every file will be proceed)")
    parser.add_argument("--disable_wavelet", type=bool, default=DEFAULT_DISABLE_WAVELET, help=f"Desables wavelet encoding")
    parser.add_argument("--disable_vectorial", type=bool, default=DEFAULT_DISABLE_VECTORIAL, help=f"Desables wavelet encoding")
    parser.add_argument("--bjontegaard", type=bool, default=DEFAULT_BJONTEGAARD, help=f"Calculates Bjontegaard's metrics and visualy display the difference")
    args = parser.parse_args()

    config_file = args.config_file
    CRM_version = args.CRM_version
    cutoff = args.cutoff
    disable_wavelet = args.disable_wavelet
    disable_vectorial = args.disable_vectorial
    output_folder = args.output if args.output and not args.output.isspace() else DEFAULT_OUTPUT
    output_folder = os.path.abspath(output_folder)
    bitrates = args.bitrates if args.bitrates else DEFAULT_BITRATES
    padding = args.padding
    filter_by_type = args.filter_by_type
    compute_bjontegaard = args.bjontegaard

    assert not config_file.isspace(), "config_file should be provided"
    assert os.path.isfile(config_file), "config_file should be a file"
    assert (os.path.splitext(config_file)[1] == ".json"), "config_file should be a json file"
    assert os.path.exists(config_file), "config_file should be an existing file"

    main()
