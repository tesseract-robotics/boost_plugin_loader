/**
 *
 * @copyright Copyright (c) 2021, Southwest Research Institute
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
#ifndef BOOST_PLUGIN_LOADER_LIBRARY_REGISTRY_H
#define BOOST_PLUGIN_LOADER_LIBRARY_REGISTRY_H

// STD
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

// Boost
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/loaded_library.h>

namespace boost_plugin_loader
{
/**
 * @brief The plugin libraries loaded in this process, shared by every PluginLoader that uses the registry.
 * @details A library is loaded and its location resolved once per registry; every later lookup, from any loader, is a
 * hash lookup. Resolving the location was the expensive step: on macOS boost::dll resolves a handle's path by
 * dlopen/dlclose of every loaded image, so its cost grows with the number of images in the process, and a cache that
 * lived in each PluginLoader paid it again for every new loader. The registry resolves it with imageLocation(), which
 * finds a just-loaded library among the newest images.
 *
 * The registry holds strong references: an entry, and the library it keeps loaded, lives as long as the registry. The
 * process-wide registry, global(), lives until the process exits. A loader given its own registry shares nothing with
 * the global one, and its libraries are released, once no plugin instance or lifetime token holds them, when that
 * registry is destroyed.
 *
 * Entries are keyed by the path string a library was loaded from (an absolute path, a search path joined with the
 * library name, or the bare library name for a system search), as the per-loader cache was.
 *
 * Thread-safe.
 */
class LibraryRegistry
{
public:
  LibraryRegistry() = default;
  ~LibraryRegistry() = default;
  LibraryRegistry(const LibraryRegistry&) = delete;
  LibraryRegistry& operator=(const LibraryRegistry&) = delete;
  LibraryRegistry(LibraryRegistry&&) = delete;
  LibraryRegistry& operator=(LibraryRegistry&&) = delete;

  /**
   * @brief The process-wide registry, used by every PluginLoader that is not given its own
   * @details Defined in the compiled boost_plugin_loader library, so all images that link it share one instance.
   */
  static std::shared_ptr<LibraryRegistry> global();

  /**
   * @brief The entry for @p key, if a library was loaded under it
   * @param key The path string the library was loaded from
   * @return The entry, or nullopt; never loads
   */
  std::optional<LoadedLibrary> find(const std::string& key) const;

  /**
   * @brief The entry for @p key, loading @p path and resolving its location if there is none
   * @details The load runs without the registry's lock held, so a library whose static initialisers use a PluginLoader
   * cannot deadlock. If two threads load the same key at once, the first entry stored wins and the other handle is
   * released.
   * @param key The path string the library is loaded from
   * @param path The path passed to boost::dll
   * @return The entry, or nullopt if the library could not be loaded
   */
  std::optional<LoadedLibrary> load(const std::string& key, const boost::filesystem::path& path);

private:
  mutable std::mutex mutex_;
  /** @brief Loaded libraries by the path string they were loaded from; never evicted */
  std::unordered_map<std::string, LoadedLibrary> libraries_;
};

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_LIBRARY_REGISTRY_H
