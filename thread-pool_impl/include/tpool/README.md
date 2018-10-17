# tpool.hpp
A very simple ***single-header*** C++17 implementation of a thread-pool

---

### Usage
```cpp
#include "paht/to/tpool.hpp"

#include <iostream>
 
/** This is a 3-step program example that showcases the use of the `tpool` library,
  * and while this is a very simple case, it can be used in a similar way for a
  * in more complex scenarios with multiple iterations over certain tasks
 **/
auto main() -> int {
  /* Step 1: Construct a default thread pool object */
  
  // Option 1:
  // implementation with manual (in-place) std::queue::push/pop synchronisation
  // the slighly faster version, which is also considerably safe for most trivial cases
  // tpool::std_queue::thread_pool pool{};
  
  // Option 2:
  // implementation with a thread-safe queue, internally composed of std::queue
  // the slightly safer version, for cases where we need to double-ensue correct ordering
  tpool::safe_queue::thread_pool pool{};

  /* Step 2: Set variables to be used in the functions passed to the pool */

  constexpr auto num1 = 12.25f;
  constexpr auto num2 = 17.75f;
  
  /* Submit tasks to the thread-pool */

  // enqueue lambdas to the pool which will give them a thread if there's any unused threads left
  // there sure are enough, unless you're running on a single-core CPU, where 
  // a default pool construction might create a pool of only 1 thread
  auto func1 = pool.enqueue([num1] { return num1; });
  auto func2 = pool.enqueue([num2] { return num2; });
  
  /* Step 3: Use the outputs from the tasks that were executed within the thread-pool */

  // the return values from the enqueued lambdas are futures, thus use std::future::get() to retrieve them
  std::cout << "Result: " << func1.get() + func2.get() << std::endl;

  return 0;
}
```

---

### Compilation
#### Linux
**gcc**
```
g++ -std=c++17 -pthread example.cpp -o example
```
**clang**
```
clang++ -std=c++17 -pthread example.cpp -o example
```
#### Windows
**Visual Studio 2017** with enabled multi-threading
