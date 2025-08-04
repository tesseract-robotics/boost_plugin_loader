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

template <class ClassBase>
typename std::enable_if<!has_getSection<ClassBase>::value, bool>::type
PluginLoader::hasSymbol(const boost::dll::shared_library& lib, const std::string& symbol_name) const
{
  return lib.has(symbol_name);
}

/**
 * @brief Checks that the library has the input symbol name and that the symbol is associated with the section defined
 * in the plugin class.
 */
template <class ClassBase>
typename std::enable_if<has_getSection<ClassBase>::value, bool>::type
PluginLoader::hasSymbol(const boost::dll::shared_library& lib, const std::string& symbol_name) const
{
  std::vector<std::string> symbols = getAllAvailableSymbols(lib, ClassBase::getSection());
  std::sort(symbols.begin(), symbols.end());
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
 * @return list of libraries with the specified input names that could be found in the specified input directories.
 * Libraries specified with absolute paths will be returned first in the list before libraries found in local paths (but
 * in no particular order in at the front of the list).
 */
static std::vector<boost::dll::shared_library> loadLibraries(const std::set<std::string>& library_names,
                                                             const std::set<std::string>& search_paths_local,
                                                             const bool search_system_folders)
{
  std::vector<boost::dll::shared_library> libraries;

  // Loop over each provided library name
  for (const std::string& library_name : library_names)
  {
    // First check if the library name is actually a complete, absolute path where the library is located
    {
      const boost::filesystem::path library_path(library_name);

      if (boost::filesystem::exists(library_path) && library_path.is_absolute())
      {
        const std::optional<boost::dll::shared_library> lib = loadLibrary(library_path);

        // If the library exists, add it to the output list and continue to the next library name
        if (lib.has_value())
        {
          // Libraries specified as absolute paths should appear first in the output list, so insert them at the front
          // of the list
          libraries.insert(libraries.begin(), lib.value());
          continue;
        }
      }
    }

    // If the library name is not an absolute path, try finding the library at the path defined as the combination of
    // each local search path and the library name
    std::optional<boost::dll::shared_library> lib = std::nullopt;
    for (const std::string& search_path : search_paths_local)
    {
      const boost::filesystem::path library_path = boost::filesystem::path(search_path) / library_name;
      lib = loadLibrary(library_path);

      // If the library exists at this path, add the library to the output list and break out of the loop
      if (lib.has_value())
      {
        libraries.push_back(lib.value());
        break;
      }
    }

    // If the library cannot be found in any of the local search paths, search in the system level directories for the
    // library (if enabled)
    if (lib == std::nullopt && search_system_folders)
    {
      lib = loadLibrary(library_name);

      // Add the library to the output list, and break out of the loop
      if (lib.has_value())
        libraries.push_back(lib.value());
    }
  }

  return libraries;
}

template <typename PluginBase>
void PluginLoader::reportErrorCommon(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                                     const std::set<std::string>& search_paths,
                                     const std::set<std::string>& search_libraries) const
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
                          const std::set<std::string>& search_paths,
                          const std::set<std::string>& search_libraries) const
{
  return reportErrorCommon<PluginBase>(msg, plugin_name, search_system_folders, search_paths, search_libraries);
}

template <typename PluginBase>
typename std::enable_if_t<has_getSection<PluginBase>::value, void>
PluginLoader::reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                          const std::set<std::string>& search_paths,
                          const std::set<std::string>& search_libraries) const
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
  const std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<boost::dll::shared_library> libraries =
      loadLibraries(library_names, search_paths_local, search_system_folders);

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
  const std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<boost::dll::shared_library> libraries =
      loadLibraries(library_names, search_paths_local, search_system_folders);

  // Check for the symbol name
  return std::any_of(libraries.begin(), libraries.end(), [&](const auto& lib) { return lib.has(plugin_name); });
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
  const std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<boost::dll::shared_library> libraries =
      loadLibraries(library_names, search_paths_local, search_system_folders);

  // Populate the list of plugins
  std::vector<std::string> plugins;
  for (const auto& lib : libraries)
  {
    std::vector<std::string> lib_plugins = getAllAvailableSymbols(lib, section);
    plugins.insert(plugins.end(), lib_plugins.begin(), lib_plugins.end());
  }

  return plugins;
}

std::vector<std::string> PluginLoader::getAvailableSections(bool include_hidden) const
{
  // Check for environment variable for plugin definitions
  const std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);

  // Load the libraries
  const std::vector<boost::dll::shared_library> libraries =
      loadLibraries(library_names, search_paths_local, search_system_folders);

  // Populate the list of sections
  std::vector<std::string> sections;
  for (const auto& lib : libraries)
  {
    std::vector<std::string> lib_sections = getAllAvailableSections(lib, include_hidden);
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

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_PLUGIN_LOADER_HPP
