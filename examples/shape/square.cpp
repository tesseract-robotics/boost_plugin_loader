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
#include "shape.h"

namespace boost_plugin_loader
{
/**
 * @brief Square shape implementation
 */
class Square : public Shape
{
public:
  Square(double w) : width(w)
  {
  }

  double area() const override
  {
    return width * width;
  }

protected:
  double width;
};

/**
 * @brief Square factory plugin implementation
 */
class SquareFactory : public ShapeFactory
{
  std::shared_ptr<Shape> create(const std::any& params) const override
  {
    double width = std::any_cast<double>(params);
    return std::make_shared<Square>(width);
  }
};

}  // namespace boost_plugin_loader

// Export the factory as the plugin to allow for multiple differently configured instances of the Square shape
EXPORT_SHAPE_PLUGIN(boost_plugin_loader::SquareFactory, Square)
