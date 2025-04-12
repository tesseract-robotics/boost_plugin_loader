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

// Boost
#include <boost/core/demangle.hpp>
#include <boost/dll/import.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/version.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{

/**
 * @brief Separates a set of library paths with mixed path specification into one set with libraries that are specified as fully-defined absolute paths and a second set with libraries that are specified by name only.
 * @param library_names The set to search and remove libraries with full paths
 * @return A tuple containing 1) the set of the libraries specified as fully-defined absolute paths, and 2) the set of remaining libraries specified as library names
 */
inline std::tuple<std::set<std::string>, std::set<std::string>> separateLibraryPathSpecifications(const std::set<std::string> &library_names)
{
  std::set<std::string> libraries_with_fullpath;
  std::set<std::string> libraries_without_fullpath;
  for (const auto& library_name : library_names)
  {
    if (boost::filesystem::exists(library_name) && boost::filesystem::path(library_name).is_absolute())
      libraries_with_fullpath.insert(library_name);
    else
      libraries_without_fullpath.insert(library_name);
  }

  return {libraries_with_fullpath, libraries_without_fullpath};
}

/**
 * @brief Create a shared instance for the provided symbol_name loaded from the library_name searching system folders
 * for library
 * @details The symbol name is the alias provide when calling EXPORT_CLASS_SECTIONED
 * @param symbol_name The symbol to create a shared instance of
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return A shared pointer of the object with the symbol name located in library_name_
 */
template <class ClassBase>
static std::shared_ptr<ClassBase> createSharedInstance(const std::string& symbol_name, const std::string& library_name,
                                                       const std::string& library_directory = "")
{
  // Get library
  const boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

  // Check if library has symbol
  if (!lib.has(symbol_name))
    throw PluginLoaderException("Failed to find symbol '" + symbol_name +
                                "' in library: " + decorate(library_name, library_directory));

#if BOOST_VERSION >= 107600
  boost::shared_ptr<ClassBase> plugin = boost::dll::import_symbol<ClassBase>(lib, symbol_name);
#else
  boost::shared_ptr<ClassBase> plugin = boost::dll::import<ClassBase>(lib, symbol_name);
#endif
  return std::shared_ptr<ClassBase>(plugin.get(), [plugin](ClassBase*) mutable { plugin.reset(); });
}

template <typename PluginBase>
void PluginLoader::reportErrorCommon(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                                     const std::set<std::string>& search_paths,
                                     const std::set<std::string>& search_libraries) const
{
  const std::string plugin_base_type = boost::core::demangle(typeid(PluginBase).name());
  msg << "Failed to create plugin instance '" << plugin_name << "' of type '" << plugin_base_type << "'\n";
  msg << "Search Paths " << std::string(search_system_folders ? "(including " : "(not including ") << "system folders)\n";

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
  const std::set<std::string> all_library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (all_library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for libraries provided as full paths. These are searched first
  std::set<std::string> libraries_with_fullpath;
  std::set<std::string> library_names;
  std::tie(libraries_with_fullpath, library_names) = separateLibraryPathSpecifications(all_library_names);
  for (const auto& library_fullpath : libraries_with_fullpath)
  {
    if (isSymbolAvailable(plugin_name, library_fullpath))
      return createSharedInstance<PluginBase>(plugin_name, library_fullpath);
  }

  // Check for environment variable for search paths
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      try
      {
        return createSharedInstance<PluginBase>(plugin_name, library, path);
      }
      catch (PluginLoaderException&)
      {
        continue;
      }
    }
  }

  // If not found in any of the provided search paths then search system folders if allowed
  if (search_system_folders)
  {
    for (const auto& library : library_names)
    {
      try
      {
        return createSharedInstance<PluginBase>(plugin_name, library);
      }
      catch (PluginLoaderException&)
      {
        continue;
      }
    }
  }

  std::stringstream msg;
  reportError<PluginBase>(msg, plugin_name, search_system_folders, search_paths_local, library_names);
  throw PluginLoaderException(msg.str());
}

bool PluginLoader::isPluginAvailable(const std::string& plugin_name) const
{
  // Check for environment variable for plugin definitions
  const std::set<std::string> all_library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (all_library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for libraries provided as full paths. These are searched first
  std::set<std::string> libraries_with_fullpath;
  std::set<std::string> library_names;
  std::tie(libraries_with_fullpath, library_names) = separateLibraryPathSpecifications(all_library_names);
  for (const auto& library_fullpath : libraries_with_fullpath)
  {
    if (isSymbolAvailable(plugin_name, library_fullpath))
      return true;
  }

  // Check for environment variable to override default library
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      if (isSymbolAvailable(plugin_name, library, path))
        return true;
    }
  }

  // If not found in any of the provided search paths then search system folders if allowed
  if (search_system_folders)
  {
    for (const auto& library : library_names)
    {
      if (isSymbolAvailable(plugin_name, library))
        return true;
    }
  }

  return false;
}

