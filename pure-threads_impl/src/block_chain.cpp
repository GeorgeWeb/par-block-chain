#include "block_chain.h"
#include "sha256.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <thread>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// Note that _time would normally be set to the time of the block's creation.
// This is part of the audit a block chain.  To enable consistent results
// from parallelisation we will just use the index value, so time increments
// by one each time: 1, 2, 3, etc.
block::block(uint32_t index, const string &data) 
    : _index(index), _data(data), _nonce(0), 
      _time(static_cast<long>(index)) {
    _hash = "";
    _nonce = make_shared<atomic<uint64_t>>(0);
    _modified_hash = make_shared<atomic<bool>>(false);
    _mu = make_shared<mutex>();
}

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
            const auto dynamic_thread_count = minimum_thread_count + max_difficulty - std::abs(static_cast<int>(max_difficulty / difficulty));
            // for low-level difficulty, this is a safer number of threads to use, otherwise
            // it is likely to (even if only rarely) to obtain wrong output values of hashes
            if (difficulty < 2) {
                return dynamic_thread_count;
            }
            return max(hardware_thread_count, dynamic_thread_count);
        };
        return calculate_optimal();
    };

    // ...
	vector<thread> threads(optimal_thread_count());
    cout << "Running using " << threads.size() << " threads" << endl;

    // ...
    auto start = high_resolution_clock::now();

    // ..
    auto concurrent_calculate_hash = [difficulty,str,this]() {
        std::unique_lock<mutex> guard(*_mu, defer_lock);
        while (!_modified_hash->load()) {
            if (const string local_hash = calculate_hash();
                local_hash.substr(0, difficulty) == str
                && !_modified_hash->load()) {
                // ...
                {
                    guard.lock();
                    _hash = local_hash;
                }
                _modified_hash->store(true);
            }
        }
    };

    // submit tasks to the available threads
    for_each(begin(threads), end(threads), [concurrent_calculate_hash](auto &th) {
        th = thread(concurrent_calculate_hash);
    });

    // execute the thread tasks
    for_each(begin(threads), end(threads), [](auto &th) {
        if (th.joinable()) {
            th.join();
        }
    });

    // ...
    auto end = high_resolution_clock::now();
    const duration<double, micro> diff = end - start;
    cout << "Block mined: " << _hash << " in " << diff.count() << " microseconds\n";
}

string block::calculate_hash() const noexcept {
    stringstream ss;
    ss << _index << _time << _data << ++*_nonce << prev_hash;
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
