#include "block_chain.h"
#include "sha256.h"

#include "../util/timer.hpp"
#include "../util/file_io.hpp"

#include <iostream>
#include <sstream>
#include <cmath>
#include <numeric>
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
}

void block::mine_block(uint32_t difficulty) noexcept {
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


    // create a thread pool object
    static tpool::std_queue::thread_pool pool{optimal_thread_count()};
    cout << "Running using " << pool.count() << " threads" << endl;

    // declares a list to hold the execution time for each iteration of a task
    std::vector<double> totals;
    
    // declares a ...
    vector<future<void>> work_items(pool.count());

    // lambda helper function that calculates the block's hash value in a concurrent/thread-safe manner
    const auto concurrent_calculate_hash = [difficulty,this]() {
        // string to compare
        static const string str(difficulty, '0');
        while (!_modified_hash->load()) {
            if (const string local_hash = calculate_hash();
                local_hash.substr(0, difficulty) == str
                && !_modified_hash->load()) {
                _modified_hash->store(true);
                _hash = local_hash;
            }
        }
    };

    // define file name
    static string filename("results-tpool-diff" + std::to_string(difficulty) + ".csv");
    // begin calculation timing
    util::timer<double, micro> per_task_timer{};

    // submits task threads in the pool a number of (iterations) times and store
    for_each(begin(work_items), end(work_items), [concurrent_calculate_hash,&per_task_timer](auto &item) {
        per_task_timer.reset();
        item = pool.add_task(concurrent_calculate_hash);
    });

    // executes the task threads from the pool
    for_each(begin(work_items), end(work_items), [&totals,per_task_timer](auto &item) {
        item.get();
        totals.push_back(per_task_timer.get_elapsed());
    });

    /* TODO: interface changes in thread pool:
    pool.parallel_for([](auto& item) {
        // allocate work to the threads in the pool:
        // function body here: ...
    });
    pool.execute([](const auto& id) {
        // allocate work to the threads in the pool:
        // get result here
        // or no param -> simply executes void tasks
    });
    */

    // end timer and print results
    const auto average = accumulate(begin(totals), end(totals), 0.0) / static_cast<double>(totals.size());
    cout << "Block mined: " << _hash << " in " << average << endl;
    // save to file
    util::file_io::get().save<double>(average, filename);
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
