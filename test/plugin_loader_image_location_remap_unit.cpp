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

/*
 * Run only with DYLD_LIBRARY_PATH=REMAP_DIR (test/CMakeLists.txt sets it). REMAP_DIR holds a copy of the multiply test
 * library under the same file name, so dlopen of the library's absolute path in PLUGIN_DIR maps the copy instead: dyld
 * searches DYLD_LIBRARY_PATH for the leaf name before using the path it was given. The resolved location must name
 * the file dyld mapped, never the path that was requested.
 */

// GTest
#include <gtest/gtest.h>

// STD
#include <cstdlib>
#include <optional>
#include <string>

// Boost
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/image_location.h>
#include <boost_plugin_loader/utils.h>

TEST(BoostPluginLoaderImageLocationRemapUnit, LocationNamesTheMappedFile)  // NOLINT
{
  const char* dyld_library_path = std::getenv("DYLD_LIBRARY_PATH");  // NOLINT
  ASSERT_NE(dyld_library_path, nullptr) << "run through ctest, which sets DYLD_LIBRARY_PATH=" REMAP_DIR;
  ASSERT_EQ(std::string(dyld_library_path), REMAP_DIR);

  const boost::filesystem::path requested =
      boost::dll::shared_library::decorate(boost::filesystem::path(PLUGIN_DIR) / PLUGINS_MULTIPLY);
  const boost::filesystem::path copy = boost::filesystem::path(REMAP_DIR) / requested.filename();
  ASSERT_TRUE(requested.is_absolute());
  ASSERT_TRUE(boost::filesystem::exists(requested));
  ASSERT_TRUE(boost::filesystem::exists(copy));
  ASSERT_FALSE(boost::filesystem::equivalent(requested, copy));

  const std::optional<boost::dll::shared_library> lib = boost_plugin_loader::loadLibrary(requested);
  ASSERT_TRUE(lib.has_value());

  // dyld mapped the copy, not the requested file...
  const boost::filesystem::path location = lib->location();
  EXPECT_TRUE(boost::filesystem::equivalent(location, copy)) << location;
  EXPECT_FALSE(boost::filesystem::equivalent(location, requested)) << location;

  // ...and the resolver names it exactly as boost::dll does.
  EXPECT_EQ(boost_plugin_loader::imageLocation(lib.value()), location);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
