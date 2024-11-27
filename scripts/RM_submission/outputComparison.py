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


def list_files(dir):
    r = []
    for root, dirs, files in os.walk(dir):
        for name in files:
            filepath = root + os.sep + name
            if filepath.endswith(".wav"):
                r.append(os.path.join(root, name))
    return r


if __name__ == "__main__":
    folder1 = "C:\\Users\\gurvan.lecuyer\\Documents\\Code\\rm0\\scripts\\RM_submission\\out"
    folder2 = "C:\\Users\\gurvan.lecuyer\\Documents\\Code\\ReferenceSoftwarePhase1\\scripts\\RM_submission\\out"
    fileList1 = list_files(folder1)
    fileList2 = list_files(folder2)
    nbFiles = len(fileList1)
    if(nbFiles == len(fileList2)):
        with open('outputComparison.csv', 'w+', newline='') as csvFile:
            writer = csv.writer(csvFile)
            writer.writerow(["Files", "PSNR"])
            print(datetime.now().strftime("[ %Hh : %Mm : %Ss ] => START\n"))
            for i in range(nbFiles):
                print(fileList1[i])
                psnrValue = psnr_two_files(fileList1[i], fileList2[i], True)
                csvRow=[fileList1[i]]
                csvRow.append(psnrValue)
                writer.writerow(csvRow)

