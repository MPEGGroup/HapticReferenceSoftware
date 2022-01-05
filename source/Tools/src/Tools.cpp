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
                                      double end_frequency, int position) -> double {

  int start_t = start_time;
  int end_t = end_time;
  double start_f = start_frequency;
  double end_f = end_frequency;

  if (end_t > start_t) {
    std::swap(start_t, end_t);
    std::swap(start_f, end_f);
  }

  if (start_t == end_t) {
    return end_f;
  }

  return position * (end_f - start_f) / (end_t - start_t) + start_f;
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
} // namespace haptics::tools