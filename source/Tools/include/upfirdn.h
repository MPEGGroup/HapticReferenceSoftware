#pragma once

#include <complex>
#include <stdexcept>
#include <vector>

namespace haptics::tools {
class Resampler {
public:
  Resampler(int upRate, int downRate, const std::vector<double> &coefs);
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
      _transposedCoefs.clear();
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
      , _paddedCoefCount(other._paddedCoefCount)
      , _coefsPerPhase(other._coefsPerPhase)
      , _t(other._t)
      , _xOffset(other._xOffset) {}
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
    }
    return *this;
  }

  virtual ~Resampler() = default;

  auto apply(const std::vector<double> &in, std::vector<double> &out) -> int;
  [[nodiscard]] auto neededOutCount(int inCount) const -> int;
  [[nodiscard]] auto coefsPerPhase() const -> int { return _coefsPerPhase; }

private:
  int _upRate;
  int _downRate;

  std::vector<double> _transposedCoefs;
  std::vector<double> _state;

  int _paddedCoefCount; // ceil(len(coefs)/upRate)*upRate
  int _coefsPerPhase;   // _paddedCoefCount / upRate

  int _t; // "time" (modulo upRate)
  int _xOffset;
};

Resampler::Resampler(int upRate, int downRate, const std::vector<double> &coefs)
    : _upRate(upRate), _downRate(downRate), _t(0), _xOffset(0) {
  _paddedCoefCount = static_cast<int>(coefs.size());
  while ((_paddedCoefCount % _upRate) != 0) {
    _paddedCoefCount++;
  }
  _coefsPerPhase = _paddedCoefCount / _upRate;

  _transposedCoefs.resize(_paddedCoefCount, 0.0);
  _state.resize(_coefsPerPhase - 1, 0.0);

  for (int i = 0; i < _upRate; ++i) {
    for (int j = 0; j < _coefsPerPhase; ++j) {
      if (j * _upRate + i < static_cast<int>(coefs.size())) {
        _transposedCoefs[(_coefsPerPhase - 1 - j) + i * _coefsPerPhase] = coefs[j * _upRate + i];
      }
    }
  }
}

auto Resampler::neededOutCount(int inCount) const -> int {
  int np = inCount * _upRate;
  int need = np / _downRate;
  if ((_t + _upRate * _xOffset) < (np % _downRate)) {
    need++;
  }
  return need;
}

auto Resampler::apply(const std::vector<double> &in, std::vector<double> &out) -> int {
  int inCount = static_cast<int>(in.size());
  int outCount = neededOutCount(inCount);
  if (static_cast<int>(out.size()) < outCount) {
    throw std::invalid_argument("Not enough output samples");
  }

  auto idxIn = _xOffset; // in
  auto idxOut = 0;       // out
  // auto end = in.end();

  while (idxIn < static_cast<int>(in.size())) {
    double acc = 0.0;
    int idxCoef = _t * _coefsPerPhase; // _transposedCoefs
    int xPtr = idxIn - _coefsPerPhase + 1;
    int offset = -xPtr;

    if (offset > 0) {
      auto idxState = static_cast<int>(_state.size()) - offset;
      while (idxState < static_cast<int>(_state.size())) {
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

  _xOffset = idxIn - static_cast<int>(in.size());

  int retain = (_coefsPerPhase - 1) - inCount;
  if (retain > 0) {
    std::copy(_state.end() - retain, _state.end(), _state.begin());
    std::copy(in.begin(), in.end(), _state.end() - inCount);
  } else {
    std::copy(in.end() - (_coefsPerPhase - 1), in.end(), _state.begin());
  }

  return idxOut;
}

auto upfirdn(int upRate, int downRate, const std::vector<double> &input,
             const std::vector<double> &filter, std::vector<double> &results) -> void {
  Resampler theResampler(upRate, downRate, filter);

  int padding = theResampler.coefsPerPhase() - 1;
  std::vector<double> inputPadded(static_cast<int>(input.size()) + padding, 0);
  std::copy(input.begin(), input.end(), inputPadded.begin());

  int resultsCount = theResampler.neededOutCount(static_cast<int>(input.size()) + padding);
  results.resize(resultsCount);

  theResampler.apply(inputPadded, results);
}
} // namespace haptics::tools