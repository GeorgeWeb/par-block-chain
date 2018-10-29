#include "block_chain.h"

#include "../util/file_io.hpp"
#include "../util/timer.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

auto main(int argc, char **argv) -> int {
  // Benchmark configuration variables.
  constexpr auto chain_size = 1u;  // tested with 50u
  constexpr auto iterations = 1u;  // tested with 50u
  constexpr auto max_difficulty = 5u;

  // Choose an option for calculating the nubmer of threads to be used while
  // running the parallel bit of the program.
  static thread_alloc option;
  if (argc > 1 && std::string(argv[1]).find("dynamic") != string::npos) {
    option = thread_alloc::dynamic;
  } else if (argc > 1 &&
             std::string(argv[1]).find("hardware") != string::npos) {
    option = thread_alloc::hardware;
  } else {
    option = thread_alloc::dynamic;
  }

  for (auto diff_i = 1u; diff_i < max_difficulty + 1u; ++diff_i) {
    // Define file name to save benchmark data to.
    const auto type = (option == thread_alloc::hardware)
                          ? std::string("hardware")
                          : std::string("dynamic");
    const string filename("results-pure-threads-" + type + ".csv");

    // Save header to file.
    const auto header = string("Average Time Per Block, Difficulty");
    util::file_io::get().save(header, filename);

    // Define program timer.
    util::timer<double, milli> timer{};

    // Declaring a vector that will store average times to mine a block for each
    // iteration.
    vector<double> block_times;

    block_chain bchain(diff_i, max_difficulty);
    for (auto iter_i = 1u; iter_i < iterations + 1u; iter_i++) {
      // Begin new timer every iteration.
      timer.reset();
      for (auto block_i = 1u; block_i < chain_size + 1u; ++block_i) {
        cout << "Mining block " << block_i << "... Difficulty = " << diff_i
             << ", Iteration = " << iter_i << endl;
        bchain.add_block(block(
            block_i, string("Block ") + to_string(block_i) + string(" Data"),
            option));
      }
      // End timer and add an averaged value the vector of iterations.
      block_times.push_back(timer.get_elapsed() /
                            static_cast<double>(chain_size));
    }

    // Calculate and save the average time taken to mine 1 block based on all of
    // the iterations.
    const auto block_avg_time =
        accumulate(begin(block_times), end(block_times), 0.0) / iterations;
    cout.precision(numeric_limits<double>::max_digits10);
    cout << "[AVERAGE] Time Per Block: " << block_avg_time << endl;

    // Save (append) measured data to file after the header.
    const auto body =
        string(to_string(block_avg_time) + ", " + to_string(diff_i));
    util::file_io::get().save(body, filename);
  }

  return EXIT_SUCCESS;
}