template <class PluginBase>
typename std::enable_if_t<has_getSection<PluginBase>::value, std::vector<std::string>>
PluginLoader::getAvailablePlugins() const
{
  return getAvailablePlugins(PluginBase::getSection());
}

std::vector<std::string> PluginLoader::getAvailablePlugins(const std::string& section) const
{
  std::vector<std::string> plugins;

  // Check for environment variable for plugin definitions
  const std::set<std::string> all_library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (all_library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for libraries provided as full paths. These are searched first
  std::set<std::string> libraries_with_fullpath;
  std::set<std::string> library_names;
  std::tie(libraries_with_fullpath, library_names) = separateLibraryPathSpecifications(all_library_names);
  for (const auto& library_fullpath : libraries_with_fullpath)
  {
    // Attempt to load the library
    const std::optional<boost::dll::shared_library> lib = tryLoadLibrary(library_fullpath, "");

            // Skip if the library doesn't exist or cannot be opened
    if (!lib.has_value())
      continue;

    std::vector<std::string> lib_plugins = getAllAvailableSymbols(section, lib.value());
    plugins.insert(plugins.end(), lib_plugins.begin(), lib_plugins.end());
  }

  // Check for environment variable to override default library
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  if (search_paths_local.empty())
  {
    if (!search_system_folders)
      throw PluginLoaderException("No plugin search paths were provided!");

    // Insert an empty string into the search paths set to look in system folders
    search_paths_local.insert(std::string{});
  }

  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      // Attempt to load the library
      const std::optional<boost::dll::shared_library> lib = tryLoadLibrary(library, path);

              // Skip if the library doesn't exist or cannot be opened
      if (!lib.has_value())
        continue;

      std::vector<std::string> lib_plugins = getAllAvailableSymbols(section, lib.value());
      plugins.insert(plugins.end(), lib_plugins.begin(), lib_plugins.end());
    }
  }

  return plugins;
}

std::vector<std::string> PluginLoader::getAvailableSections(bool include_hidden) const
{
  std::vector<std::string> sections;

  // Check for environment variable for plugin definitions
  const std::set<std::string> all_library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (all_library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for libraries provided as full paths. These are searched first
  std::set<std::string> libraries_with_fullpath;
  std::set<std::string> library_names;
  std::tie(libraries_with_fullpath, library_names) = separateLibraryPathSpecifications(all_library_names);
  for (const auto& library_fullpath : libraries_with_fullpath)
  {
    // Attempt to load the library
    const std::optional<boost::dll::shared_library> lib = tryLoadLibrary(library_fullpath, "");

    // Skip if the library doesn't exist or cannot be opened
    if (!lib.has_value())
      continue;

    std::vector<std::string> lib_sections = getAllAvailableSections(lib.value(), include_hidden);
    sections.insert(sections.end(), lib_sections.begin(), lib_sections.end());
  }

  // Check for environment variable to override default library
  const std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      // Attempt to load the library
      const std::optional<boost::dll::shared_library> lib = tryLoadLibrary(library, path);

      // Skip if the library doesn't exist or cannot be opened
      if (!lib.has_value())
        continue;

      std::vector<std::string> lib_sections = getAllAvailableSections(lib.value(), include_hidden);
      sections.insert(sections.end(), lib_sections.begin(), lib_sections.end());
    }
  }

  return sections;
}

void PluginLoader::addSymbolLibraryToSearchLibrariesEnv(const void* symbol_ptr, const std::string& search_libraries_env)
{
  std::string env_var_str;
  char* env_var = std::getenv(search_libraries_env.c_str());
  if (env_var != nullptr)
  {
    env_var_str = env_var;
  }

  const boost::filesystem::path lib_path = boost::filesystem::canonical(boost::dll::symbol_location_ptr(symbol_ptr));

  if (env_var_str.empty())
  {
    env_var_str = lib_path.string();
  }
  else
  {
#ifndef _WIN32
    env_var_str = env_var_str + ":" + lib_path.string();
#else
    env_var_str = env_var_str + ";" + lib_path.string();
#endif
  }

#ifndef _WIN32
  setenv(search_libraries_env.c_str(), env_var_str.c_str(), 1);
#else
  _putenv_s(search_libraries_env.c_str(), env_var_str.c_str());
#endif
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
