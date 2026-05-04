Installation
============

Prerequisites
-------------

- CMake 3.20 or newer.
- A C++20 compiler. The documented baseline is clang >= 14 or gcc >= 11.
- Git, when using CMake ``FetchContent`` to download this repo or
  ``polycpp``.
- OpenSSL development headers and libraries. bcrypt-pbkdf uses SHA-512
  through ``polycpp``, and polycpp links OpenSSL.
- Access to the base ``polycpp`` source. By default CMake fetches it from
  the configured Git remote; if that remote is unavailable or private in
  your environment, pass a local checkout with ``POLYCPP_SOURCE_DIR`` or
  ``FETCHCONTENT_SOURCE_DIR_POLYCPP``.
- Ninja is optional, but recommended for the examples below.

The currently documented and tested toolchains are clang and gcc on
Unix-like systems. MSVC and Windows builds are not covered by these docs or
project CI; treat them as unsupported until they are tested explicitly.

CMake FetchContent (recommended)
--------------------------------

Add the library to your ``CMakeLists.txt``:

.. code-block:: cmake

   include(FetchContent)

   FetchContent_Declare(
       polycpp_bcrypt_pbkdf
       GIT_REPOSITORY https://github.com/polycpp/bcrypt-pbkdf.git
       GIT_TAG        v1.0.0
   )
   FetchContent_MakeAvailable(polycpp_bcrypt_pbkdf)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::bcrypt_pbkdf)

The first configure pulls ``polycpp`` transitively. Keep ``GIT_TAG`` pinned
to a release tag or full commit SHA instead of a moving branch such as
``master`` so the same bcrypt-pbkdf source is used on every build. For a
fully reproducible dependency graph, also provide a pinned or local
``polycpp`` checkout with ``FETCHCONTENT_SOURCE_DIR_POLYCPP`` or
``POLYCPP_SOURCE_DIR``.

Using a local clone
-------------------

If you already have bcrypt-pbkdf and polycpp checked out side by side,
tell CMake to use them instead of fetching from GitHub:

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
    Build the GoogleTest suite. Defaults to ``ON`` for standalone builds
    and ``OFF`` when consumed via FetchContent.

``POLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES``
    Build the runnable examples under ``examples/``. Defaults to ``OFF``.

Verifying the install
---------------------

Standalone checkout
~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ctest --test-dir build --output-on-failure

All tests should pass on a supported toolchain — if they do not, open an
issue on the `repository <https://github.com/polycpp/bcrypt-pbkdf/issues>`_
with the compiler version and the failing test name.

Consumer project
~~~~~~~~~~~~~~~~

Confirm that your app links the exported target and can include the public
header:

.. code-block:: cmake

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::bcrypt_pbkdf)

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   int main() {
       const auto password = polycpp::Buffer::from("password");
       const auto salt = polycpp::Buffer::from("0123456789abcdef");
       const auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, 16, 48);
       return key.length() == 48 ? 0 : 1;
   }

Then configure, build, and run the executable from your build tree:

.. code-block:: bash

   cmake -S . -B build -G Ninja
   cmake --build build
   ./build/my_app

Troubleshooting
---------------

``Could NOT find OpenSSL`` or missing ``openssl/*.h``
    Install the OpenSSL development package for your platform, for example
    ``libssl-dev`` on Debian/Ubuntu or ``openssl-devel`` on Fedora/RHEL.
    Reconfigure from a clean build directory after installing it.

``CMake 3.20 or higher is required``
    Upgrade CMake or run the configure step with a newer CMake binary.

FetchContent cannot clone or says Git is missing
    Install Git and make sure it is on ``PATH``. For reproducible and more
    reliable fetches, pin ``GIT_TAG`` to a release tag or full commit SHA.
    If the network is unavailable, use the local clone workflow above.

FetchContent cannot clone ``polycpp`` because authentication is required
    Check out ``polycpp`` yourself and point this build at it with
    ``POLYCPP_SOURCE_DIR`` for standalone builds or
    ``FETCHCONTENT_SOURCE_DIR_POLYCPP`` from a consuming project.

Unknown target or missing bcrypt-pbkdf header
    Link ``polycpp::bcrypt_pbkdf`` exactly and include
    ``<polycpp/bcrypt_pbkdf.hpp>``. Detail headers and the build target name
    ``polycpp_bcrypt_pbkdf`` are not the public consumer interface.
