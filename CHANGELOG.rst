^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package boost_plugin_loader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.5.0 (2026-10-30)
------------------
* Merge pull request `#46 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/46>`_ from tesseract-robotics/shared-library-cache
  Share cached libraries with plugin instances instead of copying them
* CI: run macOS on macos-15; Homebrew dropped bottles for macos-14
* Apply cmake-format
* Potential fix for pull request finding 'Link benchmark target against Threads::Threads'
  Co-authored-by: Copilot Autofix powered by AI <175728472+Copilot@users.noreply.github.com>
* Test createSharedInstance from a LoadedLibrary, including a missing symbol
* Apply clang-format
* Share cached libraries with plugin instances instead of copying them
  createInstance on an already-loaded library cost ~1 ms on macOS and serialised
  across threads (`#45 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/45>`_). Per call it copied boost::dll::shared_library (dlopen /
  dlclose under dyld's process-wide lock via path_from_handle), re-read and parsed
  the library file for its section symbols, and stat'ed every library name.
  - The cache holds one shared_ptr<shared_library> per library with its location,
  resolved once at load. Instances alias it; lifetime tokens share it.
  - A per-library SymbolIndex reads each section's symbols from the file once.
  - is_absolute() before exists(); a cached absolute path skips the stat.
  macOS arm64, 8 threads with a loader each: 1020 us -> 0.5 us per call,
  contention factor 8 -> ~2.5 (test/plugin_loader_concurrency_bench.cpp).
  createSharedInstance(shared_library, name) is unchanged.
  Closes `#45 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/45>`_
* Contributors: Jelle Feringa

0.4.5 (2026-09-09)
------------------
* Add plugin library lifetime tokens
  Add an explicit API for eagerly loading configured plugin libraries and
  returning independent RAII lifetime tokens.
  Each token includes the resolved library path for deduplication and retains
  type-erased shared ownership after the originating PluginLoader is cleared
  or destroyed.
  Add unit coverage for token acquisition and ownership persistence.
* Cancel superseded pull-request CI runs
  Five workflows trigger on push, pull_request and a daily cron with no
  concurrency group, so every push to an open PR left the previous run
  building. Supersede in-progress pull-request runs only; pushes and cron
  runs still queue so two runs on one ref never race.
  Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
* Bump colcon-action to v15 on Windows
  v15 is the first release with Windows ccache support, so this is the
  only platform gaining caching it never had. The -G "Ninja" already
  passed in both arg sets is the precondition: the Visual Studio generator
  ignores CMAKE\_<LANG>_COMPILER_LAUNCHER silently, and the only symptom
  would be zero cacheable calls.
  The prefix named matrix.distro, a key this single-leg job never defined,
  so it expanded to the empty string.
  Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
* Bump colcon-action to v15 on Linux and macOS
  ccache has never worked on either platform. The save step never ran, so
  every run was cold, and the launcher never reached the compiler: on Linux
  it was passed in a second --cmake-args group, which colcon drops as
  last-wins; on macOS `if [ ... = "true" ]]` errors and forced the
  no-launcher branch. That same bracket also skipped rosdep install, so
  fixing it would newly run rosdep against a package.xml declaring
  libboost-filesystem-dev, which has no Homebrew mapping and nothing to
  install there - Boost comes from vcpkg. Disable it on macOS instead.
  CCACHE_DIR was load-bearing under v14, whose cache path was relative;
  v15 resolves the directory itself, so drop it.
  Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
* Contributors: Levi Armstrong, Roelof Oomen

0.4.4 (2026-08-21)
------------------
* Add BUILD_EXAMPLES option
* Dropped CI support for Ubuntu 20.04; added CI support for Ubuntu 26.04 (`#40 <https://github.com/tesseract-robotics/boost_plugin_loader/issues/40>`_)
  * Dropped CI support for Ubuntu 20.04; added CI support for Ubuntu 26.04
  * Bump colcon-action to v14
* Contributors: Michael Ripperger, Rick van Essen

0.4.3 (2025-11-03)
------------------
* Fixed finding multiple libraries during library loading
* Contributors: David Spielman

0.4.2 (2025-10-14)
------------------
* Fix debian and archive name
* Contributors: Levi Armstrong

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
