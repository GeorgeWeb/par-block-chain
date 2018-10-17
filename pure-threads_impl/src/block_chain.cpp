#include "block_chain.h"
#include "sha256.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cmath>

using namespace std;
using namespace std::chrono;

// Note that _time would normally be set to the time of the block's creation.
// This is part of the audit a block chain.  To enable consistent results
// from parallelisation we will just use the index value, so time increments
// by one each time: 1, 2, 3, etc.
block::block(uint32_t index, const string &data)
    : _index(index), _data(data), _nonce(0),
      _time(static_cast<long>(index)) {}

void block::mine_block(uint32_t difficulty) noexcept {
    string str(difficulty, '0');
    
    auto start = high_resolution_clock::now();

    const auto num_threads = thread::hardware_concurrency();
	vector<thread> threads;

	for (auto i = 0u; i < num_threads; ++i) {
		threads.push_back(thread(&block::calculate_hash, this, std::move(difficulty)));
	}

	for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    auto end = high_resolution_clock::now();
    duration<double, std::milli> diff = end - start;
    cout << "Block mined: " << _hash << " in " << diff.count() << " milliseconds\n";
}

void block::calculate_hash(uint32_t difficulty) noexcept {
    const string str(difficulty, '0');
    while (!_satisfies_diff) {
		stringstream ss;
		ss << _index << _time << _data << ++_nonce << prev_hash; 

        const string temp_hash = sha256(ss.str());
		if (temp_hash.substr(0, difficulty) == str) {
			_satisfies_diff = true;
			_hash = temp_hash;
		}
	}
}

block_chain::block_chain() : _difficulty(3) {
    _chain.emplace_back(block(0, "Genesis Block"));
}

void block_chain::add_block(block new_block) noexcept {
    new_block.prev_hash = get_last_block().get_hash();
    new_block.mine_block(_difficulty);
    _chain.push_back(std::move(new_block));
}