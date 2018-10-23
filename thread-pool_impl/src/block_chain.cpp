#include "block_chain.h"
#include "sha256.h"

#include <iostream>
#include <sstream>
#include <chrono>

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
          pool = make_shared<tpool::std_queue::thread_pool>(); // 8 on my CPU
      }

void block::mine_block(uint32_t difficulty) noexcept {
    string str(difficulty, '0');
    constexpr auto iters = 10u;

    // begin clock
    auto start = high_resolution_clock::now();

    vector<future<void>> hashings;
    for(auto i = 0u; i < iters; ++i)
    {
        auto job = pool->enqueue([difficulty,str,this]() {
            while (!_modified_hash->load()) {
                string local_hash = calculate_hash();
                if (local_hash.substr(0, difficulty) == str) {
                    _modified_hash->store(true);
                    _hash = local_hash;
                }
            }
        });
        
        // add to the list of pool jobs
        hashings.push_back(move(job));
    }

    auto count = 0u;
    double total = 0.0;
    for(auto &hash : hashings) {
        hash.get();

        // end clock
        duration<double, milli> diff = high_resolution_clock::now() - start;
        cout << "Iteration " << ++count << ":" << endl;
        cout << "Block mined: " << _hash << " in " << diff.count() << " milliseconds" << endl;
        // sum total
        total += diff.count();
    }
    // calculate average time for each block's hash to get calculated
    duration<double, milli> diff = high_resolution_clock::now() - start;
    cout << "Total (AVG) block execution time: " << total / static_cast<double>(iters) << " milliseconds\n";
}

string block::calculate_hash() const noexcept {
    stringstream ss;
    ss << _index << _time << _data << ++*_nonce << prev_hash;
    return sha256(ss.str());
}

block_chain::block_chain() : _difficulty(5) {
    _chain.emplace_back(block(0, "Genesis Block"));
}

void block_chain::add_block(block new_block) noexcept {
    new_block.prev_hash = get_last_block().get_hash();
    new_block.mine_block(_difficulty);
    _chain.push_back(std::move(new_block));
}
