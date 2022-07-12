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

[[nodiscard]] auto linearInterpolation2(std::vector<std::pair<int, double>> points)
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
      double amp = (f0 * (t1 - t) + f1 * (t - t0)) / (t1 - t0);
      Ylinear.push_back(amp);
      t += 1;
    }
    i++;
  }
return Ylinear;
}

[[nodiscard]] auto akimaInterpolation(std::vector<std::pair<int, double>> points)
    -> std::vector<double> {
  int n = static_cast<int>(points.size());
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

  for (int i = 0; i < dx.size(); i++) {
    dx[i] = points[i + 1].first - points[i].first;
    dy[i] = points[i + 1].second - points[i].second;
    m[i] = dy[i] / dx[i];
    m1[i + 2] = m[i];
  }

  m1[0] = 3 * m[0] - 2 * m[1];
  m1[1] = 2 * m[0] - m[1];
  m1[n + 1] = 2 * m[n - 2] - m[n - 3];
  m1[n + 2] = 3 * m[n - 2] - 2 * m[n - 3];

  for (int i = 0; i < m1.size() - 1; i++) {
    dm[i] = abs(m1[i + 1] - m1[i]);
    if (i < n) {
      b[i] = m1[i + 1];
    }
  }

  double max = std::pow(1, AKIMA_CST_2);

  for (int i = 0; i < f.size(); i++) {
    f1[i] = dm[i + 2];
    f2[i] = dm[i];
    f[i] = f1[i] + f2[i];
    if (f[i] > max) {
      max = f[i];
    }
  }

  for (int i = 0; i < f.size(); i++) {
    if (f[i] > std::pow(AKIMA_CST_1, AKIMA_CST_3) * max) {
      b[i] = (f1[i] * m1[i + 1] + f2[i] * m1[i + 2]) / f[i];
    }
  }

  for (int i = 0; i < c.size(); i++) {
    c[i] = (3 * m[i] - 2 * b[i] - b[i + 1]) / dx[i];
    d[i] = (b[i] + b[i + 1] - 2 * m[i]) / std::pow(dx[i], 2);
  }

  int count = 0;
  std::vector<double> Yakima(points.back().first + 1);

  for (int i = 0; i < points.size() - 1; i++) {
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

} // namespace haptics::tools