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

#include "test_plugin_base.h"
#include "test_plugin_multiply.h"

// Boost Plugin Loader
#include <boost_plugin_loader/macros.h>

namespace boost_plugin_loader
{
double TestPluginMultiply::multiply(double x, double y)
{
  return x * y;
}

PLUGIN_ANCHOR_IMPL(TestPluginMultiplyAnchor)

}  // namespace boost_plugin_loader

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
EXPORT_TEST_PLUGIN(boost_plugin_loader::TestPluginMultiply, plugin)
