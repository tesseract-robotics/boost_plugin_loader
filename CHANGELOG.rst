^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package boost_plugin_loader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.4.1 (2025-10-14)
------------------
* Fix ubuntu CI pipeline
* Contributors: Levi Armstrong

0.4.0 (2025-10-14)
------------------
* Leverage library cache (`#36 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/36>`_)
  Co-authored-by: Michael Ripperger <michael.ripperger@swri.org>
* Bug fix so search path order is preserved (`#35 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/35>`_)
* Contributors: Levi Armstrong

0.3.2 (2025-08-04)
------------------
* Ensure symbol is associated with defined section (`#33 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/33>`_)
  * Added hasSymbol method to check if the symbol exists in the library and the section associated with the plugin
  * Added compile definitions for plugin section and symbol; changed name of test plugin base class; update unit tests
  * Added second plugin for addition with same name and different section; added unit test to validate section checking functionality
  * Ran cmake format
  * Updates to support Windows and Mac
* Update ubuntu.yml
* Contributors: Levi Armstrong, Michael Ripperger

0.3.1 (2025-07-09)
------------------
* Update to colcon-action v13
* Add macos CI (`#27 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/27>`_)
* Update dependencies.repos (`#30 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/30>`_)
* Fixed shared_ptr type selection in createSharedInstance (`#29 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/29>`_)
* Fix missing algorithm.h header in plugin_loader.hpp (`#26 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/26>`_)
* Example update (`#19 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/19>`_)
* Contributors: John Wason, Levi Armstrong, Michael Ripperger

0.3.0 (2025-04-22)
------------------
* Add forward declaration header
* Update badges in readme
* Add codecov.yml file
* Update use of target_code_coverage to fix codecov build
* Move addSymbolLibraryToSearchLibrariesEnv to utils
* Add back cpack build to ubuntu.yml
* Fix clang-tidy error in utils.cpp
* Update CI to push codecov report
* Static plugin loading using symbol module resolution (`#21 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/21>`_)
  * Static plugin loading using symbol moodule resolution
  * Avoid exceptions in isSymbolAvailable
  * Removed deprecated functions; revised loadLibrary function
  * Add method to plugin loader to load valid libraries in the correct order; revised implementation of plugin loader functions to use library loader function
  * Updated function signature of getAllAvailableSymbols
  * Return libraries specified by absolute paths at the front of the return list
  * Update createSharedInstance signature to match other function
  * Fix macro formatting
  * Add unit test which uses full path for library name
  * Add plugin achor unit test
  * Update decorate to support library name as full path
  * Add name to clang format CI
  ---------
  Co-authored-by: Michael Ripperger <michael.ripperger@swri.org>
* CI Update (`#20 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/20>`_)
  * Updated clang format job
  * Updated Ubuntu CI job to run on focal, jammy, and noble
  * Update to non-deprecated version of upload artifact action
  * Changed name of variable per clang-tidy
* Removed duplicate entry in clang-format file (`#18 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/18>`_)
* Contributors: Levi Armstrong, Michael Ripperger

0.2.2 (2024-01-16)
------------------
* Catch exceptions when enumerating possible symbols and sections (`#17 <https://github.com/marip8/boost_plugin_loader/issues/17>`_)
* CI Update (`#12 <https://github.com/marip8/boost_plugin_loader/issues/12>`_)
* Change CI to run on Ubuntu 20.04 (`#15 <https://github.com/marip8/boost_plugin_loader/issues/15>`_)
* Contributors: Michael Ripperger

0.2.1 (2022-12-09)
------------------
* Improved error messaging (`#14 <https://github.com/marip8/boost_plugin_loader/issues/14>`_)
* Only catch plugin loader exception
* Fix issue not using library names returned from getAllLibraryNames
* Contributors: Levi Armstrong, Michael Ripperger

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
