// RESAMPLE  Change the sampling rate of a signal.
//    Y = RESAMPLE(UpFactor, DownFactor, InputSignal, OutputSignal) resamples
//    the sequence in vector InputSignal at UpFactor/DownFactor times and stores
//    the resampled data to OutputSignal. OutputSignal is UpFactor/DownFactor
//    times the length of InputSignal. UpFactor and DownFactor must be positive
//    integers.

// This function is translated from Matlab's Resample funtion.

// Author: Haoqi Bai

#pragma once

#include "upfirdn.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

using std::vector;
namespace haptics::tools {

const double THRESHOLD_SIN = 0.000001;
const double FILTER_DIV = 2.0;
const double FILTER_INC = 0.5;

template <typename T> auto sinc(T x) -> T {
  if (std::abs(x - 0.0) < THRESHOLD_SIN) {
    return 1;
  }
  return std::sin(M_PI * x) / (M_PI * x);
}

inline auto quotientCeil(int num1, int num2) -> int {
  if (num1 % num2 != 0) {
    return num1 / num2 + 1;
  }
  return num1 / num2;
}

template <typename T>
auto firls(int length, vector<T> freq, const vector<T> &amplitude) -> std::vector<T> {
  int freqSize = freq.size();
  int weightSize = freqSize / 2;

  vector<T> weight(weightSize, 1.0);

  int filterLength = length + 1;

  for (auto &it : freq) {
    it /= FILTER_DIV;
  }

  length = (filterLength - 1) / 2;
  bool Nodd = filterLength & 1;
  vector<T> k(length + 1);
  std::iota(k.begin(), k.end(), 0.0);
  if (!Nodd) {
    for (auto &it : k) {
      it += FILTER_INC;
    }
  }

  T b0 = 0.0;
  if (Nodd) {
    k.erase(k.begin());
  }

  vector<T> b(k.size(), 0.0);
  for (int i = 0; i < freqSize; i += 2) {
    auto Fi = freq[i];
    auto Fip1 = freq[i + 1];
    auto ampi = amplitude[i];
    auto ampip1 = amplitude[i + 1];
    auto wt2 = std::pow(weight[i / 2], 2);
    auto m_s = (ampip1 - ampi) / (Fip1 - Fi);
    auto b1 = ampi - (m_s * Fi);
    if (Nodd) {
      b0 += (b1 * (Fip1 - Fi)) + m_s / 2 * (std::pow(Fip1, 2) - std::pow(Fi, 2)) * wt2;
    }
    std::transform(b.begin(), b.end(), k.begin(), b.begin(), [m_s, Fi, Fip1, wt2](T b, T k) {
      return b + (m_s / (4 * std::pow(M_PI, 2)) *
                  (std::cos(2 * M_PI * Fip1) - std::cos(2 * M_PI * Fi)) / (std::pow(k, 2))) *
                     wt2;
    });
    std::transform(b.begin(), b.end(), k.begin(), b.begin(), [m_s, Fi, Fip1, wt2, b1](T b, T k) {
      return b + (Fip1 * (m_s * Fip1 + b1) * sinc<T>(2 * k * Fip1) -
                  Fi * (m_s * Fi + b1) * sinc<T>(2 * k * Fi)) *
                     wt2;
    });
  }

  if (Nodd) {
    b.insert(b.begin(), b0);
  }

  auto w0 = weight[0];
  vector<T> a(b.size());
  std::transform(b.begin(), b.end(), a.begin(), [w0](T b) { return std::pow(w0, 2) * 4 * b; });

  vector<T> result = {a.rbegin(), a.rend()};
  decltype(a.begin()) it;
  if (Nodd) {
    it = a.begin() + 1;
  } else {
    it = a.begin();
  }
  result.insert(result.end(), it, a.end());

  for (auto &it : result) {
    it *= FILTER_INC;
  }

  return result;
}

template <typename T> auto kaiser(const int order, const T bta) -> std::vector<T> {
  T Numerator;
  T Denominator;
  Denominator = custom_cyl_bessel_i0(bta);
  // Denominator = std::cyl_bessel_i(0, bta);
  auto od2 = (static_cast<T>(order) - 1) / 2;
  std::vector<T> window;
  window.reserve(order);
  for (int n = 0; n < order; n++) {
    auto x = bta * std::sqrt(1 - std::pow((n - od2) / od2, 2));
    Numerator = custom_cyl_bessel_i0(x);
    // Numerator = std::cyl_bessel_i(0, x);
    window.push_back(Numerator / Denominator);
  }
  return window;
}

template <typename T>
auto resample(int upFactor, int downFactor, vector<T> &inputSignal, vector<T> &outputSignal)
    -> void {
  const int n = 10;
  const T bta = 5.0;
  if (upFactor <= 0 || downFactor <= 0) {
    throw std::runtime_error("factors must be positive integer");
  }
  int gcd_o = std::gcd(upFactor, downFactor);
  upFactor /= gcd_o;
  downFactor /= gcd_o;

  if (upFactor == downFactor) {
    outputSignal = inputSignal;
    return;
  }

  int inputSize = inputSignal.size();
  outputSignal.clear();
  int outputSize = quotientCeil(inputSize * upFactor, downFactor);
  outputSignal.reserve(outputSize);

  int maxFactor = std::max(upFactor, downFactor);
  T firlsFreq = 1.0 / FILTER_DIV / static_cast<T>(maxFactor);
  int length = 2 * n * maxFactor + 1;
  vector<T> firlsFreqsV = {0.0, 2 * firlsFreq, 2 * firlsFreq, 1.0};
  vector<T> firlsAmplitudeV = {1.0, 1.0, 0.0, 0.0};
  vector<T> coefficients = firls<T>(length - 1, firlsFreqsV, firlsAmplitudeV);
  vector<T> window = kaiser<T>(length, bta);
  int coefficientsSize = coefficients.size();
  for (int i = 0; i < coefficientsSize; i++) {
    coefficients[i] *= upFactor * window[i];
  }

  int lengthHalf = (length - 1) / 2;
  int nz = downFactor - lengthHalf % downFactor;
  vector<T> h;
  h.reserve(coefficients.size() + nz);
  // Insert nz zeros
  h.insert(h.end(), nz, 0.0);
  // Insert coefficients
  h.insert(h.end(), coefficients.begin(), coefficients.end());

  int hSize = h.size();
  lengthHalf += nz;
  int delay = lengthHalf / downFactor;
  nz = 0;
  while (quotientCeil((inputSize - 1) * upFactor + hSize + nz, downFactor) - delay < outputSize) {
    nz++;
  }
  h.insert(h.end(), nz, 0.0);

  vector<T> y;
  upfirdn(upFactor, downFactor, inputSignal, h, y);
  for (int i = delay; i < outputSize + delay; i++) {
    outputSignal.push_back(y[i]);
  }
  return;
}
#include <cmath>
#include <vector>

template <typename T> T custom_cyl_bessel_i0(T x) {
  T sum = 1.0;
  T y = x / 2.0;
  T t = y * y;
  T term = t;
  int k = 1;
  while (term > 1e-10) {
    sum += term;
    k++;
    term *= t / (k * k);
  }
  return sum;
}
} // namespace haptics::tools