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

// Boost
#include <boost/dll/library_info.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>

// STD
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include <cstring>
#include <cstdlib>

// Boost Plugin Loader
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
std::optional<boost::dll::shared_library> loadLibrary(const boost::filesystem::path& library_path)
{
  boost::dll::load_mode::type mode{ boost::dll::load_mode::type::default_mode };

  if (!library_path.has_parent_path())
  {
    mode = boost::dll::load_mode::append_decorations | boost::dll::load_mode::search_system_folders;
  }
  else
  {
    mode = boost::dll::load_mode::append_decorations;
  }

  boost::system::error_code ec;
  boost::dll::shared_library lib = boost::dll::shared_library(library_path, ec, mode);
  if (ec)
    return std::nullopt;

  return lib;
}

std::vector<std::string> getAllAvailableSymbols(const boost::dll::shared_library& library, const std::string& section)
{
  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(library.location());

  // Getting symbols exported from he provided section
  return inf.symbols(section);
}

std::vector<std::string> getAllAvailableSections(const boost::dll::shared_library& library, bool include_hidden)
{
  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(library.location());

  // Getting section from library
  std::vector<std::string> sections = inf.sections();

  auto search_fn = [include_hidden](const std::string& section) {
    if (section.empty())
      return true;

    if (include_hidden)
      return false;

    return (section.substr(0, 1) == ".") || (section.substr(0, 2) == "__");
  };

  sections.erase(std::remove_if(sections.begin(), sections.end(), search_fn), sections.end());
  return sections;
}

std::string decorate(const std::string& library_name, const std::string& library_directory)
{
  boost::filesystem::path lib_path;
  if (library_directory.empty())
    lib_path = boost::filesystem::path(library_name);
  else
    lib_path = boost::filesystem::path(library_directory) / library_name;

  // Support when library_name is already full path
  if (lib_path.is_absolute())
    return lib_path.string();

  boost::filesystem::path actual_path =
      (std::strncmp(lib_path.filename().string().c_str(), "lib", 3) != 0 ?
           boost::filesystem::path((lib_path.has_parent_path() ? lib_path.parent_path() / L"lib" : L"lib").native() +
                                   lib_path.filename().native()) :
           lib_path);

  actual_path += boost::dll::shared_library::suffix();
  return actual_path.string();
}

std::vector<std::string> parseEnvironmentVariableList(const std::string& env_variable)
{
  char* env_var = std::getenv(env_variable.c_str());
  if (env_var == nullptr)  // Environment variable not found
    return {};

  std::string evn_str = std::string(env_var);
  std::vector<std::string> env_list;
#ifndef _WIN32
  boost::split(env_list, evn_str, boost::is_any_of(":"), boost::token_compress_on);
#else
  boost::split(env_list, evn_str, boost::is_any_of(";"), boost::token_compress_on);
#endif

  std::vector<std::string> list;
  list.insert(list.end(), env_list.begin(), env_list.end());
  return list;
}

std::vector<std::string> getAllSearchPaths(const std::string& search_paths_env,
                                           const std::vector<std::string>& existing_search_paths)
{
  // Check for environment variable to override default library
  if (!search_paths_env.empty())
  {
    std::vector<std::string> search_paths = parseEnvironmentVariableList(search_paths_env);
    search_paths.insert(search_paths.end(), existing_search_paths.begin(), existing_search_paths.end());
    return search_paths;
  }

  return existing_search_paths;
}

std::vector<std::string> getAllLibraryNames(const std::string& search_libraries_env,
                                            const std::vector<std::string>& existing_search_libraries)
{
  // Check for environment variable to override default library
  if (!search_libraries_env.empty())
  {
    std::vector<std::string> search_libraries = parseEnvironmentVariableList(search_libraries_env);
    search_libraries.insert(search_libraries.end(), existing_search_libraries.begin(), existing_search_libraries.end());
    return search_libraries;
  }

  return existing_search_libraries;
}

void addSymbolLibraryToSearchLibrariesEnv(const void* symbol_ptr, const std::string& search_libraries_env)
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
  setenv(search_libraries_env.c_str(), env_var_str.c_str(), 1);  // NOLINT(misc-include-cleaner)
#else
  _putenv_s(search_libraries_env.c_str(), env_var_str.c_str());  // NOLINT(misc-include-cleaner)
#endif
}

}  // namespace boost_plugin_loader
