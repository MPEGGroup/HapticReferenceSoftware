#include <Tools/include/Tools.h>

namespace haptics::tools {

  extern auto haptics::tools::linearInterpolation(std::pair<int, double> a, std::pair<int, double> b,
                                                int x) -> double {

    int start = a.first;
    int end = b.first;
    double start_a = a.second;
    double end_a = b.second;

    if (x < start || x > end) {
      return EXIT_FAILURE;
    }

    if (start >= end) {
      std::swap(start, end);
      std::swap(start_a, end_a);
    }

    return (start_a * (end - x) + end_a * (x - start)) / (end - start);
  }

  [[nodiscard]] extern auto chirpInterpolation(int start_time, int end_time, double start_frequency,
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

    if (position < start_t && position > end_t) {
      return EXIT_FAILURE;
    }

    return position * (end_f - start_f) / (end_t - start_t) + start_f;
  }

  [[nodiscard]] extern auto genericNormalization(double start_in, double end_in, double start_out,
                                                 double end_out, double x_in) -> double {

    double x_out = -1;

    if (end_in - start_in != 0) {
      x_out = ((end_out - start_out) / (end_in - start_in)) * (x_in - end_in) + end_out;
    }

    return x_out;
  }

  [[nodiscard]] extern auto is_eq(double a, double b) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return std::fabs(a - b) <= std::numeric_limits<double>::epsilon() * 100;
  }
} // namespace haptics::tools