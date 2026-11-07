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

// STD
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

// Boost
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/image_location.h>
#include <boost_plugin_loader/library_registry.h>
#include <boost_plugin_loader/loaded_library.h>
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
std::shared_ptr<LibraryRegistry> LibraryRegistry::global()
{
  static const std::shared_ptr<LibraryRegistry> registry = std::make_shared<LibraryRegistry>();
  return registry;
}

std::optional<LoadedLibrary> LibraryRegistry::find(const std::string& key) const
{
  std::scoped_lock lock(mutex_);
  auto it = libraries_.find(key);
  if (it == libraries_.end())
    return std::nullopt;
  return it->second;
}

std::optional<LoadedLibrary> LibraryRegistry::load(const std::string& key, const boost::filesystem::path& path)
{
  if (std::optional<LoadedLibrary> cached = find(key); cached.has_value())
    return cached;

  std::optional<boost::dll::shared_library> lib = loadLibrary(path);
  if (!lib.has_value())
    return std::nullopt;

  auto library = std::make_shared<boost::dll::shared_library>(std::move(lib.value()));
  boost::filesystem::path location = imageLocation(*library);
  LoadedLibrary loaded{ std::move(library), std::move(location), std::make_shared<SymbolIndex>() };

  std::scoped_lock lock(mutex_);
  return libraries_.try_emplace(key, std::move(loaded)).first->second;  // a concurrent load of `key` may have won
}

}  // namespace boost_plugin_loader
