^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package boost_plugin_loader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.2.0 (2022-06-23)
------------------
* Update package CI
* Add cassert include to example
* Update package.xml
* Updates (`#3 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/3>`_)
  * Updated example and README
  * Replace pragma once with header guard
  * Remove include of implementation in header
  * Simplify test plugin getSection
* Contributors: Levi Armstrong, Michael Ripperger

0.1.1 (2022-06-21)
------------------
* Add cpack
* update windows ci
* update ubuntu focal CI build
* add std::enable_if to PluginLoader::getAvailablePlugins()
* update package.xml
* Update unit test to have full coverage
* Add PluginLoaderException class
* Update parseEnvironmentVariableList documentation
* Fixed search in system directories for plugins
* Switch got getSection method
* Rename getAllAvailablePlugins
* Port over @marip8 example
* Switch SECTION_NAME to section as member variable
* Add code coverage to ubunut CI build
* Rename to align with @marip8 refactor
* Remove ClassLoader class
* port macros.h
* Initial port from tesseract_common
* Initial commit
* Contributors: Levi Armstrong
