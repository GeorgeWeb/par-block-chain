#pragma once

#include <string>
#include <vector>

enum class thread_alloc : int { dynamic = 0, hardware = 1 };

class block final {
 public:
  block(uint32_t index, const std::string &data,
        thread_alloc option = thread_alloc::hardware);

  // Difficulty is the minimum number of zeros we require at the
  // start of the hash.
  void mine_block(uint32_t difficulty, uint32_t max_difficulty) noexcept;

  inline const std::string &get_hash() const noexcept { return _hash; }

  // Hash code of the previous block in the chain.
  std::string prev_hash;

 private:
  // The index of the block in the chain.
  uint32_t _index;
  // A modifier used to get a suitable block.
  mutable uint64_t _nonce;
  bool _modified_hash;
  // Data stored in the block.
  std::string _data;
  // Hash code of this block.
  std::string _hash;
  // Time code block was created.
  long _time;

  // The number of threads value.
  unsigned int _thread_num;
  // Option to determine a way to define/calculate the number of threads.
  thread_alloc _thread_alloc;

  std::string calculate_hash() const noexcept;
};

class block_chain final {
 private:
  uint32_t _difficulty;
  uint32_t _max_difficulty;
  std::vector<block> _chain;

  inline const block &get_last_block() const noexcept { return _chain.back(); }

 public:
  block_chain();
  block_chain(uint32_t difficulty, uint32_t max_difficulty);

  void add_block(block &&new_block) noexcept;
};