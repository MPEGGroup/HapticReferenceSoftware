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
    book_cumulative.resize(l_book+1);
    book_cumulative[0] = 0;
    book_cumulative[1] = book[0];
    book_cumulative[2] = book[1] << 1;
    for(int i=2; i<l_book; i++){
        book[i] = book[i-1] << 1;
        book_cumulative[i+1] = book_cumulative[i] << 1;
    }

}


auto WaveletEncoder::encodeSignal(std::vector<double> &sig_time, int bitbudget, double f_cutoff, Band &band) -> bool {

  int numBlocks = (int)ceil((double)sig_time.size() / (double)bl);
  band.setBandType(BandType::Wave);
  band.setEncodingModality(EncodingModality::Quantized);
  band.setLowerFrequencyLimit((int)f_cutoff);
  band.setUpperFrequencyLimit((int)fs);
  band.setWindowLength(bl);

  int pos_effect = 0;

  for (int b = 0; b < numBlocks; b++) {
    Effect effect;
    auto start = sig_time.begin() + (long long)b * bl;
    auto end = start + bl;

    std::vector<double> block_time(bl, 0);
    if (end - sig_time.begin() > sig_time.size()) {
      end = sig_time.end();
    }
    std::copy(start, end, block_time.begin());
    double scalar = 0;
    std::vector<double> block_quant = encodeBlock(block_time, bitbudget, scalar);
    int pos = 0;
    for (auto v : block_quant) {
      Keyframe keyframe(pos, (float)v, 0);
      effect.addKeyframe(keyframe);
      pos++;
    }
    Keyframe keyframe(bl, (float)scalar, 0); // add scalar of block to block data for now
    effect.addKeyframe(keyframe);
    effect.setPosition(pos_effect);
    band.addEffect(effect);
    pos_effect += bl;
  }
  return true;
}

auto WaveletEncoder::encodeBlock(std::vector<double> &block_time, int bitbudget, double &scalar) -> std::vector<double> {

    std::vector<double> block_dwt(bl, 0);
    Wavelet wavelet;
    wavelet.DWT(block_time, dwtlevel, block_dwt);
    modelResult pm_result = pm.getSMR(block_time);

    std::vector<double> block_dwt_quant(bl,0);
    std::vector<int> block_intquant(bl,0);
    std::vector<double> SNR(book.size(),0);
    std::vector<double> MNR(book.size(),0);
    std::vector<double> noiseenergy(book.size(),0);
    std::vector<int> bitalloc(book.size(),0);
    int bitalloc_sum = 0;

    double qwavmax = 0;
    std::vector<unsigned char> bitwavmax;
    bitwavmax.reserve(WAVMAXLENGTH);
    maximumWaveletCoefficient(block_dwt,qwavmax,bitwavmax);

    //Quantization
    int i = 0;
    for(int block=0; block<book.size(); block++){
        bitalloc[block] = 0;
        noiseenergy[block] = 0;
        for(; i<book_cumulative[block+1]; i++){
            noiseenergy[block] += pow(block_dwt[i]-block_dwt_quant[i],2);
        }
    }


    while(bitalloc_sum < bitbudget){

        updateNoise(pm_result.bandenergy,noiseenergy,SNR,MNR,pm_result.SMR);
        for(int i=0; i<book.size(); i++){
            if(bitalloc[i]>=MAXBITS){
                MNR[i] = INFINITY;
            }
        }
        size_t index = findMinInd(MNR);
        if(bitalloc_sum-bitalloc[book.size()-1] >= MAXBITS*dwtlevel){
            int temp = bitalloc[book.size()-1];
            bitalloc[book.size()-1] = bitbudget - MAXBITS*dwtlevel;
            bitalloc_sum +=  bitalloc[book.size()-1] - temp;
        }else{
            bitalloc[index]++;
            bitalloc_sum++;
        }

        uniformQuant(block_dwt, book_cumulative[index], qwavmax, bitalloc[index], book[index],
                     block_dwt_quant);

        noiseenergy[index] = 0;
        int i = book_cumulative[index];
        for(; i<book_cumulative[index+1]; i++){
            noiseenergy[index] += pow(block_dwt[i]-block_dwt_quant[i],2);
        }

    }


    //scale signal to int values
    int bitmax = findMax(bitalloc);
    int intmax = 1 << bitmax;
    double multiplicator = (double)intmax/(double)qwavmax;
    //double multiplicator = (double)1 / qwavmax;

    std::vector<double> test(bl, 0);
    wavelet.inv_DWT(block_dwt_quant, dwtlevel, test);
    /*for (auto v : block_dwt) {
      std::cout << v << std::endl;
    }*/

    if (qwavmax != 0) {
      for (int i = 0; i < bl; i++) {
        block_intquant[i] = (int)round((block_dwt_quant[i] * multiplicator));
        block_dwt_quant[i] = block_dwt_quant[i] / qwavmax;
      }
    }
    scalar = qwavmax;
    //std::cout << "scalar encoder: " << scalar << std::endl;
    return block_dwt_quant;
}

