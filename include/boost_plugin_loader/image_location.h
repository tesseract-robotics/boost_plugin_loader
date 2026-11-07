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
#ifndef BOOST_PLUGIN_LOADER_IMAGE_LOCATION_H
#define BOOST_PLUGIN_LOADER_IMAGE_LOCATION_H

// Boost
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
/** @brief Thrown when no loaded image matches a library's handle */
class ImageNotFoundError : public PluginLoaderException
{
public:
  using PluginLoaderException::PluginLoaderException;
};

/**
 * @brief The path of the file the dynamic loader mapped for @p library: the same path as library.location()
 * @details On macOS boost::dll's location() scans dyld's image list from the oldest image, opening and closing each
 * until one returns the library's handle. Each dlclose runs dyld's garbage collector, so a library loaded after
 * hundreds of images costs milliseconds. This scans from the newest image with the same comparison, so a library that
 * was just loaded is found in a probe or two. It returns dyld's own name for the image, never the path the library was
 * requested by: with DYLD_LIBRARY_PATH set, dyld may have mapped another file.
 *
 * On other platforms location() resolves the handle directly; this returns it.
 * @param library A loaded library
 * @return The path of its file
 * @throws ImageNotFoundError if no loaded image has the library's handle (e.g. the library is not loaded)
 */
boost::filesystem::path imageLocation(const boost::dll::shared_library& library);

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_IMAGE_LOCATION_H
