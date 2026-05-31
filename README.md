Requirements
------------
Visual Studio 2022
vcpkg
SDL2
SDL2_image

Setup
-----
setx VCPKG_ROOT C:/Dev/vcpkg

cmake --preset vs2022-x64-vcpkg
cmake --build build --config Release
