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

#include "../include/WaveletEncoder.h"

namespace haptics::encoder {

WaveletEncoder::WaveletEncoder(int bl_new, int fs_new)
    : pm(bl_new, fs_new), bl(bl_new), fs(fs_new), dwtlevel((int)log2((double)bl / 4)) {

  int l_book = dwtlevel + 1;
  book.resize(l_book);
  book[0] = bl >> dwtlevel;
  book[1] = book[0];
  book_cumulative.resize(l_book + 1);
  book_cumulative[0] = 0;
  book_cumulative[1] = book[0];
  book_cumulative[2] = book[1] << 1;
  for (int i = 2; i < l_book; i++) {
    book[i] = book[i - 1] << 1;
    book_cumulative[i + 1] = book_cumulative[i] << 1;
  }
}

auto WaveletEncoder::encodeSignal(std::vector<double> &sig_time, int bitbudget, double f_cutoff,
                                  Band &band, const unsigned int timescale) -> bool {
  int numBlocks = (int)ceil((double)sig_time.size() / (double)bl);
  band.setBandType(BandType::WaveletWave);
  band.setLowerFrequencyLimit((int)f_cutoff);
  band.setUpperFrequencyLimit((int)fs);
  band.setBlockLength(bl * static_cast<int>(timescale) / fs);
  int pos_effect = 0;
  long add_start = 0;
  unsigned int add_end = bl;
  for (int b = 0; b < numBlocks; b++) {
    Effect effect;
    std::vector<double> block_time(bl, 0);
    if (add_end > sig_time.size()) {
      add_end = (long)sig_time.size();
    }
    std::copy(sig_time.begin() + add_start, sig_time.begin() + add_end, block_time.begin());

    double scalar = 0;
    int maxbits = 0;
    encodeBlock(block_time, bitbudget, scalar, maxbits, effect.getWaveletBitstream());

    effect.setPosition(pos_effect);
    band.addEffect(effect);
    pos_effect += (int)band.getBlockLength().value();
    add_start += bl;
    add_end += bl;
  }
  return true;
}

void WaveletEncoder::encodeBlock(std::vector<double> &block_time, int bitbudget, double &scalar,
                                 int &maxbits, std::vector<unsigned char> &bitstream) {

  std::vector<double> block_dwt(bl, 0);
  Wavelet wavelet;
  wavelet.DWT(block_time, dwtlevel, block_dwt);
  modelResult pm_result = pm.getSMR(block_time);

  std::vector<double> block_dwt_quant(bl, 0);
  std::vector<int> block_intquant(bl, 0);
  std::vector<double> SNR(book.size(), 0);
  std::vector<double> MNR(book.size(), 0);
  std::vector<double> noiseenergy(book.size(), 0);
  std::vector<int> bitalloc(book.size(), 0);
  int bitalloc_sum = 0;

  double qwavmax = 0;
  std::vector<unsigned char> bitwavmax;
  bitwavmax.reserve(spiht::WAVMAXLENGTH);
  maximumWaveletCoefficient(block_dwt, qwavmax, bitwavmax);

  // Quantization
  int i = 0;
  for (uint32_t block = 0; block < book.size(); block++) {
    bitalloc[block] = 0;
    noiseenergy[block] = 0;
    for (; i < book_cumulative[block + 1]; i++) {
      noiseenergy[block] += pow(block_dwt[i] - block_dwt_quant[i], 2);
    }
  }

  if (bitbudget > ((int)book.size() * spiht::MAXBITS)) {
    bitbudget = ((int)book.size() * spiht::MAXBITS);
  }

  while (bitalloc_sum < bitbudget) {

    updateNoise(pm_result.bandenergy, noiseenergy, SNR, MNR, pm_result.SMR);
    for (uint32_t i = 0; i < book.size(); i++) {
      if (bitalloc[i] >= spiht::MAXBITS) {
        MNR[i] = INFINITY;
      }
    }
    size_t index = findMinInd(MNR);
    if (bitalloc_sum - bitalloc[book.size() - 1] >= spiht::MAXBITS * dwtlevel) {
      int temp = bitalloc[book.size() - 1];
      bitalloc[book.size() - 1] = bitbudget - spiht::MAXBITS * dwtlevel;
      bitalloc_sum += bitalloc[book.size() - 1] - temp;
    } else {
      bitalloc[index]++;
      bitalloc_sum++;
    }

    uniformQuant(block_dwt, book_cumulative[index], qwavmax, bitalloc[index], book[index],
                 block_dwt_quant);

    noiseenergy[index] = 0;
    int i = book_cumulative[index];
    for (; i < book_cumulative[index + 1]; i++) {
      noiseenergy[index] += pow(block_dwt[i] - block_dwt_quant[i], 2);
    }
  }

  // scale signal to int values
  int bitmax = findMax(bitalloc);
  int intmax = 1 << bitmax;
  double multiplicator = (double)intmax / (double)qwavmax;

  if (qwavmax != 0) {
    for (int i = 0; i < bl; i++) {
      block_intquant[i] = (int)round((block_dwt_quant[i] * multiplicator));
      block_dwt_quant[i] = block_dwt_quant[i] / qwavmax;
    }
  }
  scalar = qwavmax;
  maxbits = bitmax;
  spihtEnc.encodeEffect(block_intquant, maxbits, scalar, bitstream);
}

void WaveletEncoder::maximumWaveletCoefficient(std::vector<double> &sig, double &qwavmax,
                                               std::vector<unsigned char> &bitwavmax) {

  double wavmax = findMax(sig);

  auto m = Spiht_Enc::getQuantMode(wavmax);
  int integerpart = 0;
  if (m.mode == 1) {
    integerpart = 1;
  }

  qwavmax = maxQuant(wavmax - (double)integerpart, m) + integerpart;
  Spiht_Enc::setBitwavmax(qwavmax, integerpart, m, bitwavmax);
}

void WaveletEncoder::maximumWaveletCoefficient(double qwavmax,
                                               std::vector<unsigned char> &bitwavmax) {

  auto m = Spiht_Enc::getQuantMode(qwavmax);
  int integerpart = 0;
  if (m.mode == 1) {
    integerpart = 1;
  }

  Spiht_Enc::setBitwavmax(qwavmax, integerpart, m, bitwavmax);
}

void WaveletEncoder::updateNoise(std::vector<double> &bandenergy, std::vector<double> &noiseenergy,
                                 std::vector<double> &SNR, std::vector<double> &MNR,
                                 std::vector<double> &SMR) {

  for (uint32_t i = 0; i < book.size(); i++) {
    SNR[i] = LOGFACTOR * log10(bandenergy[i] / noiseenergy[i]);
    MNR[i] = SNR[i] - SMR[i];
  }
}

void WaveletEncoder::uniformQuant(std::vector<double> &in, size_t start, double max, int bits,
                                  size_t length, std::vector<double> &out) {
  double delta = max / (1 << bits);
  double max_q = delta * ((1 << bits) - 1);
  for (size_t i = start; i < start + length; i++) {
    double sign = sgn(in[i]);
    if (max == 0) {
      out[i] = 0;
    } else {
      double q = sign * delta * floor(fabs(in[i]) / delta + QUANT_ADD);
      if (fabs(q) > max_q) {
        out[i] = sign * max_q;
      } else {
        out[i] = q;
      }
    }
  }
}

auto WaveletEncoder::maxQuant(double in, spiht::quantMode m) -> double {

  double max = ((double)(1 << (m.integerbits + m.fractionbits)) - 1) / (1 << m.fractionbits);

  double q = in;
  if (q >= max) {
    q = sgn(q) * max * MAXQUANTFACTOR;
  }
  double delta = pow(2, (double)-m.fractionbits);
  return ceil(fabs(q) / delta) * delta;
}

template <class T> auto WaveletEncoder::findMax(std::vector<T> &data) -> T {
  T max = fabs(data[0]);

  for (size_t i = 1; i < data.size(); i++) {
    if (fabs(data[i]) > max) {
      max = fabs(data[i]);
    }
  }

  return max;
}

auto WaveletEncoder::findMinInd(std::vector<double> &data) -> size_t {
  double min = data[0];
  size_t index = 0;
  for (uint32_t i = 1; i < data.size(); i++) {
    if (data[i] < min) {
      min = data[i];
      index = i;
    }
  }

  return index;
}

auto WaveletEncoder::sgn(double val) -> double {
  return (double)((double)0 < val) - (double)(val < (double)0);
}

void WaveletEncoder::de2bi(int val, std::vector<unsigned char> &outstream, int length) {
  int n = length;
  while (n > 0) {
    outstream.push_back((unsigned char)(val % 2));
    val = val >> 1;
    n--;
  }
}

} // namespace haptics::encoder
