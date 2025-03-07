/*
Copyright (c) 2009, Motorola, Inc

All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Motorola nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#include <complex>
#include <stdexcept>
#include <vector>
namespace haptics::tools {
template <class S1, class S2, class C> class Resampler {
public:
  using inputType = S1;
  using outputType = S2;
  using coefType = C;

  Resampler(int upRate, int downRate, std::vector<C> coefs, int coefCount);
  Resampler(const Resampler &src) {
    _upRate = src._upRate;
    _downRate = src._downRate;
    _paddedCoefCount = src._paddedCoefCount;
    _coefsPerPhase = src._coefsPerPhase;
    _t = src._t;
    _xOffset = src._xOffset;
  }
  auto operator=(const Resampler &other) -> Resampler & {
    if (this != &other) {
      // Perform deep copy of other members
      _upRate = other._upRate;
      _downRate = other._downRate;
      _paddedCoefCount = other._paddedCoefCount;
      _coefsPerPhase = other._coefsPerPhase;
      _t = other._t;
      _xOffset = other._xOffset;

      // Allocate new memory for _transposedCoefs and copy the data
      delete[] _transposedCoefs;
      _transposedCoefs = std::move(other._transposedCoefs);
      std::copy(other._transposedCoefs.data(), other._transposedCoefs.data() + _paddedCoefCount,
                _transposedCoefs.data());
    }
    return *this;
  }

  // Move constructor
  Resampler(Resampler &&other) noexcept
      : _upRate(other._upRate)
      , _downRate(other._downRate)
      , _transposedCoefs(std::move(other._transposedCoefs))
      , _state(std::move(other._state))
      , _stateEnd(other._stateEnd)
      , _paddedCoefCount(other._paddedCoefCount)
      , _coefsPerPhase(other._coefsPerPhase)
      , _t(other._t)
      , _xOffset(other._xOffset) {
    other._stateEnd = nullptr;
  }
  // Move assignment operator
  auto operator=(Resampler &&other) noexcept -> Resampler & {
    if (this != &other) {
      _upRate = other._upRate;
      _downRate = other._downRate;
      _paddedCoefCount = other._paddedCoefCount;
      _coefsPerPhase = other._coefsPerPhase;
      _t = other._t;
      _xOffset = other._xOffset;

      _transposedCoefs = std::move(other._transposedCoefs);
      _state = std::move(other._state);
      _stateEnd = other._stateEnd;
      other._stateEnd = nullptr;
    }
    return *this;
  }
  virtual ~Resampler() = default;

  auto apply(std::vector<S1> &in, int inCount, std::vector<S2> &out, int outCount) -> int;
  auto neededOutCount(int inCount) -> int;
  auto coefsPerPhase() -> int { return _coefsPerPhase; }

private:
  int _upRate;
  int _downRate;

  std::vector<coefType> _transposedCoefs;
  std::vector<coefType> _state;
  inputType *_stateEnd;

  int _paddedCoefCount; // ceil(len(coefs)/upRate)*upRate
  int _coefsPerPhase;   // _paddedCoefCount / upRate

  int _t; // "time" (modulo upRate)
  int _xOffset;
};

#include <cmath>
#include <iostream>

using std::invalid_argument;

template <class S1, class S2, class C>
Resampler<S1, S2, C>::Resampler(int upRate, int downRate, const std::vector<C> coefs, int coefCount)
    : _upRate(upRate)
    , _downRate(downRate)
    , _t(0)
    , _xOffset(0)
/*
  The coefficients are copied into local storage in a transposed, flipped
  arrangement.  For example, suppose upRate is 3, and the input number
  of coefficients coefCount = 10, represented as h[0], ..., h[9].
  Then the internal buffer will look like this:
                h[9], h[6], h[3], h[0],   // flipped phase 0 coefs
                   0, h[7], h[4], h[1],   // flipped phase 1 coefs (zero-padded)
                   0, h[8], h[5], h[2],   // flipped phase 2 coefs (zero-padded)
*/
{
  _paddedCoefCount = coefCount;
  while ((_paddedCoefCount % _upRate) != 0) {
    _paddedCoefCount++;
  }
  _coefsPerPhase = _paddedCoefCount / _upRate;

  _transposedCoefs.resize(_paddedCoefCount);
  std::fill(_transposedCoefs.begin(), _transposedCoefs.begin() + _paddedCoefCount, 0.);

  _state.resize(_coefsPerPhase - 1);
  _stateEnd = &(*(_state.begin() + (_coefsPerPhase - 1)));
  std::fill(_state.begin(), _state.begin() + (_coefsPerPhase - 1), 0.);

  /* This both transposes, and "flips" each phase, while
   * copying the defined coefficients into local storage.
   * There is probably a faster way to do this
   */
  for (int i = 0; i < _upRate; ++i) {
    for (int j = 0; j < _coefsPerPhase; ++j) {
      if (j * _upRate + i < coefCount) {
        _transposedCoefs.at((_coefsPerPhase - 1 - j) + i * _coefsPerPhase) = coefs[j * _upRate + i];
      }
    }
  }
}

