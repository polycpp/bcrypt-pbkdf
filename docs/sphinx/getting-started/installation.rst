Installation
============

bcrypt-pbkdf targets C++20 and builds with clang >= 14 or gcc >= 11. It
depends only on the base `polycpp <https://github.com/enricohuang/polycpp>`_
library (which transitively links OpenSSL for SHA-512). It introduces no
other runtime dependencies.

CMake FetchContent (recommended)
--------------------------------

Add the library to your ``CMakeLists.txt``:

.. code-block:: cmake

   include(FetchContent)

   FetchContent_Declare(
       polycpp_bcrypt_pbkdf
       GIT_REPOSITORY https://github.com/polycpp/bcrypt-pbkdf.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_bcrypt_pbkdf)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::bcrypt_pbkdf)

The first configure pulls ``polycpp`` transitively. Pin ``GIT_TAG`` to a
specific commit for reproducible builds.

Using a local clone
-------------------

If you already have bcrypt-pbkdf and polycpp checked out side by side, tell
CMake to use them instead of fetching from GitHub:

.. code-block:: bash

   # Building this repo directly
   cmake -B build -G Ninja \
       -DPOLYCPP_SOURCE_DIR=<polycpp checkout>

   # Consuming this repo through FetchContent
   cmake -B build -G Ninja \
       -DFETCHCONTENT_SOURCE_DIR_POLYCPP=<polycpp checkout> \
       -DFETCHCONTENT_SOURCE_DIR_POLYCPP_BCRYPT_PBKDF=<bcrypt-pbkdf checkout>

Build options
-------------

``POLYCPP_BCRYPT_PBKDF_BUILD_TESTS``
    Build the GoogleTest suite. Defaults to ``ON`` for standalone builds and
    ``OFF`` when consumed via FetchContent.

``POLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES``
    Build the runnable examples under ``examples/``. Defaults to ``OFF``.

Verifying the install
---------------------

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ctest --test-dir build --output-on-failure

All tests should pass on a supported toolchain — if they do not, open an
issue on the `repository <https://github.com/polycpp/bcrypt-pbkdf/issues>`_
with the compiler version and the failing test name.
