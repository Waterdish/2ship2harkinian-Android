cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

project(2s2h VERSION 1.1.0 LANGUAGES C CXX)
include(CMake/2ship-cvars.cmake)
include(CMake/lus-cvars.cmake)
set(PROJECT_BUILD_NAME "Satoko Alfa" CACHE STRING "" FORCE)
set(PROJECT_TEAM "github.com/harbourmasters" CACHE STRING "" FORCE)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 2ship)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (CMAKE_C_COMPILER_LAUNCHER MATCHES "ccache|sccache")
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
    endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows|Linux")
    if(NOT DEFINED BUILD_CROWD_CONTROL)
        set(BUILD_CROWD_CONTROL OFF)
    endif()
endif()

# Enable the Gfx debugger in LUS to use libgfxd from ZAPDTR
set(GFX_DEBUG_DISASSEMBLER ON)

# Tell LUS we're using F3DEX_GBI_2 (in a way that doesn't break libgfxd)
set(GBI_UCODE F3DEX_GBI_2)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()
    message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

    if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
        OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32"))
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
endif()

################################################################################
# VCPKG dependencies for Windows
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
include(CMake/automate-vcpkg.cmake)

if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
	set(VCPKG_TRIPLET x86-windows-static)
	set(VCPKG_TARGET_TRIPLET x86-windows-static)
elseif ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
	set(VCPKG_TRIPLET x64-windows-static)
	set(VCPKG_TARGET_TRIPLET x64-windows-static)
endif()

vcpkg_bootstrap()
vcpkg_install_packages(zlib bzip2 libzip libpng sdl2 sdl2-net glew glfw3 nlohmann-json tinyxml2 spdlog)
endif()

