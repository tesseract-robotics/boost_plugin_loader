/**
 * @file plugin_loader_image_count_bench.cpp
 * @brief Cost of the first createInstance through a new PluginLoader as a function of the number of loaded images
 * (macOS only).
 *
 * boost::dll resolves a library handle's path on macOS by dlopen/dlclose of every loaded image until one matches, so
 * resolving a location costs more the more images the process has loaded. A cache kept in each PluginLoader paid that
 * for every new loader; the process-wide LibraryRegistry resolves it once per library, with imageLocation(), which
 * scans from the newest image.
 *
 * Usage: plugin_loader_image_count_bench <extra_images> [reps] [sources]
 * sources is a colon-separated list of "frameworks" (/System/Library/Frameworks, the default) and directories whose
 * *.dylib files are loaded, used in order. Images are loaded from them until the
 * process has at least <extra_images> more images, then loads the
 * test plugin library, so the plugin sits after them in dyld's image list as it does in an application that loads its
 * plugins late. Run once per image count: an image cannot be unloaded to shrink the list again.
 *
 * Columns (microseconds; medians over reps unless noted):
 *   first_registry  the process's first createInstance of the plugin, through the global registry: dlopen, location
 *                   by imageLocation(), and the symbol index's first read of the file (one sample)
 *   per_loader      first createInstance through a new loader with its own registry: the library is already loaded,
 *                   so a dlopen reference, imageLocation() and a symbol read
 *   registry        first createInstance through a new loader using the global registry, after the global registry
 *                   has loaded the library once
 *   location        boost::dll location() of the loaded handle (scans from the oldest image)
 *   image_location  imageLocation() of the loaded handle (scans from the newest image)
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
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <dlfcn.h>
#include <mach-o/dyld.h>

#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <boost_plugin_loader/image_location.h>
#include <boost_plugin_loader/library_registry.h>
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/plugin_loader.hpp>
#include <boost_plugin_loader/utils.h>
#include "test_plugin.h"

namespace
{
using Clock = std::chrono::steady_clock;

/** @brief Default repetitions per timed column */
constexpr int DEFAULT_REPS = 50;

double microseconds(Clock::time_point start, Clock::time_point stop)
{
  return std::chrono::duration<double, std::micro>(stop - start).count();
}

double median(std::vector<double> samples)
{
  std::sort(samples.begin(), samples.end());
  return samples[samples.size() / 2];
}

/**
 * @brief Load images from @p source until the process has @p target images
 * @param target The image count to reach
 * @param source "frameworks" for /System/Library/Frameworks (dyld shared cache images), or a directory whose *.dylib
 * files are loaded (images outside the shared cache, as a conda or Python process loads)
 */
void loadImagesFrom(std::uint32_t target, const std::string& source)
{
  const bool frameworks = source == "frameworks";
  for (const auto& entry : boost::filesystem::directory_iterator(frameworks ? boost::filesystem::path("/System/Library/"
                                                                                                      "Frameworks") :
                                                                              boost::filesystem::path(source)))
  {
    if (_dyld_image_count() >= target)
      return;
    const boost::filesystem::path path = entry.path();
    if (path.extension() != (frameworks ? ".framework" : ".dylib"))
      continue;
    const boost::filesystem::path binary = frameworks ? path / path.stem() : path;
    // The handle is deliberately kept open: the image must stay in the list. A library that fails to load (a missing
    // dependency) adds no image and is skipped; loadExtraImages fails loud if too few load.
    dlopen(binary.c_str(), RTLD_LAZY | RTLD_LOCAL);  // NOLINT
  }
}

/**
 * @brief Load images until the process has at least @p extra more images than now
 * @param extra The number of images to add
 * @param sources Colon-separated sources, used in order; see loadImagesFrom
 */
