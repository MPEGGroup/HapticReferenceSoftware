#!/usr/bin/env python3

# The copyright in this software is being made available under the BSD
# License, included below. This software may be subject to other third party
# and contributor rights, including patent rights, and no such rights are
# granted under this license.
#
# Copyright (c) 2010-2021, ISO/IEC
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
import numpy as np
from soundfile import read
from os import path
import argparse
import sys

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


def psnr_two_files(original, compressed, autopad=False):
    # read data
    data_original, sample_rate_original = read(original)
    data_compressed, sample_rate_compressed = read(compressed)
    
    # check samplerate
    if(sample_rate_original != sample_rate_compressed):
        sys.exit("[!] The sample rates of the input files are not the same.")

    # check number of channels
    num_chan_original = 1
    num_chan_compressed = 1

    if len(data_original.shape) > 1:
        num_chan_original = data_original.shape[1]

    if len(data_compressed.shape) > 1:
        num_chan_compressed = data_compressed.shape[1]

    if(num_chan_compressed != num_chan_original):
        sys.exit("[!] The number of channels is not the same.")

    # check file length + channels
    if(data_original.shape != data_compressed.shape):
        # zero-pad signals to the right (start is assumed to be aligned!)
        if autopad and (data_original.shape[0] != data_compressed.shape[0]):
            padLen = data_compressed.shape[0] - data_original.shape[0]
            if num_chan_original == 1:
                zPad = np.zeros( abs(padLen) )
            else:
                s = (abs(padLen), num_chan_original)
                zPad = np.zeros(s)
            
            # if the sample length difference is negative, the compressed file is shorter
            if padLen < 0:
                # zero-pads the compressed file
                data_compressed = np.concatenate((data_compressed, zPad))
                print("[.] Adding automatic zero-padding of", abs(padLen), "samples to the compressed file.")
            
            # if the sample length difference is positive, the original file is shorter
            elif padLen > 0:
                # zero-pads the original file
                data_original = np.concatenate((data_original, zPad))
                print("[.] Adding automatic zero-padding of", abs(padLen), "samples to the original file.")
            
            # Theoretically unreachable - as for the length check above, but implemented for logic completeness.
            else:
                print("[.] Both signals are equally long. No padding added.")
        else:
            sys.exit("[!] The files have different sizes. Automatic padding is off. Exiting.")
        
    # calculate psnr (dB)
    # cast to python float to enable serialization
    return float(psnr(data_original, data_compressed))

if __name__ == "__main__":  
    # parse arguments
    parser = argparse.ArgumentParser(description='Computing PSNR')
    parser.add_argument("original", help="path to the original file")
    parser.add_argument("compressed", help="path to the compressed file")
    parser.add_argument('--autopad', help='turns on automatic padding, \
                                           else PSNR won\'t be computed with \
                                           unequal signal lengths',
                        action="store_true")
    args = parser.parse_args()
    if(not path.exists(args.original)):
        sys.exit(args.original + " file could not be found")
    if(not path.exists(args.compressed)):
        sys.exit(args.compressed + " file could not be found")

    psnr_val = psnr_two_files(args.original, args.compressed, args.autopad)
    print("The PSNR is:", psnr_val, "dB")
