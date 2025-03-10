#pragma once

#include <complex>
#include <stdexcept>
#include <vector>

namespace haptics::tools {
template <class S1, class S2, class C> class Resampler {
public:
  typedef S1 inputType;
  typedef S2 outputType;
  typedef C coefType;

  Resampler(int upRate, int downRate, const std::vector<C> &coefs);
  virtual ~Resampler() = default;

  int apply(const std::vector<S1> &in, std::vector<S2> &out);
  int neededOutCount(int inCount) const;
  int coefsPerPhase() const { return _coefsPerPhase; }

private:
  int _upRate;
  int _downRate;

  std::vector<coefType> _transposedCoefs;
  std::vector<inputType> _state;

  int _paddedCoefCount; // ceil(len(coefs)/upRate)*upRate
  int _coefsPerPhase;   // _paddedCoefCount / upRate

  int _t; // "time" (modulo upRate)
  int _xOffset;
};

template <class S1, class S2, class C>
Resampler<S1, S2, C>::Resampler(int upRate, int downRate, const std::vector<C> &coefs)
    : _upRate(upRate), _downRate(downRate), _t(0), _xOffset(0) {
  _paddedCoefCount = coefs.size();
  while (_paddedCoefCount % _upRate) {
    _paddedCoefCount++;
  }
  _coefsPerPhase = _paddedCoefCount / _upRate;

  _transposedCoefs.resize(_paddedCoefCount, 0.0);
  _state.resize(_coefsPerPhase - 1, 0.0);

  for (int i = 0; i < _upRate; ++i) {
    for (int j = 0; j < _coefsPerPhase; ++j) {
      if (j * _upRate + i < coefs.size()) {
        _transposedCoefs[(_coefsPerPhase - 1 - j) + i * _coefsPerPhase] = coefs[j * _upRate + i];
      }
    }
  }
}

template <class S1, class S2, class C> int Resampler<S1, S2, C>::neededOutCount(int inCount) const {
  int np = inCount * _upRate;
  int need = np / _downRate;
  if ((_t + _upRate * _xOffset) < (np % _downRate)) {
    need++;
  }
  return need;
}

template <class S1, class S2, class C>
int Resampler<S1, S2, C>::apply(const std::vector<S1> &in, std::vector<S2> &out) {
  int inCount = in.size();
  int outCount = neededOutCount(inCount);
  if (out.size() < outCount) {
    throw std::invalid_argument("Not enough output samples");
  }

  auto idxIn = _xOffset; // in
  auto idxOut = 0;       // out
  // auto end = in.end();

  while (idxIn < in.size()) {
    outputType acc = 0.0;
    int idxCoef = _t * _coefsPerPhase; // _transposedCoefs
    int xPtr = idxIn - _coefsPerPhase + 1;
    int offset = -xPtr;

    if (offset > 0) {
      auto idxState = _state.size() - offset;
      while (idxState < _state.size()) {
        acc += _state[idxState++] * _transposedCoefs[idxCoef++];
      }
      xPtr += offset;
    }

    while (xPtr <= idxIn) {
      acc += in[xPtr++] * _transposedCoefs[idxCoef++];
    }
    out[idxOut++] = acc;
    _t += _downRate;

    int advanceAmount = _t / _upRate;
    idxIn += advanceAmount;
    _t %= _upRate;
  }

  _xOffset = idxIn - in.size();

  int retain = (_coefsPerPhase - 1) - inCount;
  if (retain > 0) {
    std::copy(_state.end() - retain, _state.end(), _state.begin());
    std::copy(in.begin(), in.end(), _state.end() - inCount);
  } else {
    std::copy(in.end() - (_coefsPerPhase - 1), in.end(), _state.begin());
  }

  return idxOut;
}

template <class S1, class S2, class C>
void upfirdn(int upRate, int downRate, const std::vector<S1> &input, const std::vector<C> &filter,
             std::vector<S2> &results) {
  Resampler<S1, S2, C> theResampler(upRate, downRate, filter);

  int padding = theResampler.coefsPerPhase() - 1;
  std::vector<S1> inputPadded(input.size() + padding, 0);
  std::copy(input.begin(), input.end(), inputPadded.begin());

  int resultsCount = theResampler.neededOutCount(input.size() + padding);
  results.resize(resultsCount);

  theResampler.apply(inputPadded, results);
}
} // namespace haptics::tools