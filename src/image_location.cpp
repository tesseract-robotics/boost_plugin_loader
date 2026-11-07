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
#include <cstddef>
#include <cstdint>
#include <string>

// Boost
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

#ifdef __APPLE__
#include <dlfcn.h>
#include <mach-o/dyld.h>
#endif

// Boost Plugin Loader
#include <boost_plugin_loader/image_location.h>

namespace boost_plugin_loader
{
#ifdef __APPLE__
namespace
{
/** @brief A dlopen handle without its two mode bits, as boost::dll's path_from_handle compares handles */
void* stripHandle(void* handle)
{
  return reinterpret_cast<void*>((reinterpret_cast<std::ptrdiff_t>(handle) >> 2) << 2);  // NOLINT
}
}  // namespace

boost::filesystem::path imageLocation(const boost::dll::shared_library& library)
{
  void* const handle = stripHandle(library.native());
  if (handle != nullptr)
  {
    // Newest first: a library is appended to the image list when it is loaded. RTLD_NOLOAD returns the handle of an
    // image that is already loaded and never loads one; an image unloaded by another thread meanwhile yields null
    // (or a null name) and is skipped.
    for (std::uint32_t i = _dyld_image_count(); i-- > 0;)
    {
      const char* image_name = _dyld_get_image_name(i);
      if (image_name == nullptr)
        continue;
      void* probe = dlopen(image_name, RTLD_LAZY | RTLD_NOLOAD);
      if (probe == nullptr)
        continue;
      dlclose(probe);
      if (stripHandle(probe) == handle)
        return image_name;
    }
  }
  throw ImageNotFoundError("No loaded image has the handle of library '" +
                           std::to_string(reinterpret_cast<std::uintptr_t>(library.native())) +  // NOLINT
                           "'; is it loaded?");
}
#else
boost::filesystem::path imageLocation(const boost::dll::shared_library& library)
{
  if (!library.is_loaded())
    throw ImageNotFoundError("Library is not loaded");
  return library.location();
}
#endif

}  // namespace boost_plugin_loader