template <class S1, class S2, class C>
auto Resampler<S1, S2, C>::neededOutCount(int inCount) -> int
/* compute how many outputs will be generated for inCount inputs  */
{
  int np = inCount * _upRate;
  int need = np / _downRate;
  if ((_t + _upRate * _xOffset) < (np % _downRate)) {
    need++;
  }
  return need;
}

template <class S1, class S2, class C>
auto Resampler<S1, S2, C>::apply(std::vector<S1> &in, int inCount, std::vector<S2> &out,
                                 int outCount) -> int {
  if (outCount < neededOutCount(inCount)) {
    throw std::invalid_argument("Not enough output samples");
  }

  // x points to the latest processed input sample
  auto x = in.begin() + _xOffset;
  auto y = out.begin();
  auto end = in.begin() + inCount;

  while (x < end) {
    outputType acc = 0.;
    auto h = _transposedCoefs.begin() + _t * _coefsPerPhase;
    auto xPtr = x - _coefsPerPhase + 1;
    int offset = std::distance(in.begin(), xPtr);

    if (offset > 0) {
      // need to draw from the _state buffer
      auto statePtr = _state.begin() + (_coefsPerPhase - 1) - offset;
      while (statePtr < _state.begin() + (_coefsPerPhase - 1)) {
        acc += *statePtr++ * *h++;
      }
      xPtr += offset;
    }

    while (xPtr <= x) {
      acc += *xPtr++ * *h++;
    }

    *y++ = acc;
    _t += _downRate;

    int advanceAmount = _t / _upRate;
    std::advance(x, advanceAmount);

    // which phase of the filter to use
    _t %= _upRate;
  }

  _xOffset = std::distance(end, x);

  // manage _state buffer
  // find number of samples retained in buffer:
  int retain = (_coefsPerPhase - 1) - inCount;
  if (retain > 0) {
    // for inCount smaller than state buffer, copy end of buffer to beginning:
    std::copy(_state.begin() + (_coefsPerPhase - 1) - retain, _state.begin() + (_coefsPerPhase - 1),
              _state.data());
    // Then, copy the entire (short) input to end of buffer
    std::copy(in.begin(), end, _state.begin() + (_coefsPerPhase - 1) - inCount);
  } else {
    // just copy last input samples into state buffer
    std::copy(end - (_coefsPerPhase - 1), end, _state.data());
  }

  // number of samples computed
  return std::distance(out.begin(), y);
}

/*
This template function provides a one-shot resampling.  Extra samples
are padded to the end of the input in order to capture all of the non-zero
output samples.
The output is in the "results" vector which is modified by the function.

Note, I considered returning a vector instead of taking one on input, but
then the C++ compiler has trouble with implicit template instantiation
(e.g. have to say upfirdn<float, float, float> every time - this
way we can let the compiler infer the template types).

Thanks to Lewis Anderson (lkanders@ucsd.edu) at UCSD for
the original version of this function.
*/
template <class S1, class S2, class C>
auto upfirdn(std::tuple<int, int> upDownRate, std::vector<S1> input, int inLength, std::vector<C> filter,
             int filterLength, std::vector<S2> &results) -> void {
  // Create the Resampler
  Resampler<S1, S2, C> theResampler(std::get<0>(upDownRate), std::get<1>(upDownRate), filter,
                                    filterLength);

  // pad input by length of one polyphase of filter to flush all values out
  int padding = theResampler.coefsPerPhase() - 1;
  //std::vector<S1> inputPadded;
  //for (int i = 0; i < inLength + padding; i++) {
  //  if (i < inLength) {
  //    inputPadded.push_back(input[i]);
  //  } else {
  //    inputPadded.push_back(0);
  //  }
  //}
  std::vector<S1> inputPadded(input.size() + padding, 0);
  std::copy(input.begin(), input.end(), inputPadded.begin());

  // calc size of output
  int resultsCount = theResampler.neededOutCount(inLength + padding);

  results.resize(resultsCount);

  // run filtering
  std::vector<S2> resultsVector(resultsCount);
  theResampler.apply(inputPadded, inLength + padding, resultsVector, resultsCount);

  // Copy results back to the original results vector
  std::copy(resultsVector.begin(), resultsVector.end(), results.begin());
}

template <class S1, class S2, class C>
auto upfirdn(int upRate, int downRate, std::vector<S1> &input, std::vector<C> &filter,
             std::vector<S2> &results) -> void
/*
This template function provides a one-shot resampling.
The output is in the "results" vector which is modified by the function.
In this version, the input and filter are vectors as opposed to
pointer/count pairs.
*/
{
  upfirdn<S1, S2, C>(std::tuple<int, int>(upRate, downRate), input, input.size(), filter,
                     filter.size(), results);
}
} // namespace haptics::tools