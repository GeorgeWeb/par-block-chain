#include "block_chain.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::to_string;

auto main() -> int {
    constexpr auto max_difficulty = 5u;
    constexpr auto iters = 50u;
    constexpr auto num_blocks = 50u;

    for (auto diff_i = 1u; diff_i < max_difficulty + 1u; ++diff_i) {
        for (auto iter_i = 0u; iter_i < iters; ++iter_i) {
            block_chain bchain(diff_i);
            for (auto block_i = 1u; block_i < num_blocks + 1u; ++block_i) {
                cout << "Mining block " << block_i << " at difficulty " << diff_i << " ..." << endl;
                bchain.add_block(block(block_i, string("Block ") + to_string(block_i) + string(" Data")));
            }
        }
    }
    return EXIT_SUCCESS;
}
