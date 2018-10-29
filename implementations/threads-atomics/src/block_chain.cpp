#include "block_chain.h"
#include "sha256.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

// Note that _time would normally be set to the time of the block's creation.
// This is part of the audit a block chain.  To enable consistent results
// from parallelisation we will just use the index value, so time increments
// by one each time: 1, 2, 3, etc.
block::block(uint32_t index, const string &data, thread_alloc option)
    : _index(index),
      _data(data),
      _nonce(0),
      _time(static_cast<long>(index)),
      _thread_alloc(option) {
  _hash = "";
  _nonce = make_shared<atomic<uint64_t>>(0);
  _modified_hash = make_shared<atomic<bool>>(false);
  _mu = make_shared<mutex>();
}

void block::mine_block(uint32_t difficulty, uint32_t max_difficulty) noexcept {
  // Lambda that *dynamically* returns optimal thread count depending on the
  // difficulty level.
  const auto optimal_thread_count = [difficulty,
                                     max_difficulty]() -> unsigned int {
    constexpr auto potentially_used_thread_count = 1u;  // assuming: main()
    constexpr auto minimum_thread_count = 2u;
    // Lambda that performs the calculation,
    const auto calculate_optimal = [difficulty, max_difficulty] {
      const auto hardware_thread_count =
          thread::hardware_concurrency() - potentially_used_thread_count;
      const auto dynamic_thread_count =
          minimum_thread_count + max_difficulty -
          abs(static_cast<int>(max_difficulty / difficulty));
      // Configurations for lower-level difficulties.
      if (difficulty < 2) {
        return dynamic_thread_count;
      } else if (difficulty == 2) {
        return static_cast<unsigned int>(pow(minimum_thread_count, difficulty));
      }
      return max(hardware_thread_count, dynamic_thread_count);
    };
    return calculate_optimal();
  };

  // Get the number of threads to run.
  if (_thread_alloc == thread_alloc::hardware) {
    _thread_num = thread::hardware_concurrency();
  } else {
    _thread_num = optimal_thread_count();
  }

  // Declares a vector of threads to run.
  vector<thread> threads(_thread_num);
  cout << "Running using " << threads.size() << " threads" << endl;

  // Lambda helper function that calculates the block's hash value in a
  // concurrent/thread-safe manner.
  auto concurrent_calculate_hash = [difficulty, this]() {
    const string str(difficulty, '0');
    std::unique_lock<mutex> guard(*_mu, defer_lock);
    // Calculate the correct hash value.
    while (!_modified_hash->load()) {
      if (const string local_hash = calculate_hash();
          local_hash.substr(0, difficulty) == str && !_modified_hash->load()) {
        {
          guard.lock();
          _hash = local_hash;
        }
        _modified_hash->store(true);
      }
    }
  };

  // Submits tasks to the threads.
  for_each(begin(threads), end(threads), [concurrent_calculate_hash](auto &th) {
    th = thread(concurrent_calculate_hash);
  });

  // Executes the threads.
  for_each(begin(threads), end(threads), [](auto &th) {
    if (th.joinable()) {
      th.join();
    }
  });

  cout << "Block mined: " << _hash << endl;
}

string block::calculate_hash() const noexcept {
  stringstream ss;
  ss << _index << _time << _data << ++*_nonce << prev_hash;
  return sha256(ss.str());
}

block_chain::block_chain() : _difficulty(1), _max_difficulty(1) {
  _chain.emplace_back(block(0, "Genesis Block"));
}

block_chain::block_chain(uint32_t difficulty, uint32_t max_difficulty)
    : _difficulty(difficulty), _max_difficulty(max_difficulty) {
  _chain.emplace_back(block(0, "Genesis Block"));
}

void block_chain::add_block(block &&new_block) noexcept {
  new_block.prev_hash = get_last_block().get_hash();
  new_block.mine_block(_difficulty, _max_difficulty);
  _chain.push_back(new_block);
}
