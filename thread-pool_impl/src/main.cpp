#include "block_chain.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::to_string;

auto main() -> int {
    block_chain bchain;
    for (auto i = 1; i < 21u; ++i) {
        cout << "Mining block " << i << "..." << endl;
        bchain.add_block(block(i, string("Block ") + to_string(i) + string(" Data")));
    }
    return EXIT_SUCCESS;
}