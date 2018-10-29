#ifndef UTIL_TIMER_HPP_
#define UTIL_TIMER_HPP_

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <type_traits>

namespace util {

// prototype of the timer class
template <typename T, typename Ratio> class timer;

// prototype of the operator to be overloaded for class timer
template <typename T, typename Ratio>
std::ostream &operator<<(std::ostream &os, const timer<T, Ratio> &obj);

// implementing the timer class
template <typename T = double,
          typename Ratio = std::ratio<1, 1>> // default Ratio is seconds
class timer final {
  // convenience type aliases
  using clock = std::chrono::high_resolution_clock;
  using duration = std::chrono::duration<T, Ratio>;

public:
  timer() : _start(clock::now()) {}

  inline void reset() { _start = clock::now(); }

  inline T get_elapsed() const {
    duration elapsed = clock::now() - _start;
    return elapsed.count();
  }

  inline std::string RatioToString() const {
    return std::string(std::is_same<Ratio, std::ratio<1, 1>>::value
                           ? "second(s)"
                           : std::is_same<Ratio, std::milli>::value
                                 ? "millisecond(s)"
                                 : std::is_same<Ratio, std::micro>::value
                                       ? "microsecond(s)"
                                       : std::is_same<Ratio, std::nano>::value
                                             ? "nanosecond(s)"
                                             : "");
  }

  // declaring operator<< overload for class timer
  friend std::ostream &operator<<<>(std::ostream &os, const timer &obj);

private:
  std::chrono::time_point<clock> _start;
};

// implementing the operator<< overload for class timer
template <typename T, typename Ratio>
std::ostream &operator<<(std::ostream &os, const timer<T, Ratio> &obj) {
  return os << obj.get_elapsed() << ' ' << obj.RatioToString();
}

} // namespace util

#endif // UTIL_TIMER_HPP_
