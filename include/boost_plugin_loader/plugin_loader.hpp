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
#ifndef BOOST_PLUGIN_LOADER_PLUGIN_LOADER_HPP
#define BOOST_PLUGIN_LOADER_PLUGIN_LOADER_HPP

// STD
#include <sstream>
#include <algorithm>
#include <mutex>
#include <utility>

// Boost
#include <boost/core/demangle.hpp>
#include <boost/dll/import.hpp>
#include <boost/version.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
/**
 * @brief Create a shared instance for the provided symbol_name loaded from the library_name searching system folders
 * for library
 * @param lib The library to search for available symbols
 * @param symbol_name The symbol from which to create a shared instance. This name is the alias provided to
 * EXPORT_CLASS_SECTIONED
 * @return A shared pointer of the object with the symbol name located in library_name
 */
template <class ClassBase>
static std::shared_ptr<ClassBase> createSharedInstance(const boost::dll::shared_library& lib,
                                                       const std::string& symbol_name)
{
  // Check if library has symbol
  if (!lib.has(symbol_name))
    throw PluginLoaderException("Failed to find symbol '" + symbol_name +
                                "' in library: " + boost::dll::shared_library::decorate(lib.location()).string());

#if BOOST_VERSION >= 108800
  return boost::dll::import_symbol<ClassBase>(lib, symbol_name);
#else
#if BOOST_VERSION >= 107600
  boost::shared_ptr<ClassBase> plugin = boost::dll::import_symbol<ClassBase>(lib, symbol_name);
#else
  boost::shared_ptr<ClassBase> plugin = boost::dll::import <ClassBase>(lib, symbol_name);
#endif
  return std::shared_ptr<ClassBase>(plugin.get(), [plugin](ClassBase*) mutable { plugin.reset(); });
#endif
}

/**
 * @brief Create a shared instance for the provided symbol_name from a library shared through the library registry
 * @details The instance aliases the shared library, keeping it loaded, instead of copying it as the overload taking a
 * boost::dll::shared_library does: a copy re-resolves the handle's path, which on macOS runs dlopen/dlclose under
 * dyld's process-wide lock.
 * @param lib The library to search for available symbols
 * @param symbol_name The symbol from which to create a shared instance. This name is the alias provided to
 * EXPORT_CLASS_SECTIONED
 * @return A shared pointer to the object with the symbol name, sharing ownership of the library
 */
template <class ClassBase>
static std::shared_ptr<ClassBase> createSharedInstance(const LoadedLibrary& lib, const std::string& symbol_name)
{
  // Check if library has symbol
  void* address = lib.symbols->address(*lib.library, symbol_name);
  if (address == nullptr)
    throw PluginLoaderException("Failed to find symbol '" + symbol_name +
                                "' in library: " + boost::dll::shared_library::decorate(lib.location).string());

  // Alias the shared library: the instance keeps it loaded without copying it (a copy re-resolves the handle).
  return std::shared_ptr<ClassBase>(lib.library, static_cast<ClassBase*>(address));
}

const std::vector<std::string>& SymbolIndex::symbols(const boost::filesystem::path& location,
                                                     const std::string& section)
{
  return sections_.findOrInsert(section, [&] { return getAllAvailableSymbols(location, section); });
}

void* SymbolIndex::address(const boost::dll::shared_library& library, const std::string& symbol_name)
{
  // Resolved without the index's lock held (see PublishedMap): the dynamic loader takes its own process-wide lock, and
  // holding both would order them. A concurrent resolve of the same name finds the same address.
  return addresses_.findOrInsert(symbol_name, [&]() -> void* {
    address_resolutions_.fetch_add(1, std::memory_order_relaxed);
    return library.has(symbol_name) ? static_cast<void*>(&library.get<char>(symbol_name)) : nullptr;
  });
}

std::size_t SymbolIndex::addressResolutions() const
{
  return address_resolutions_.load(std::memory_order_relaxed);
}

PluginLoader::PluginLoader(std::shared_ptr<LibraryRegistry> registry) : registry_(std::move(registry))
{
  if (registry_ == nullptr)
    throw PluginLoaderException("PluginLoader requires a LibraryRegistry; got null");
}

PluginLoader::PluginLoader(const PluginLoader& other)
  : search_system_folders(other.search_system_folders)
  , search_paths(other.search_paths)
  , search_libraries(other.search_libraries)
  , search_paths_env(other.search_paths_env)
  , search_libraries_env(other.search_libraries_env)
{
  std::scoped_lock lock{ libraries_mutex_, other.libraries_mutex_ };
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  libraries_ = other.libraries_;
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  registry_ = other.registry_;
}

