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

import subprocess
import argparse
import os
import pathlib
import json
import sys

# To be used by the gitlab runner to retrieve data.
# Please contact the software coordinator to get access to mpeg content

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Computing PSNRs')
    parser.add_argument("--data_dir", help="local data directory", required=True)
    parser.add_argument("--ftp_info", help="JSON file with MPEG FTP info", required=True)
    parser.add_argument("--mpeg_user", help="MPEG login", required=True)
    parser.add_argument("--mpeg_pwd", help="MPEG password", required=True)
    
    args = parser.parse_args()
    if not os.path.exists(args.data_dir):
        pathlib.Path(args.data_dir).mkdir(parents=True, exist_ok=True)
    if not os.path.exists(args.ftp_info):
        sys.exit("FTP information missing.")

    ftp_info_file = open(args.ftp_info)
    ftp_info = json.load(ftp_info_file)
    ftp_address = ftp_info['ftp']
    #https_address = ftp_address.replace("ftp", "https")
    https_address = "https://mpegfs.int-evry.fr/"

    wget = ['wget', '-m', '-np', '-nH', '--cut-dir=2',
            '-P'+ args.data_dir,
            "--http-user="+args.mpeg_user,
            "--http-password="+args.mpeg_pwd,
            https_address]
    subprocess.run(wget)
