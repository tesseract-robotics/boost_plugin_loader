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
#ifndef BOOST_PLUGIN_LOADER_LOADED_LIBRARY_H
#define BOOST_PLUGIN_LOADER_LOADED_LIBRARY_H

// STD
#include <atomic>
#include <cstddef>
#include <string>
#include <memory>
#include <vector>

// Boost
#include <boost/dll/shared_library.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/published_map.h>

namespace boost_plugin_loader
{
/**
 * @brief What has been looked up in a loaded library, resolved once and shared by every copy of its LoadedLibrary: the
 * symbols of each section, read from its file, and the address of each symbol name, resolved through the dynamic
 * loader.
 * @details A loaded library's file cannot change under it, so the first read stays valid; re-reading it on every
 * createInstance cost more than the rest of the call. Nor can its symbols move while it stays loaded, so the first
 * resolve stays valid too; every resolve through the dynamic loader takes its process-wide lock. A missing symbol is
 * remembered as missing (nullptr).
 *
 * An address is only valid while the library stays loaded. The index is only used through a LoadedLibrary, which
 * holds the library, and plugin instances alias that library.
 *
 * Every loader in the process shares one index per library, and every createInstance reads it, so a lookup takes no
 * lock (see PublishedMap).
 */
class SymbolIndex
{
public:
  SymbolIndex() = default;
  ~SymbolIndex() = default;
  SymbolIndex(const SymbolIndex&) = delete;
  SymbolIndex& operator=(const SymbolIndex&) = delete;
  SymbolIndex(SymbolIndex&&) = delete;
  SymbolIndex& operator=(SymbolIndex&&) = delete;

  /**
   * @brief The symbols exported under @p section, read from @p location on first use
   * @param location The path of the library file
   * @param section The section name
   * @return The symbols; valid for the life of this index
   */
  inline const std::vector<std::string>& symbols(const boost::filesystem::path& location, const std::string& section);

  /**
   * @brief The address of @p symbol_name in @p library, resolved on first use
   * @param library The loaded library this index belongs to
   * @param symbol_name The symbol name
   * @return The symbol's address, or nullptr if @p library does not export it
   */
  inline void* address(const boost::dll::shared_library& library, const std::string& symbol_name);

  /**
   * @brief How many times address() has resolved a name through the dynamic loader, for diagnostics and tests
   * @details One per name, plus one per thread that lost a race to publish the same name first.
   */
  inline std::size_t addressResolutions() const;

private:
  PublishedMap<std::vector<std::string>> sections_;
  PublishedMap<void*> addresses_;
  std::atomic<std::size_t> address_resolutions_{ 0 };
};

/**
 * @brief A loaded plugin library, shared by the library registry, every plugin instance created from it and every
 * lifetime token.
 * @details Plugin instances alias @p library instead of copying it: copying a boost::dll::shared_library re-resolves
 * the handle's path, which on macOS runs dlopen/dlclose under dyld's process-wide lock. @p location is resolved once,
 * at load time, for the same reason.
 */
struct LoadedLibrary
{
  std::shared_ptr<boost::dll::shared_library> library; /**< The loaded library */
  boost::filesystem::path location;                    /**< Its resolved path */
  std::shared_ptr<SymbolIndex> symbols;                /**< Its symbols per section and their addresses */
};

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_LOADED_LIBRARY_H