PluginLoader& PluginLoader::operator=(const PluginLoader& other)
{
  // Self-assignment would lock libraries_mutex_ twice below.
  if (this == &other)
    return *this;

  search_system_folders = other.search_system_folders;
  search_paths = other.search_paths;
  search_libraries = other.search_libraries;
  search_paths_env = other.search_paths_env;
  search_libraries_env = other.search_libraries_env;

  std::scoped_lock lock{ libraries_mutex_, other.libraries_mutex_ };
  libraries_ = other.libraries_;
  registry_ = other.registry_;
  return *this;
}

// A moved-from loader keeps its registry (copied, not moved), so registry_ is never null.
PluginLoader::PluginLoader(PluginLoader&& other) noexcept
  : search_system_folders(other.search_system_folders)
  , search_paths(std::move(other.search_paths))
  , search_libraries(std::move(other.search_libraries))
  , search_paths_env(std::move(other.search_paths_env))
  , search_libraries_env(std::move(other.search_libraries_env))
{
  std::scoped_lock lock{ libraries_mutex_, other.libraries_mutex_ };
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  libraries_ = std::move(other.libraries_);
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  registry_ = other.registry_;
}

PluginLoader& PluginLoader::operator=(PluginLoader&& other) noexcept
{
  // Self-assignment would lock libraries_mutex_ twice below and move the search lists out of themselves.
  if (this == &other)
    return *this;

  search_system_folders = other.search_system_folders;
  search_paths = std::move(other.search_paths);
  search_libraries = std::move(other.search_libraries);
  search_paths_env = std::move(other.search_paths_env);
  search_libraries_env = std::move(other.search_libraries_env);

  std::scoped_lock lock{ libraries_mutex_, other.libraries_mutex_ };
  libraries_ = std::move(other.libraries_);
  registry_ = other.registry_;
  return *this;
}

template <class ClassBase>
typename std::enable_if<!has_getSection<ClassBase>::value, bool>::type
PluginLoader::hasSymbol(const LoadedLibrary& lib, const std::string& symbol_name) const
{
  return lib.symbols->address(*lib.library, symbol_name) != nullptr;
}

/**
 * @brief Checks that the library has the input symbol name and that the symbol is associated with the section defined
 * in the plugin class.
 */
template <class ClassBase>
typename std::enable_if<has_getSection<ClassBase>::value, bool>::type
PluginLoader::hasSymbol(const LoadedLibrary& lib, const std::string& symbol_name) const
{
  const std::vector<std::string>& symbols = lib.symbols->symbols(lib.location, ClassBase::getSection());
  return std::find(symbols.begin(), symbols.end(), symbol_name) != symbols.end();
}

/**
 * @brief Loads all libraries
 * @details This function first attempts to load the libraries specified as complete, absolute paths.
 * If the provided library name is not an absolute path, then this function searches for the library in each of the
 * provided local search paths. If the provided library name is not an absolute path and does not exist in the local
 * search paths, then this function optionally searches for the library in system directories.
 * @param library_names list of library names
 * @param search_paths_local list of local search paths in which to look for plugin libraries
 * @param search_system_folders flag indicating whether to look for plugins in system level folders
 * @param cache The loader's own entries, looked up first; registry entries it uses are copied into it
 * @param registry The registry to look libraries up in, and to load them into, when @p cache has no entry
 * @return list of libraries with the specified input names that could be found in the specified input directories.
 * Libraries specified with absolute paths will be returned first in the list before libraries found in local paths (but
 * in no particular order in at the front of the list).
 */