void loadExtraImages(std::uint32_t extra, const std::string& sources)
{
  const std::uint32_t target = _dyld_image_count() + extra;
  std::size_t begin = 0;
  while (begin <= sources.size() && _dyld_image_count() < target)
  {
    const std::size_t end = std::min(sources.find(':', begin), sources.size());
    loadImagesFrom(target, sources.substr(begin, end - begin));
    begin = end + 1;
  }
  if (_dyld_image_count() < target)
    throw std::runtime_error("Could not load " + std::to_string(extra) + " extra images; reached " +
                             std::to_string(_dyld_image_count()));
}

boost_plugin_loader::PluginLoader makeLoader(std::shared_ptr<boost_plugin_loader::LibraryRegistry> registry)
{
  boost_plugin_loader::PluginLoader loader(std::move(registry));
  loader.search_paths.emplace_back(PLUGIN_DIR);
  loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
  return loader;
}
}  // namespace

int main(int argc, char** argv)
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::TestPluginMultiply;

  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <extra_images> [reps] [sources]\n";  // NOLINT
    return EXIT_FAILURE;
  }
  const auto extra = static_cast<std::uint32_t>(std::stoul(argv[1]));  // NOLINT
  const int reps = argc > 2 ? std::stoi(argv[2]) : DEFAULT_REPS;       // NOLINT
  const std::string source = argc > 3 ? argv[3] : "frameworks";        // NOLINT
  const boost::filesystem::path plugin_path = boost::filesystem::path(PLUGIN_DIR) / PLUGINS_MULTIPLY;

  loadExtraImages(extra, source);
  const std::uint32_t images = _dyld_image_count();

  // The process's first load of the plugin library, through the global registry, which keeps it loaded.
  const auto first_start = Clock::now();
  makeLoader(LibraryRegistry::global()).createInstance<TestPluginMultiply>(getSymbolName());
  const double first_registry_us = microseconds(first_start, Clock::now());

  const std::optional<boost_plugin_loader::LoadedLibrary> loaded =
      LibraryRegistry::global()->find(plugin_path.string());
  if (!loaded.has_value())
    throw std::runtime_error("The global registry has no entry for " + plugin_path.string());
  const boost::dll::shared_library& first = *loaded->library;
  const boost::filesystem::path first_location = first.location();
  if (loaded->location != first_location)
    throw std::runtime_error("registry location " + loaded->location.string() + " differs from location() " +
                             first_location.string());

  std::vector<double> per_loader;
  std::vector<double> registry;
  std::vector<double> location;
  std::vector<double> reverse;
  for (int r = 0; r < reps; ++r)
  {
    {
      auto loader = makeLoader(std::make_shared<LibraryRegistry>());
      auto start = Clock::now();
      loader.createInstance<TestPluginMultiply>(getSymbolName());
      per_loader.push_back(microseconds(start, Clock::now()));
    }
    {
      auto loader = makeLoader(LibraryRegistry::global());
      auto start = Clock::now();
      loader.createInstance<TestPluginMultiply>(getSymbolName());
      registry.push_back(microseconds(start, Clock::now()));
    }
    {
      auto start = Clock::now();
      const boost::filesystem::path p = first.location();
      location.push_back(microseconds(start, Clock::now()));
      if (p != first_location)
        throw std::runtime_error("location() changed");
    }
    {
      auto start = Clock::now();
      const boost::filesystem::path p = boost_plugin_loader::imageLocation(first);
      reverse.push_back(microseconds(start, Clock::now()));
      if (p != first_location)
        throw std::runtime_error("imageLocation() disagrees with location(): " + p.string() + " vs " +
                                 first_location.string());
    }
  }

  std::cout << std::fixed << std::setprecision(1) << "source=" << source << " images=" << images << " extra=" << extra
            << " first_registry=" << first_registry_us << " per_loader=" << median(per_loader)
            << " registry=" << median(registry) << " location=" << median(location)
            << " image_location=" << median(reverse) << " (us)\n";
  return EXIT_SUCCESS;
}
