#include <Tools/include/Tools.h>
#include <cmath>

namespace haptics::tools {

auto linearInterpolation(std::pair<int, double> a, std::pair<int, double> b, double x) -> double {

  double start = a.first;
  double end = b.first;
  double start_a = a.second;
  double end_a = b.second;

  if (x < start) {
    return start_a;
  }

  if (x > end) {
    return end_a;
  }

  if (start >= end) {
    std::swap(start, end);
    std::swap(start_a, end_a);
  }

  return (start_a * (end - x) + end_a * (x - start)) / (end - start);
}

[[nodiscard]] auto chirpInterpolation(int start_time, int end_time, double start_frequency,
                                      double end_frequency, double position) -> double {
  if (end_time < start_time) {
    std::swap(start_time, end_time);
    std::swap(start_frequency, end_frequency);
  }

  if (start_time == end_time) {
    return end_frequency;
  }

  return (((end_frequency - start_frequency) / (end_time - start_time)) / 2) *
             (position - start_time) +
         start_frequency;
}

[[nodiscard]] auto genericNormalization(double start_in, double end_in, double start_out,
                                        double end_out, double x_in) -> double {

  double x_out = -1;

  if (end_in - start_in != 0) {
    x_out = ((end_out - start_out) / (end_in - start_in)) * (x_in - end_in) + end_out;
  }

  return x_out;
}

[[nodiscard]] auto is_eq(double a, double b) -> bool {
  const int threshold = 100;
  return std::fabs(a - b) <= std::numeric_limits<double>::epsilon() * threshold;
}

[[nodiscard]] auto linearInterpolation2(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {
  std::vector<double> Ylinear;
  double t = 0;
  int i = 0;
  while (t < points.back().first) {
    double t0 = points[i].first;
    double f0 = points[i].second;
    double t1 = points[i + 1].first;
    double f1 = points[i + 1].second;
    while (t <= t1) {
      double amp = (f0 * (t1 - t) + f1 * (t - t0)) / (t1 - t0);
      Ylinear.push_back(amp);
      t += 1;
    }
    i++;
  }
  return Ylinear;
}

[[nodiscard]] auto cubicInterpolation2(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {
  std::vector<double> Ylinear;
  double t = 0;
  int i = 0;
  while (t < points.back().first) {
    double t0 = points[i].first;
    double f0 = points[i].second;
    double t1 = points[i + 1].first;
    double f1 = points[i + 1].second;
    double h = t1 - t0;
    while (t <= t1) {
      double amp = f0 + (f1 - f0) * (3 * h + 2 * (t0 - t)) * std::pow(t - t0, 2) / std::pow(h, 3);
      Ylinear.push_back(amp);
      t += 1;
    }
    i++;
  }
  return Ylinear;
}
[[nodiscard]] auto cubicInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {
  std::vector<double> x;
  std::vector<double> y;
  for (auto point : points) {
    x.push_back(point.first);
    y.push_back(point.second);
  }
  auto n = points.size() - 1;
  std::vector<double> a;
  a.insert(a.begin(), y.begin(), y.end());
  std::vector<double> b(n);
  std::vector<double> d(n);
  std::vector<double> h(n);

  for (size_t i = 0; i < n; ++i) {
    h[i] = x[i + 1] - x[i];
  }

  std::vector<double> alpha;
  alpha.push_back(0);
  for (size_t i = 1; i < n; ++i) {
    alpha.push_back(3 * (a[i + 1] - a[i]) / h[i] - 3 * (a[i] - a[i - 1]) / h[i - 1]);
  }
  std::vector<double> c(n + 1);
  std::vector<double> l(n + 1);
  std::vector<double> mu(n + 1);
  std::vector<double> z(n + 1);
  l[0] = 1;
  mu[0] = 0;
  z[0] = 0;

  for (size_t i = 1; i < n; ++i) {
    l[i] = 2 * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
    mu[i] = h[i] / l[i];
    z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
  }

  l[n] = 1;
  z[n] = 0;
  c[n] = 0;

  for (int j = static_cast<int>(n - 1); j >= 0; --j) {
    c[j] = z[j] - mu[j] * c[j + 1];
    b[j] = (a[j + 1] - a[j]) / h[j] - h[j] * (c[j + 1] + 2 * c[j]) / 3;
    d[j] = (c[j + 1] - c[j]) / 3 / h[j];
  }

  std::vector<double> output;
  double t = 0;
  int i = 0;
  while (t < x.back()) {
    double delta = t - x[i];
    while (t <= x[i + 1]) {
      double amp = a[i] + b[i] * delta + c[i] * std::pow(delta, 2) + d[i] * std::pow(delta, 3);
      output.push_back(amp);
      t += 1;
    }
    i++;
  }
  return output;
}

[[nodiscard]] auto akimaInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {
  auto n = points.size();
  std::vector<double> dx(n - 1);
  std::vector<double> dy(dx.size());
  std::vector<double> m(dx.size());
  std::vector<double> m1(m.size() + 4);
  std::vector<double> dm(m1.size() - 1);
  std::vector<double> f1(n);
  std::vector<double> f2(n);
  std::vector<double> f(n);
  std::vector<double> b(n);
  std::vector<double> c(n - 1);
  std::vector<double> d(n - 1);

  for (size_t i = 0; i < dx.size(); i++) {
    dx[i] = points[i + 1].first - points[i].first;
    dy[i] = points[i + 1].second - points[i].second;
    m[i] = dy[i] / dx[i];
    m1[i + 2] = m[i];
  }

  m1[0] = 3 * m[0] - 2 * m[1];
  m1[1] = 2 * m[0] - m[1];
  m1[n + 1] = 2 * m[n - 2] - m[n - 3];
  m1[n + 2] = 3 * m[n - 2] - 2 * m[n - 3];

  for (size_t i = 0; i < m1.size() - 1; i++) {
    dm[i] = fabs(m1[i + 1] - m1[i]);
    if (i < n) {
      b[i] = m1[i + 1];
    }
  }

  double max = AKIMA_EPSILON;

  for (size_t i = 0; i < f.size(); i++) {
    f1[i] = dm[i + 2];
    f2[i] = dm[i];
    f[i] = f1[i] + f2[i];
    if (f[i] > max) {
      max = f[i];
    }
  }

  for (size_t i = 0; i < f.size(); i++) {
    if (f[i] > std::pow(AKIMA_THRESHOLD, AKIMA_THRESHOLD2) * max) {
      b[i] = (f1[i] * m1[i + 1] + f2[i] * m1[i + 2]) / f[i];
    }
  }

  for (size_t i = 0; i < c.size(); i++) {
    c[i] = (3 * m[i] - 2 * b[i] - b[i + 1]) / dx[i];
    d[i] = (b[i] + b[i + 1] - 2 * m[i]) / std::pow(dx[i], 2);
  }

  int count = 0;
  std::vector<double> Yakima(points.back().first + 1);

  for (size_t i = 0; i < points.size() - 1; i++) {
    for (int j = points[i].first; j < points[i + 1].first; j++) {
      Yakima[count] =
          (((count - points[i].first) * d[i] + c[i]) * (count - points[i].first) + b[i]) *
              (count - points[i].first) +
          points[i].second;
      count++;
    }
  }

  Yakima[count] =
      (((count - points.back().first) * d.back() + c.back()) * (count - points.back().first) +
       b.back()) *
          (count - points.back().first) +
      points.back().second;

  return Yakima;
}

[[nodiscard]] auto bezierInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {

  std::vector<double> Ybezier;
  int dx1 = 0;
  int dx2 = 0;
  double f0 = 0;
  double f1 = 0;
  double f2 = 0;
  double t = 0;
  int i = 0;

  auto size = points.size();
  if (size % 2 == 0) {
    size--;
  }

  while (i < static_cast<int>(size) - 1) {
    dx1 = points[i + 1].first - points[i].first + 1;
    dx2 = points[i + 2].first - points[i].first + 1;
    f0 = points[i].second;
    f1 = points[i + 1].second;
    f2 = points[i + 2].second;

    for (int j = 1; j < dx2; j++) {
      if (1 - 2 * dx1 + dx2 != 0) {
        t = (1 - dx1 + std::sqrt(std::pow(dx1, 2) - 2 * j * dx1 + j * 1 + j * dx2 - 1 * dx2)) /
            (1 - 2 * dx1 + dx2);
      } else {
        t = (1 - j) / static_cast<double>(2 * (1 - dx1));
      }
      Ybezier.push_back(std::pow(1 - t, 2) * f0 + (1 - t) * 2 * t * f1 + std::pow(t, 2) * f2);
    }

    i += 2;
  }

  Ybezier.push_back(points[size - 1].second);

  return Ybezier;
}

[[nodiscard]] auto bsplineInterpolation(const std::vector<std::pair<int, double>> &points)
    -> std::vector<double> {
  auto p = static_cast<int>(points.size());
  auto n = points.back().first;
  std::vector<std::vector<double>> X1(3, std::vector<double>(3));
  std::vector<std::vector<double>> X2(3, std::vector<double>(3));
  std::vector<double> t(p + 3);
  std::vector<double> Ybspline(n + 1);

  t[0] = 0;
  t[1] = 0;
  for (int i = 0; i < p - 1; i++) {
    t[i + 2] = i / static_cast<double>(p - 2);
  }
  t[p + 1] = 1;
  t[p + 2] = 1;

  Ybspline[0] = points[0].second;

  double t0 = 0;
  double num = 0;
  double s = 0;
  double wt = 0;
  int k = 0;

  for (int m = 1; m < n; m++) {
    while (X1[2][2] < m) {
      t0 = t0 + (1 / static_cast<double>(n)) / BSPLINE_STEP;
      k = 0;
      while (t0 >= t[k]) {
        k = k + 1;
      }
      X1[0][0] = points[k - 3].first;
      X1[0][1] = points[k - 2].first;
      X1[0][2] = points[k - 1].first;

      for (int i = 1; i < 3; i++) {
        for (int j = i; j < 3; j++) {
          num = t0 - t[k - 3 + j];
          if (num == 0) {
            wt = 0;
          } else {
            s = t[k + j - i] - t[k - 3 + j];
            wt = num / s;
          }
          X1[i][j] = (1 - wt) * X1[i - 1][j - 1] + wt * X1[i - 1][j];
        }
      }
    }

    X2[0][0] = points[k - 3].second;
    X2[0][1] = points[k - 2].second;
    X2[0][2] = points[k - 1].second;

    for (int i = 1; i < 3; i++) {
      for (int j = i; j < 3; j++) {
        num = t0 - t[k - 3 + j];
        if (num == 0) {
          wt = 0;
        } else {
          s = t[k + j - i] - t[k - 3 + j];
          wt = num / s;
        }
        X2[i][j] = (1 - wt) * X2[i - 1][j - 1] + wt * X2[i - 1][j];
      }
    }
    Ybspline[m] = X2[2][2];
  }

  Ybspline[n] = points.back().second;

  return Ybspline;
}

[[nodiscard]] auto interpolationCodec(const std::vector<std::pair<int, double>> &points,
                                      types::CurveType curveType) -> std::vector<double> {
  std::vector<double> interpolation;
  double t = 0;
  int i = 0;
  double amp = 0;

  while (t < points.back().first) {
    double t0 = points[i].first;
    double f0 = points[i].second;
    double t1 = points[i + 1].first;
    double f1 = points[i + 1].second;
    double h = t1 - t0;

    while (t <= t1) {
      switch (curveType) {
      case types::CurveType::Cubic: {
        amp = f0 + (f1 - f0) * (3 * h + 2 * (t0 - t)) * std::pow(t - t0, 2) / std::pow(h, 3);
        break;
      }
      case types::CurveType::Linear: {
        amp = (f0 * (t1 - t) + f1 * (t - t0)) / (t1 - t0);
        break;
      }
      default:
        amp = 0;
        break;
      }
      interpolation.push_back(amp);
      t += 1;
    }
    i++;
  }
  return interpolation;
}


[[nodiscard]] auto is_number(const std::string &s) -> bool {
  return !s.empty() && std::find_if(s.begin(), s.end(),
                                    [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

} // namespace haptics::tools