static std::vector<LoadedLibrary> loadLibraries(const std::vector<std::string>& library_names,
                                                const std::vector<std::string>& search_paths_local,
                                                const bool search_system_folders,
                                                std::unordered_map<std::string, LoadedLibrary>& cache,
                                                LibraryRegistry& registry)
{
  // The loader's own entry for `key`, else the registry's, which is added to the loader's cache. Only a hit in
  // neither loads and resolves the library; a hit in the loader's cache takes no lock shared with other loaders.
  //
  // The loader's entry points at the registry's library and symbol index through its own reference counts, each of
  // which holds a reference to the registry's: copies and plugin instances made through this loader then count
  // references on memory no other loader writes to. Sharing the registry's counts made every createInstance write to
  // the same memory from every thread.
  auto view = [&cache](const std::string& key, const LoadedLibrary& registered) -> LoadedLibrary {
    LoadedLibrary own{ std::shared_ptr<boost::dll::shared_library>(
                           registered.library.get(), [keep = registered.library](boost::dll::shared_library*) {}),
                       registered.location,
                       std::shared_ptr<SymbolIndex>(registered.symbols.get(),
                                                    [keep = registered.symbols](SymbolIndex*) {}) };
    return cache.emplace(key, std::move(own)).first->second;
  };
  auto find = [&cache, &registry, &view](const std::string& key) -> std::optional<LoadedLibrary> {
    auto it = cache.find(key);
    if (it != cache.end())
      return it->second;
    std::optional<LoadedLibrary> lib = registry.find(key);
    if (!lib.has_value())
      return std::nullopt;
    return view(key, lib.value());
  };
  auto load = [&registry, &find, &view](const std::string& key,
                                        const boost::filesystem::path& path) -> std::optional<LoadedLibrary> {
    std::optional<LoadedLibrary> lib = find(key);
    if (lib.has_value())
      return lib;
    lib = registry.load(key, path);
    if (!lib.has_value())
      return std::nullopt;
    return view(key, lib.value());
  };

  std::vector<LoadedLibrary> libraries;
  libraries.reserve(library_names.size());

  // Loop over each provided library name
  for (const std::string& library_name : library_names)
  {
    // First check if the library name is actually a complete, absolute path where the library is located
    {
      const boost::filesystem::path library_path(library_name);

      // is_absolute() is a string check; exists() is a stat call, so it runs only when the library is not yet loaded
      if (library_path.is_absolute())
      {
        std::optional<LoadedLibrary> lib = find(library_path.string());
        if (!lib.has_value() && boost::filesystem::exists(library_path))
          lib = load(library_path.string(), library_path);

        // If the library exists, add it to the output list and continue to the next library name
        if (lib.has_value())
        {
          // Libraries specified as absolute paths should appear first in the output list, so insert them at the front
          // of the list
          libraries.insert(libraries.begin(), std::move(lib.value()));
          continue;
        }
      }
    }

    // If the library name is not an absolute path, try finding the library at the path defined as the combination of
    // each local search path and the library name
    std::optional<LoadedLibrary> lib = std::nullopt;
    for (const std::string& search_path : search_paths_local)
    {
      const boost::filesystem::path library_path = boost::filesystem::path(search_path) / library_name;
      lib = load(library_path.string(), library_path);

      // If the library exists at this path, add the library to the output list and break out of the loop
      if (lib.has_value())
      {
        libraries.push_back(std::move(lib.value()));
        break;
      }
    }

    // If the library cannot be found in any of the local search paths, search in the system level directories for the
    // library (if enabled)
    if (!lib.has_value() && search_system_folders)
    {
      lib = load(library_name, library_name);
      if (lib.has_value())
        libraries.push_back(std::move(lib.value()));
    }
  }

  return libraries;
}

template <typename PluginBase>
void PluginLoader::reportErrorCommon(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                                     const std::vector<std::string>& search_paths,
                                     const std::vector<std::string>& search_libraries) const
{
  const std::string plugin_base_type = boost::core::demangle(typeid(PluginBase).name());
  msg << "Failed to create plugin instance '" << plugin_name << "' of type '" << plugin_base_type << "'\n";
  msg << "Search Paths " << std::string(search_system_folders ? "(including " : "(not including ")
      << "system folders)\n";

  for (const auto& path : search_paths)
    msg << "    - " << path << "\n";

  msg << "Search Libraries:\n";
  for (const auto& library : search_libraries)
    msg << "    - " << decorate(library) << "\n";
}

template <typename PluginBase>
typename std::enable_if_t<!has_getSection<PluginBase>::value, void>
PluginLoader::reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                          const std::vector<std::string>& search_paths,
                          const std::vector<std::string>& search_libraries) const
{
  return reportErrorCommon<PluginBase>(msg, plugin_name, search_system_folders, search_paths, search_libraries);
}

template <typename PluginBase>
typename std::enable_if_t<has_getSection<PluginBase>::value, void>
PluginLoader::reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                          const std::vector<std::string>& search_paths,
                          const std::vector<std::string>& search_libraries) const
{
  reportErrorCommon<PluginBase>(msg, plugin_name, search_system_folders, search_paths, search_libraries);

  // Add information about the available plugins
  const std::string plugin_base_type = boost::core::demangle(typeid(PluginBase).name());
  auto plugins = getAvailablePlugins(PluginBase::getSection());
  msg << "Available plugins of type '" << plugin_base_type << "':\n";
  for (const auto& p : plugins)
    msg << "    - " << p << "\n";
}

