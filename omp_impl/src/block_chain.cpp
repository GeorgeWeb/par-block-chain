#include "block_chain.h"
#include "sha256.h"

#include "../util/timer.hpp"
#include "../util/file_io.hpp"

#include <omp.h>

#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <thread>

using namespace std;
using namespace std::chrono;

// Note that _time would normally be set to the time of the block's creation.
// This is part of the audit a block chain.  To enable consistent results
// from parallelisation we will just use the index value, so time increments
// by one each time: 1, 2, 3, etc.
block::block(uint32_t index, const string &data)
    : _index(index), _data(data), _nonce(0), _modified_nonce(false),
      _time(static_cast<long>(index)) {}

void block::mine_block(uint32_t difficulty) noexcept {
    string str(difficulty, '0');

    // lambda that *dynamically* returns optimal thread count depending on the difficulty level,
    // so it will performance fast without the need for allocating excessive threads in memory when we won't need as many
    const auto optimal_thread_count = [difficulty](uint32_t max_difficulty = 6u) -> unsigned int {
        constexpr auto potentially_used_thread_count = 1u; // assuming: main()
        constexpr auto minimum_thread_count = 2u;;
        // helper lambda that calculates the optimal choice for a number of threads based on the difficulty
        const auto calculate_optimal = [difficulty,max_difficulty] {
            const auto hardware_thread_count = thread::hardware_concurrency() - potentially_used_thread_count;
            const auto dynamic_thread_count = minimum_thread_count + max_difficulty - abs(static_cast<int>(max_difficulty / difficulty));
            // for low-level difficulty, this is a safer number of threads to use, otherwise
            // it is likely to (even if only rarely) to obtain wrong output values of hashes
            if (difficulty < 2) {
                return dynamic_thread_count;
            }
            return max(hardware_thread_count, dynamic_thread_count);
        };
        return calculate_optimal();
    };

    // lambda helper function that calculates the block's hash value in a concurrent/thread-safe manner
    auto concurrent_calculate_hash = [difficulty,str,this] {
        while (!_modified_nonce) {
            if (const auto local_hash = calculate_hash();
                local_hash.substr(0, difficulty) == str
                && !_modified_nonce) {
                #pragma omp critical
                {
                    _modified_nonce = true;
                }
                _hash = local_hash;
            }
        }
    };

    // define file name
    static string filename("results-omp-diff" + std::to_string(difficulty) + ".csv");

    // ...
    double time_sum = 0.0;
    #pragma omp parallel num_threads(optimal_thread_count()) default(none) shared(concurrent_calculate_hash,time_sum)
    {
        double local_time_sum = omp_get_wtime();
        
        std::invoke(concurrent_calculate_hash);

        // convert the omp returned thread execution time seconds into microseconds
        local_time_sum = omp_get_wtime() - local_time_sum;
        time_sum += local_time_sum;
    }
    
    // end timer and print results
    constexpr auto milli_ratio = 1000.0;
    const auto average = milli_ratio * (time_sum / static_cast<double>(optimal_thread_count()));
    cout << "Block mined: " << _hash << " in " << average << endl;
    // save to file
    util::file_io::get().save<double>(average, filename);
}

std::string block::calculate_hash() const noexcept {
    stringstream ss;
    #pragma omp critical
    {
        ss << _index << _time << _data << ++_nonce << prev_hash;
    }
    return sha256(ss.str());
}

block_chain::block_chain() : _difficulty(3) {
    _chain.emplace_back(block(0, "Genesis Block"));
}

void block_chain::add_block(block new_block) noexcept {
    new_block.prev_hash = get_last_block().get_hash();
    new_block.mine_block(_difficulty);
    _chain.push_back(std::move(new_block));
}