void WaveletEncoder::maximumWaveletCoefficient(std::vector<double> &sig, double &qwavmax,
                                               std::vector<unsigned char> &bitwavmax) {

    double wavmax = findMax(sig);

    int integerpart = 0;
    int integerbits = 0;
    int fractionbits = 0;
    char mode = 0;
    quantMode m = {0,0};
    if(wavmax<1){
        fractionbits = FRACTIONBITS_0;
        m.integerbits = 0;
        m.fractionbits = FRACTIONBITS_0; 
    }else{
        integerpart = 1;
        m.integerbits = 3;
        m.fractionbits = 4; 
        mode = 1;
    }

    qwavmax = maxQuant(wavmax-(double)integerpart,m) + integerpart;
    bitwavmax.clear();
    bitwavmax.reserve(WAVMAXLENGTH);
    bitwavmax.push_back(mode);
    de2bi((int)((qwavmax-(double)integerpart)*pow(2,(double)m.fractionbits)),bitwavmax,m.integerbits+m.fractionbits);


}

void WaveletEncoder::updateNoise(std::vector<double> &bandenergy, std::vector<double> &noiseenergy, std::vector<double> &SNR, std::vector<double> &MNR, std::vector<double> &SMR){

    for(int i=0; i<book.size(); i++){
        SNR[i] = LOGFACTOR*log10(bandenergy[i]/noiseenergy[i]);
        MNR[i] = SNR[i] - SMR[i];
    }
}

void WaveletEncoder::uniformQuant(std::vector<double> &in, size_t start, double max, int bits, size_t length, std::vector<double> &out) {
    double delta = max/(1<<bits);
    double max_q = delta * ((1<<bits)-1);
    for(size_t i=start; i<start+length; i++){
        double sign = sgn(in[i]);
        //std::cout << "quant: " << floor(abs(in[i]) / delta + QUANT_ADD) << ", " << in[i] << ", " << abs(in[i]) / delta << ", " << delta << ", " << max << std::endl;
        if (max == 0) {
          out[i] = 0;
        } else {
          double q = sign * delta * floor(abs(in[i]) / delta + QUANT_ADD);
          if (abs(q) > max_q) {
            out[i] = sign * max_q;
          } else {
            out[i] = q;
          }
        }
    }
}

auto WaveletEncoder::maxQuant(double in, quantMode m) -> double {

    double max = ((double)(1<<(m.integerbits+m.fractionbits))-1) / (1<<m.fractionbits);

    double q = in;
    if(q>=max){
        q = sgn(q) * max * MAXQUANTFACTOR;
    }
    double delta = pow(2,(double)-m.fractionbits);
    return ceil(abs(q)/delta) * delta;

}

template <class T>
auto WaveletEncoder::findMax(std::vector<T> &data) -> T {
    T max = data[0];

    for(size_t i = 1; i<data.size(); i++){
        if(data[i]>max){
            max = data[i];
        }
    }

    return max;
}

auto WaveletEncoder::findMinInd(std::vector<double> &data) -> size_t {
    double min = data[0];
    size_t index = 0;
    for(int i = 1; i<data.size(); i++){
        if(data[i]<min){
            min = data[i];
            index = i;
        }
    }

    return index;
}

auto WaveletEncoder::sgn(double val) -> double {
    return (double)((double)0 < val) - (double)(val < (double)0);
}

void WaveletEncoder::de2bi(int val, std::vector<unsigned char> &outstream, int length){
    int n = length;
    while (n > 0) {
        outstream.push_back((unsigned char)val % 2);
        val = val >> 1;
        n--;
    }
}

} // namespace haptics::encoder