template <class PluginBase>
std::shared_ptr<PluginBase> PluginLoader::createInstance(const std::string& plugin_name) const
{
  // Check for environment variable for plugin definitions
  const std::vector<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::vector<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<LoadedLibrary> libraries = [&]() {
    std::scoped_lock lock(libraries_mutex_);
    return loadLibraries(library_names, search_paths_local, search_system_folders, libraries_, *registry_);
  }();

  // Create an instance of the plugin
  for (const auto& lib : libraries)
  {
    if (hasSymbol<PluginBase>(lib, plugin_name))
      return createSharedInstance<PluginBase>(lib, plugin_name);
  }

  std::stringstream msg;
  reportError<PluginBase>(msg, plugin_name, search_system_folders, search_paths_local, library_names);
  throw PluginLoaderException(msg.str());
}

bool PluginLoader::isPluginAvailable(const std::string& plugin_name) const
{
  // Check for environment variable for plugin definitions
  const std::vector<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::vector<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<LoadedLibrary> libraries = [&]() {
    std::scoped_lock lock(libraries_mutex_);
    return loadLibraries(library_names, search_paths_local, search_system_folders, libraries_, *registry_);
  }();

  // Check for the symbol name
  return std::any_of(libraries.begin(), libraries.end(),
                     [&](const auto& lib) { return lib.symbols->address(*lib.library, plugin_name) != nullptr; });
}

template <class PluginBase>
typename std::enable_if_t<has_getSection<PluginBase>::value, std::vector<std::string>>
PluginLoader::getAvailablePlugins() const
{
  return getAvailablePlugins(PluginBase::getSection());
}

std::vector<std::string> PluginLoader::getAvailablePlugins(const std::string& section) const
{
  // Check for environment variable for plugin definitions
  const std::vector<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::vector<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<LoadedLibrary> libraries = [&]() {
    std::scoped_lock lock(libraries_mutex_);
    return loadLibraries(library_names, search_paths_local, search_system_folders, libraries_, *registry_);
  }();

  // Populate the list of plugins
  std::vector<std::string> plugins;
  for (const auto& lib : libraries)
  {
    std::vector<std::string> lib_plugins = lib.symbols->symbols(lib.location, section);
    plugins.insert(plugins.end(), lib_plugins.begin(), lib_plugins.end());
  }

  return plugins;
}

std::vector<LibraryLifetimeToken> PluginLoader::acquireLibraryLifetimeTokens() const
{
  // Check for environment variable for plugin definitions
  const std::vector<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::vector<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries into the registry; each token shares a registered library's ownership
  std::vector<LoadedLibrary> libraries = [&]() {
    std::scoped_lock lock(libraries_mutex_);
    return loadLibraries(library_names, search_paths_local, search_system_folders, libraries_, *registry_);
  }();

  std::vector<LibraryLifetimeToken> tokens;
  tokens.reserve(libraries.size());
  for (auto& library : libraries)
    tokens.push_back({ library.location.string(), std::move(library.library) });

  return tokens;
}

std::vector<std::string> PluginLoader::getAvailableSections(bool include_hidden) const
{
  // Check for environment variable for plugin definitions
  const std::vector<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::vector<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<LoadedLibrary> libraries = [&]() {
    std::scoped_lock lock(libraries_mutex_);
    return loadLibraries(library_names, search_paths_local, search_system_folders, libraries_, *registry_);
  }();

  // Populate the list of sections
  std::vector<std::string> sections;
  for (const auto& lib : libraries)
  {
    std::vector<std::string> lib_sections = getAllAvailableSections(lib.location, include_hidden);
    sections.insert(sections.end(), lib_sections.begin(), lib_sections.end());
  }

  return sections;
}

int PluginLoader::count() const
{
  return static_cast<int>(getAllLibraryNames(search_libraries_env, search_libraries).size());
}

bool PluginLoader::empty() const
{
  return (count() == 0);
}

void PluginLoader::clear()
{
  std::scoped_lock lock(libraries_mutex_);
  libraries_.clear();
}

const std::shared_ptr<LibraryRegistry>& PluginLoader::registry() const
{
  return registry_;
}

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_PLUGIN_LOADER_HPP
