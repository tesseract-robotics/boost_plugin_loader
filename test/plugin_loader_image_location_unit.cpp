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

// GTest
#include <gtest/gtest.h>

// STD
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Boost
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/image_location.h>
#include <boost_plugin_loader/utils.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __APPLE__
TEST(BoostPluginLoaderImageLocationUnit, ImagesLoadedAtStartMatchLocation)  // NOLINT
{
  // Every image in the process before this test loads anything: the executable, its libraries and the system's.
  std::vector<std::string> names;
  const std::uint32_t count = _dyld_image_count();
  for (std::uint32_t i = 0; i < count; ++i)
    names.emplace_back(_dyld_get_image_name(i));
  ASSERT_GT(names.size(), 1);

  for (const std::string& name : names)
  {
    const boost::dll::shared_library lib(name);
    EXPECT_EQ(boost_plugin_loader::imageLocation(lib), lib.location()) << name;
  }
}
#endif

TEST(BoostPluginLoaderImageLocationUnit, LibrariesLoadedLateMatchLocation)  // NOLINT
{
  for (const char* name : { PLUGINS_MULTIPLY, PLUGINS_ADD })
  {
    const std::optional<boost::dll::shared_library> lib =
        boost_plugin_loader::loadLibrary(boost::filesystem::path(PLUGIN_DIR) / name);
    ASSERT_TRUE(lib.has_value()) << name;
    EXPECT_EQ(boost_plugin_loader::imageLocation(lib.value()), lib->location()) << name;
  }
}

TEST(BoostPluginLoaderImageLocationUnit, UnloadedLibraryThrows)  // NOLINT
{
  const boost::dll::shared_library not_loaded;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_THROW(boost_plugin_loader::imageLocation(not_loaded), boost_plugin_loader::ImageNotFoundError);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