################################################################################
# Global configuration types
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch|Android")
set(CMAKE_C_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
else()
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_OBJCXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

if(NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
else()
    message(STATUS ${CMAKE_SYSTEM_NAME})
endif()

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################
add_subdirectory(libultraship ${CMAKE_BINARY_DIR}/libultraship)
add_subdirectory(ZAPDTR/ZAPD ${CMAKE_BINARY_DIR}/ZAPD)
add_subdirectory(OTRExporter)
add_subdirectory(mm)

set_property(TARGET 2ship PROPERTY APPIMAGE_DESKTOP_FILE_TERMINAL YES)
set_property(TARGET 2ship PROPERTY APPIMAGE_DESKTOP_FILE "${CMAKE_SOURCE_DIR}/mm/linux/2s2h.desktop")
set_property(TARGET 2ship PROPERTY APPIMAGE_ICON_FILE "${CMAKE_BINARY_DIR}/2s2hIcon.png")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
install(PROGRAMS "${CMAKE_BINARY_DIR}/linux/2s2h.sh" DESTINATION . COMPONENT appimage)
install(FILES "${CMAKE_BINARY_DIR}/mm/2ship.o2r" DESTINATION . COMPONENT ship)
install(TARGETS ZAPD DESTINATION ./assets/extractor COMPONENT extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/extractor COMPONENT extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/extractor/xmls COMPONENT extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/filelists/" DESTINATION ./assets/extractor/filelists COMPONENT extractor)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ActorList_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT extractor)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ObjectList_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT extractor)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/SymbolMap_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT extractor)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/extractor COMPONENT 2s2h)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/extractor/xmls COMPONENT 2s2h)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/filelists/" DESTINATION ./assets/extractor/filelists COMPONENT 2s2h)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ActorList_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT 2s2h)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ObjectList_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT 2s2h)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/SymbolMap_MM.txt" DESTINATION ./assets/extractor/symbols COMPONENT 2s2h)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
file(COPY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor")
file(COPY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor/xmls")
file(COPY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/filelists/" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor/filelists")
file(COPY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ActorList_MM.txt" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor/symbols")
file(COPY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ObjectList_MM.txt" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor/symbols")
file(COPY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/SymbolMap_MM.txt" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/extractor/symbols")
endif()


find_package(Python3 COMPONENTS Interpreter)

# Target to generate OTRs
add_custom_target(
    ExtractAssets
    # CMake versions prior to 3.17 do not have the rm command, use remove instead for older versions
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:${CMAKE_VERSION},3.17>,remove,rm> -f mm.zip 2ship.o2r
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --non-interactive --xml-root ../mm/assets/xml --custom-otr-file 2ship.o2r "--custom-assets-path" ${CMAKE_CURRENT_SOURCE_DIR}/mm/assets/custom --port-ver "${CMAKE_PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -DSYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DTARGET_DIR="$<TARGET_FILE_DIR:ZAPD>" -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/copy-existing-otrs.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter
    COMMENT "Running asset extraction..."
    DEPENDS ZAPD
    BYPRODUCTS mm.zip ${CMAKE_SOURCE_DIR}/mm.o2r ${CMAKE_SOURCE_DIR}/2ship.o2r
)

# Target to generate headers
add_custom_target(
    ExtractAssetHeaders
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --non-interactive --xml-root ../mm/assets/xml --gen-headers
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter
    COMMENT "Generating asset headers..."
    DEPENDS ZAPD
)

# Target to generate only 2ship.o2r
add_custom_target(
    Generate2ShipOtr
    # CMake versions prior to 3.17 do not have the rm command, use remove instead for older versions
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:${CMAKE_VERSION},3.17>,remove,rm> -f 2ship.o2r
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --norom --custom-otr-file 2ship.o2r "--custom-assets-path" ${CMAKE_CURRENT_SOURCE_DIR}/mm/assets/custom --port-ver "${CMAKE_PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -DSYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DTARGET_DIR="$<TARGET_FILE_DIR:ZAPD>" -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_BINARY_DIR} -DONLY2SHIPOTR=On -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/copy-existing-otrs.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter
    COMMENT "Generating 2ship.o2r..."
    DEPENDS ZAPD
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
file(COPY ${CMAKE_SOURCE_DIR}/mm/linux/2s2hIcon.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
add_custom_target(CreateOSXIcons
   COMMAND mkdir -p ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset
   COMMAND sips -z 16 16     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_16x16.png
   COMMAND sips -z 32 32     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_16x16@2x.png
   COMMAND sips -z 32 32     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_32x32.png
   COMMAND sips -z 64 64     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_32x32@2x.png
   COMMAND sips -z 128 128   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_128x128.png
   COMMAND sips -z 256 256   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_128x128@2x.png
   COMMAND sips -z 256 256   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_256x256.png
   COMMAND sips -z 512 512   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_256x256@2x.png
   COMMAND sips -z 512 512   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_512x512.png
   COMMAND cp                mm/macosx/2s2hIcon.png ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_512x512@2x.png
   COMMAND iconutil -c icns -o ${CMAKE_BINARY_DIR}/macosx/2s2h.icns ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   COMMENT "Creating OSX icons ..."
   )
add_dependencies(2ship CreateOSXIcons)

install(TARGETS ZAPD DESTINATION ${CMAKE_BINARY_DIR}/assets/extractor)

set(PROGRAM_PERMISSIONS_EXECUTE OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/extractor/xmls)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/filelists/" DESTINATION ./assets/extractor/filelists)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ActorList_MM.txt" DESTINATION ./assets/extractor/symbols)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ObjectList_MM.txt" DESTINATION ./assets/extractor/symbols)
install(FILES "${CMAKE_SOURCE_DIR}/OTRExporter/CFG/SymbolMap_MM.txt" DESTINATION ./assets/extractor/symbols)

# install(DIRECTORY ${CMAKE_BINARY_DIR}/assets
#         DESTINATION .
#         PATTERN ZAPD.out
#         PERMISSIONS ${PROGRAM_PERMISSIONS_EXECUTE}
#         )

install(CODE "
   include(BundleUtilities)
   fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/2s2h-macos\" \"\" \"${dirs}\")
   ")

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows|NintendoSwitch|CafeOS")
install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION . COMPONENT 2s2h RENAME readme.txt )
install(CODE "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/mods\")" COMPONENT 2s2h)
install(CODE "file(TOUCH \"\${CMAKE_INSTALL_PREFIX}/mods/custom_mod_files_go_here.txt\")" COMPONENT 2s2h)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CPACK_GENERATOR "External")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows|NintendoSwitch|CafeOS")
    set(CPACK_GENERATOR "ZIP")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_GENERATOR "Bundle")
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/CMake/Packaging-2.cmake)
    include(CMake/Packaging.cmake)
endif()
