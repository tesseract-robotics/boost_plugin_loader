/**
 * @file plugin_loader_concurrency_bench.cpp
 * @brief Throughput of createInstance on an already-loaded library, 1 thread vs N threads (#45).
 *
 * Each thread has its own PluginLoader, as each tesseract Environment clone has its own plugin factories. Without
 * contention N threads finish N times the work in the same wall time; a process-wide lock taken per call (dyld's, on
 * macOS) makes them serialise instead.
 *
 * Usage: plugin_loader_concurrency_bench [threads] [calls] [global|own]
 * "global" (the default) gives every loader the process-wide LibraryRegistry; "own" gives each loader its own registry,
 * the layout of the per-loader cache it replaced.
 *
 * @par License
 * Software License Agreement (Apache License)
 * @par
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * @par
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/plugin_loader.hpp>
#include "test_plugin.h"

namespace
{
// Default createInstance calls per thread: enough for a ~1 ms call (the pre-#45 macOS cost) to dominate thread
// start-up. A fast call needs more: pass the count as the second argument.
constexpr int DEFAULT_CALLS_PER_THREAD = 2000;

double runSeconds(int threads, int calls, bool own_registry)
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  std::vector<PluginLoader> loaders;
  loaders.reserve(static_cast<std::size_t>(threads));
  for (int i = 0; i < threads; ++i)
    loaders.emplace_back(own_registry ? std::make_shared<LibraryRegistry>() : LibraryRegistry::global());
  for (auto& loader : loaders)
  {
    loader.search_paths.emplace_back(PLUGIN_DIR);
    loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
    loader.createInstance<TestPluginMultiply>(getSymbolName());  // load once: measure the cached path only
  }

  const auto start = std::chrono::steady_clock::now();
  std::vector<std::thread> workers;
  workers.reserve(loaders.size());
  for (auto& loader : loaders)
    workers.emplace_back([&loader, calls] {
      for (int i = 0; i < calls; ++i)
        loader.createInstance<TestPluginMultiply>(getSymbolName());
    });
  for (auto& worker : workers)
    worker.join();
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}
}  // namespace

int main(int argc, char** argv)
{
  const int threads = argc > 1 ? std::atoi(argv[1]) : static_cast<int>(std::thread::hardware_concurrency());
  const int calls = argc > 2 ? std::atoi(argv[2]) : DEFAULT_CALLS_PER_THREAD;
  const std::string registry = argc > 3 ? argv[3] : "global";
  if (registry != "global" && registry != "own")
  {
    std::cerr << "registry must be 'global' or 'own', got '" << registry << "'\n";
    return EXIT_FAILURE;
  }
  const bool own_registry = registry == "own";
  const double one = runSeconds(1, calls, own_registry);
  const double many = runSeconds(threads, calls, own_registry);
  std::cout << "createInstance, cached library, " << registry << " registry, " << calls << " calls per thread\n"
            << "  1 thread:  " << one << " s (" << 1e6 * one / calls << " us/call)\n"
            << "  " << threads << " threads: " << many << " s; contention factor " << many / one
            << " (1.0 = perfect scaling, " << threads << " = fully serialised)\n";
  return 0;
}
