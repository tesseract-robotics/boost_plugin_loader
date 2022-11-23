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

#include <boost/core/demangle.hpp>
#include <boost/dll/import.hpp>
#include <sstream>

#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
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
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

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
  msg << "Failed to create plugin instance '" << plugin_name << "' of type '" << plugin_base_type << "'" << std::endl;
  msg << "Search Paths " << std::string(search_system_folders ? "(including " : "(not including ") << "system folders)"
      << std::endl;

  for (const auto& path : search_paths)
    msg << "    - " + path << std::endl;

  msg << "Search Libraries:" << std::endl;
  for (const auto& library : search_libraries)
    msg << "    - " + decorate(library) << std::endl;
}

template <typename PluginBase>
typename std::enable_if<!has_getSection<PluginBase>::value, void>::type
PluginLoader::reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                          const std::set<std::string>& search_paths,
                          const std::set<std::string>& search_libraries) const
{
  return reportErrorCommon<PluginBase>(msg, plugin_name, search_system_folders, search_paths, search_libraries);
}

template <typename PluginBase>
typename std::enable_if<has_getSection<PluginBase>::value, void>::type
PluginLoader::reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                          const std::set<std::string>& search_paths,
                          const std::set<std::string>& search_libraries) const
{
  reportErrorCommon<PluginBase>(msg, plugin_name, search_system_folders, search_paths, search_libraries);

  // Add information about the available plugins
  const std::string plugin_base_type = boost::core::demangle(typeid(PluginBase).name());
  auto plugins = getAvailablePlugins(PluginBase::getSection());
  msg << "Available plugins of type '" << plugin_base_type << "':" << std::endl;
  for (const auto& p : plugins)
    msg << "    - " + p << std::endl;
}

template <class PluginBase>
std::shared_ptr<PluginBase> PluginLoader::createInstance(const std::string& plugin_name) const
{
  // Check for environment variable for plugin definitions
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
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
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable to override default library
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
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
typename std::enable_if<has_getSection<PluginBase>::value, std::vector<std::string>>::type
PluginLoader::getAvailablePlugins() const
{
  return getAvailablePlugins(PluginBase::getSection());
}

std::vector<std::string> PluginLoader::getAvailablePlugins(const std::string& section) const
{
  // Check for environment variable for plugin definitions
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable to override default library
  std::vector<std::string> plugins;
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
      std::vector<std::string> lib_plugins = getAllAvailableSymbols(section, library, path);
      plugins.insert(plugins.end(), lib_plugins.begin(), lib_plugins.end());
    }
  }

  return plugins;
}

std::vector<std::string> PluginLoader::getAvailableSections(bool include_hidden) const
{
  std::vector<std::string> sections;

  // Check for environment variable for plugin definitions
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable to override default library
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      std::vector<std::string> lib_sections = getAllAvailableSections(library, path, include_hidden);
      sections.insert(sections.end(), lib_sections.begin(), lib_sections.end());
    }
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
