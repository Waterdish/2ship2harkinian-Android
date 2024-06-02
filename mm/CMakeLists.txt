cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(2ship LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    enable_language(OBJCXX)
    set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-arc")
    set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} -fobjc-arc")
endif()

set (BUILD_UTILS OFF CACHE STRING "no utilities")
set (BUILD_SHARED_LIBS OFF CACHE STRING "install/link shared instead of static libs")

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
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_C_FLAGS "")
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

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
if (NOT TARGET libultraship)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libultraship ${CMAKE_BINARY_DIR}/libultraship)
endif()

if (NOT TARGET ZAPDLib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ZAPDTR/ZAPD ${CMAKE_BINARY_DIR}/ZAPD)
endif()

set(PROJECT_NAME 2ship)

################################################################################
# Sources
################################################################################
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src/boot/build.c.in ${CMAKE_CURRENT_SOURCE_DIR}/src/boot/build.c @ONLY)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/windows/properties.h.in ${CMAKE_CURRENT_SOURCE_DIR}/windows/properties.h @ONLY)

#set(Header_Files "resource.h")
source_group("headers" FILES ${Header_Files})

# include {{{
file(GLOB Header_Files__include "include/*.h" "include/*.inc")
list(APPEND Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/stdarg.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/bgm.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/math_n64.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/stdbool_n64.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/stddef_n64.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/stdlib_n64.h)
list(REMOVE_ITEM Header_Files__include ${CMAKE_CURRENT_SOURCE_DIR}/include/ultra64.h)
source_group("include" FILES ${Header_Files__include})
# }}}

# m (root)
file(GLOB soh__ RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "2s2h/*.c" "2s2h/*.cpp" "2s2h/*.h" "2s2h/*.hpp")
source_group("2s2h" FILES ${soh__})

file(GLOB_RECURSE libultra_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "include/PR/*.h"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_source_files_properties(2s2h/BenPort.cpp PROPERTIES COMPILE_FLAGS "/utf-8")
endif()

# 2s2h/DeveloperTools {{{
file(GLOB_RECURSE soh__DeveloperTools RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
   "2s2h/DeveloperTools/*.h"
   "2s2h/DeveloperTools/*.c"
   "2s2h/DeveloperTools/*.cpp"
   "2s2h/DeveloperTools/*.hpp"
)
# }}}

# soh/config {{{
#file(GLOB_RECURSE soh__config RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#    "soh/config/*.h"
#    "soh/config/*.cpp"
#)
# }}}

# soh/enhancements {{{
file(GLOB_RECURSE soh__Enhancements RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "2s2h/Enhancements/*.c"
    "2s2h/Enhancements/*.cpp"
    "2s2h/Enhancements/*.h"
    "2s2h/Enhancements/*.hpp"
    "2s2h/Enhancements/*_extern.inc"
    "2s2h/Enhancements/*.mm"
)

file(GLOB_RECURSE soh__BenGui RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "2s2h/BenGui/*.c"
    "2s2h/BenGui/*.cpp"
    "2s2h/BenGui/*.h"
    "2s2h/BenGui/*.hpp"
)

file(GLOB_RECURSE soh__SaveManager RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "2s2h/SaveManager/*.c"
    "2s2h/SaveManager/*.cpp"
    "2s2h/SaveManager/*.h"
    "2s2h/SaveManager/*.hpp"
)

#list(REMOVE_ITEM soh__Enhancements "soh/Enhancements/gamecommand.h")
#list(FILTER soh__Enhancements EXCLUDE REGEX "soh/Enhancements/gfx.*")

# handle crowd control removals
#list(REMOVE_ITEM soh__Enhancements "soh/Enhancements/crowd-control/soh.cs")
#list(REMOVE_ITEM soh__Enhancements "soh/Enhancements/crowd-control/soh.ccpak")
#if (!BUILD_CROWD_CONTROL)
#    list(FILTER soh__Enhancements EXCLUDE REGEX "soh/Enhancements/crowd-control/*")
#endif()

# handle speechsynthesizer removals
#if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
#    list(FILTER soh__Enhancements EXCLUDE REGEX "soh/Enhancements/speechsynthesizer/Darwin*")
#elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#    list(FILTER soh__Enhancements EXCLUDE REGEX "soh/Enhancements/speechsynthesizer/SAPI*")
#else()
#    list(FILTER soh__Enhancements EXCLUDE REGEX "soh/Enhancements/speechsynthesizer/(Darwin|SAPI).*")
#endif()

#source_group("soh\\Enhancements" REGULAR_EXPRESSION "soh/Enhancements/*")
#source_group("soh\\Enhancements\\audio" REGULAR_EXPRESSION "soh/Enhancements/audio/*")
# source_group("2s2h\\Enhancements\\interpolation" REGULAR_EXPRESSION "2s2h/Enhancements/interpolation/*")
#source_group("soh\\Enhancements\\cosmetics" REGULAR_EXPRESSION "soh/Enhancements/cosmetics/*")
#source_group("soh\\Enhancements\\crowd-control" REGULAR_EXPRESSION "soh/Enhancements/crowd-control/*")
#source_group("soh\\Enhancements\\custom-message" REGULAR_EXPRESSION "soh/Enhancements/custom-message/*")
#source_group("soh\\Enhancements\\debugger" REGULAR_EXPRESSION "soh/Enhancements/debugger/*")
#source_group("soh\\Enhancements\\game-interactor" REGULAR_EXPRESSION "soh/Enhancements/game-interactor/*")
#source_group("soh\\Enhancements\\item-tables" REGULAR_EXPRESSION "soh/Enhancements/item-tables/*")
#source_group("soh\\Enhancements\\randomizer" REGULAR_EXPRESSION "soh/Enhancements/randomizer/*")
#source_group("soh\\Enhancements\\randomizer\\3drando" REGULAR_EXPRESSION "soh/Enhancements/randomizer/3drando/*")
#source_group("soh\\Enhancements\\randomizer\\3drando\\hint_list" REGULAR_EXPRESSION "soh/Enhancements/randomizer/3drando/hint_list/*")
#source_group("soh\\Enhancements\\randomizer\\3drando\\location_access" REGULAR_EXPRESSION "soh/Enhancements/randomizer/3drando/location_access/*")
#source_group("soh\\Enhancements\\speechsynthesizer" REGULAR_EXPRESSION "soh/Enhancements/speechsynthesizer/*")
#source_group("soh\\Enhancements\\tts" REGULAR_EXPRESSION "soh/Enhancements/tts/*")

#if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#    set_source_files_properties(soh/Enhancements/tts/tts.cpp PROPERTIES COMPILE_FLAGS "/utf-8")
#endif()
# }}}

if(NOT CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch|CafeOS")
    # 2s2h/Extractor {{{
    file(GLOB_RECURSE soh__Extractor RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        "2s2h/Extractor/*.c"
        "2s2h/Extractor/*.cpp"
        "2s2h/Extractor/*.h"
        "2s2h/Extractor/*.hpp"
    )
    # }}}
else()
    file(GLOB_RECURSE soh__Extractor RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        "2s2h/Extractor/*.h"
        "2s2h/Extractor/*.hpp"
    )
# }}}
endif()

# 2s2h/resource {{{
file(GLOB_RECURSE soh__Resource RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "2s2h/resource/*.cpp" "2s2h/resource/*.h")

source_group("2s2h\\resource\\type" REGULAR_EXPRESSION "2s2h/resource/type/*")
source_group("2s2h\\resource\\type\\scenecommand" REGULAR_EXPRESSION "2s2h/resource/type/scenecommand/*")
source_group("2s2h\\resource\\importer" REGULAR_EXPRESSION "2s2h/resource/importer/*")
source_group("2s2h\\resource\\importer\\scenecommand" REGULAR_EXPRESSION "2s2h/resource/importer/scenecommand/*")
# }}}

# src (decomp) {{{
file(GLOB_RECURSE src__ RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.c" "src/*.h")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND src__ ${CMAKE_CURRENT_SOURCE_DIR}/windows/Resource.rc)
endif()

list(FILTER src__ EXCLUDE REGEX "src/dmadata/*")
list(FILTER src__ EXCLUDE REGEX "src/elf_message/*")
list(FILTER src__ EXCLUDE REGEX "src/libultra/io/*")
list(FILTER src__ EXCLUDE REGEX "src/libultra/libc/*")
list(FILTER src__ EXCLUDE REGEX "src/libultra/os/*")
list(FILTER src__ EXCLUDE REGEX "src/libultra/rmon/*")
list(FILTER src__ EXCLUDE REGEX "src/libultra/*")
#list(APPEND src__ "src/libultra/libc/sprintf.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/cosf.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/lookat.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/lookathil.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/perspective.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/position.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/sinf.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/sinf.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/sqrtf.c")
#list(REMOVE_ITEM src__ "src/libultra/gu/us2dex.c")

source_group("src" REGULAR_EXPRESSION "src/*")
source_group("src\\boot" REGULAR_EXPRESSION "src/boot/*")
source_group("src\\buffers" REGULAR_EXPRESSION "src/buffers/*")
source_group("src\\code" REGULAR_EXPRESSION "src/code/*")
#source_group("src\\libultra" REGULAR_EXPRESSION "src/libultra/*")
source_group("src\\overlays\\actors" REGULAR_EXPRESSION "src/overlays/actors/*")
source_group("src\\overlays\\effects" REGULAR_EXPRESSION "src/overlays/effects/*")
source_group("src\\overlays\\fbdemos" REGULAR_EXPRESSION "src/overlays/fbdemos/*")
source_group("src\\overlays\\gamestates" REGULAR_EXPRESSION  "src/overlays/gamestates/*")
source_group("src\\overlays\\misc" REGULAR_EXPRESSION  "src/overlays/misc/*")
# }}}

set(ALL_FILES
    ${Header_Files}
    ${Header_Files__include}
    ${soh__}
	${libultra_headers}
    ${soh__DeveloperTools}
#    ${soh__config}
    ${soh__Enhancements}
    ${soh__BenGui}
    ${soh__SaveManager}
    ${soh__Extractor}
    ${soh__Resource}
    ${src__}
)

################################################################################
# Target
################################################################################
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_executable(${PROJECT_NAME} ${ALL_FILES})
else()
    add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
endif()

set(ROOT_NAMESPACE 2s2h)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_target_properties(${PROJECT_NAME} PROPERTIES
		VS_GLOBAL_KEYWORD "Win32Proj"
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        OUTPUT_NAME "2s2h-macos"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME "2s2h.elf"
	)
endif()
################################################################################
# MSVC runtime library
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
	string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
		$<$<CONFIG:Debug>:
			MultiThreadedDebug
		>
		$<$<CONFIG:Release>:
			MultiThreaded
		>
		$<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})
endif()
################################################################################
# Find/download Boost
################################################################################
# include(FetchContent)
# FetchContent_Declare(
#     Boost
#     URL      https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
#     URL_HASH SHA256=205666dea9f6a7cfed87c7a6dfbeb52a2c1b9de55712c9c1a87735d7181452b6
#     SOURCE_SUBDIR "null" # Set to a nonexistent directory so boost is not built (we don't need to build it)
#     DOWNLOAD_EXTRACT_TIMESTAMP false # supress timestamp warning, not needed since the url wont change
# )

# set(Boost_NO_BOOST_CMAKE false)
# set(BOOST_INCLUDEDIR ${FETCHCONTENT_BASE_DIR}/boost-src) # Location where FetchContent stores the source
# message("Searching for Boost installation")
# find_package(Boost)

# if (NOT ${Boost_FOUND})
#     message("Boost not found. Downloading now...")
#     FetchContent_MakeAvailable(Boost)
#     message("Boost downloaded to " ${FETCHCONTENT_BASE_DIR}/boost-src)
#     set(BOOST-INCLUDE ${FETCHCONTENT_BASE_DIR}/boost-src)
# else()
#     message("Boost found in " ${Boost_INCLUDE_DIRS})
#     set(BOOST-INCLUDE ${Boost_INCLUDE_DIRS})
# endif()
################################################################################
# Compile definitions
################################################################################
find_package(SDL2)
set(SDL2-INCLUDE ${SDL2_INCLUDE_DIRS})

if (BUILD_CROWD_CONTROL)
    find_package(SDL2_net)
    set(SDL2-NET-INCLUDE ${SDL_NET_INCLUDE_DIRS})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE assets
	${CMAKE_CURRENT_SOURCE_DIR}/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/PR
	${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/include
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/log
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/debug
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/menu
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/utils
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/utils/binarytools
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/resource
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/resource/type
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/resource/factory
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/audio
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/window
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/window/gui
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/config
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/public
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/public/libultra
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/public/bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/extern/tinyxml2
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/libultraship/Lib/
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/libultraship/Lib/libjpeg/include/
	${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/libultraship/Lib/spdlog/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/graphic/Fast3D/U64/PR
    ${CMAKE_CURRENT_SOURCE_DIR}/../libultraship/src/graphic
    ${CMAKE_CURRENT_SOURCE_DIR}/../ZAPDTR/ZAPD/resource/type
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/2s2h/
	${SDL2-INCLUDE}
    ${SDL2-NET-INCLUDE}
    ${BOOST-INCLUDE}
	${CMAKE_CURRENT_SOURCE_DIR}/assets/
	.
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		"$<$<CONFIG:Debug>:"
			"_DEBUG;"
			"_CRT_SECURE_NO_WARNINGS;"
			"ENABLE_DX11;"
		">"
		"$<$<CONFIG:Release>:"
			"NDEBUG"
		">"
           #"$<$<BOOL:${BUILD_CROWD_CONTROL}>:ENABLE_CROWD_CONTROL>"
		"INCLUDE_GAME_PRINTF;"
           #"ENABLE_CROWD_CONTROL;"
		"F3DEX_GBI_2"
		"UNICODE;"
		"_UNICODE"
		STORMLIB_NO_AUTO_LINK
		"_CRT_SECURE_NO_WARNINGS;"
		NOMINMAX
	)
elseif (CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		"$<$<CONFIG:Debug>:"
			"_DEBUG"
		">"
		"$<$<CONFIG:Release>:"
			"NDEBUG"
		">"
		"F3DEX_GBI_2"
        "SPDLOG_ACTIVE_LEVEL=3;"
		"SPDLOG_NO_THREAD_ID;"
        "SPDLOG_NO_TLS;"
		"STBI_NO_THREAD_LOCALS;"
	)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang|AppleClang")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		"$<$<CONFIG:Debug>:"
			"_DEBUG"
		">"
		"$<$<CONFIG:Release>:"
			"NDEBUG"
		">"
		"F3DEX_GBI_2"
		# "$<$<BOOL:${BUILD_CROWD_CONTROL}>:ENABLE_CROWD_CONTROL>"
		"SPDLOG_ACTIVE_LEVEL=0;"
		"_CONSOLE;"
		"_CRT_SECURE_NO_WARNINGS;"
		"ENABLE_OPENGL;"
		"UNICODE;"
		"_UNICODE"
        "NON_MATCHING;"
        "NON_EQUIVALENT;"
	)
endif()
################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /w;
            /Od
        >
        $<$<CONFIG:Release>:
            /Oi;
            /Gy;
            /W3
        >
		
		/wd4024 #different types for formal and actual parameter [x]. Usually seen along side 4047.
		/wd4047 #'uintptr_t' differs in levels of indirection from 'Gfx *'. Usually caused by passing a uintptr_t into a function expecting a pointer-types
		/wd4090 #'initializing': different 'const' qualifiers. Caused when initializing a non const struct member with a const char*.		
		/wd4101 # unreferenced local variable. Things like stack pads.
		/wd4133 #incompatible types. Usually caused by passing a c string into a function that takes a pointer.
		/wd4244 #conversion from [a] to [b]. Used to warn when copying to smaller type without a cast.
        /sdl-;
        /permissive-;
        /MP;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_compile_options(${PROJECT_NAME} PRIVATE  $<$<CONFIG:Debug>:/ZI;>)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO;
            /FORCE:MULTIPLE
        >
        /MANIFEST:NO;
        /DEBUG;
        /SUBSYSTEM:WINDOWS
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -Wno-return-type
            -Wno-unused-parameter
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-field-initializers
            -Wno-parentheses
            -Wno-narrowing
            -Wno-missing-braces
            $<$<COMPILE_LANGUAGE:C>:
                #-Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
            >
            $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wno-c++11-narrowing
                -Wno-deprecated-enum-enum-conversion
            >
            -pthread
        )

        target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -Wno-return-type
            -Wno-unused-parameter
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-field-initializers
            -Wno-parentheses
            -Wno-narrowing
            -Wno-missing-braces
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
            >
            $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wno-c++11-narrowing
                -Wno-deprecated-enum-enum-conversion
            >
            -pthread
        )

        target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O2

            # disable some warnings to not clutter output
            -Wno-multichar
            -Wno-return-type
            -Wno-narrowing
            -Wno-switch-outside-range
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
                -Wno-discarded-array-qualifiers
                -Wno-discarded-qualifiers
                -Wno-int-conversion
                -Wno-builtin-declaration-mismatch
                -Wno-switch-unreachable
                -Wno-stringop-overflow
            >
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O2

            -Wno-error
            -Wno-format-security
            # disable some warnings to not clutter output
            -Wno-multichar
            -Wno-return-type
            -Wno-narrowing
            -Wno-switch-outside-range
            $<$<COMPILE_LANGUAGE:C>:
                -Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
                -Wno-discarded-array-qualifiers
                -Wno-discarded-qualifiers
                -Wno-int-conversion
                -Wno-builtin-declaration-mismatch
                -Wno-switch-unreachable
                -Wno-stringop-overflow
                -Wno-implicit-int
            >
        )
    else()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
		set(CPU_OPTION -msse2 -mfpmath=sse)
        endif()

        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wno-error
            -Wno-unused-parameter
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-field-initializers
            -Wno-parentheses
            -Wno-narrowing
            -Wno-missing-braces
            -Wno-int-conversion
            $<$<COMPILE_LANGUAGE:C>:
                #-Werror-implicit-function-declaration
                -Wno-incompatible-pointer-types
            >
            $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
            -pthread
	    ${CPU_OPTION}
        )

        target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
            -Wl,-export-dynamic
        )
    endif()
endif()
################################################################################
# Pre build events
################################################################################
if(NOT CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch|CafeOS|Android")
    add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMENT "Copying asset xmls..."
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/mm/assets/extractor $<TARGET_FILE_DIR:2ship>/assets/extractor
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/mm/assets/xml $<TARGET_FILE_DIR:2ship>/assets/extractor/xmls
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/OTRExporter/CFG/filelists $<TARGET_FILE_DIR:2ship>/assets/extractor/filelists
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:2ship>/assets/extractor/symbols
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ActorList_MM.txt $<TARGET_FILE_DIR:2ship>/assets/extractor/symbols
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/OTRExporter/CFG/ObjectList_MM.txt $<TARGET_FILE_DIR:2ship>/assets/extractor/symbols
	    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/OTRExporter/CFG/SymbolMap_MM.txt $<TARGET_FILE_DIR:2ship>/assets/extractor/symbols
    )
endif()
################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    libultraship
)
if(NOT CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch|CafeOS")
add_dependencies(${PROJECT_NAME}
    ZAPDLib
)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(glfw3 REQUIRED)
	set(ADDITIONAL_LIBRARY_DEPENDENCIES
		"libultraship;"
		"ZAPDLib;"
		"glu32;"
		"SDL2::SDL2;"
		"SDL2::SDL2main;"
        #"$<$<BOOL:${BUILD_CROWD_CONTROL}>:SDL2_net::SDL2_net-static>"
		"glfw;"
		"winmm;"
		"imm32;"
		"version;"
		"setupapi"
	)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    find_package(SDL2)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libultraship;"
        SDL2::SDL2
        -lglad
        Threads::Threads
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libultraship;"
        "ZAPDLib;"
        SDL2::SDL2
        Threads::Threads
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "CafeOS")
    find_package(SDL2 REQUIRED)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libultraship;"
        SDL2::SDL2-static

        "$<$<CONFIG:Debug>:-Wl,--wrap=abort>"
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${DEVKITPRO}/portlibs/wiiu/include/
    )
else()
    find_package(SDL2)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
 	set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libultraship;"
		"ZAPDLib;"
		SDL2::SDL2
 #       "$<$<BOOL:${BUILD_CROWD_CONTROL}>:SDL2_net::SDL2_net>"
		${CMAKE_DL_LIBS}
		Threads::Threads
	)
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch|CafeOS")
INSTALL(TARGETS 2ship DESTINATION . COMPONENT 2s2h)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
INSTALL(FILES $<TARGET_PDB_FILE:2ship> DESTINATION ./debug COMPONENT 2s2h)
INSTALL(FILES ${CMAKE_BINARY_DIR}/mm/2ship.o2r DESTINATION . COMPONENT 2s2h)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/linux/2s2h.sh.in ${CMAKE_BINARY_DIR}/linux/2s2h.sh @ONLY)
endif()

find_program(CURL NAMES curl DOC "Path to the curl program.  Used to download files.")
execute_process(COMMAND ${CURL} -sSfL https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt -o ${CMAKE_BINARY_DIR}/gamecontrollerdb.txt OUTPUT_VARIABLE RESULT)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/macosx/Info.plist.in ${CMAKE_BINARY_DIR}/macosx/Info.plist @ONLY)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/macosx/2s2h-macos.sh.in ${CMAKE_BINARY_DIR}/macosx/2s2h-macos.sh @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/gamecontrollerdb.txt DESTINATION ../MacOS COMPONENT 2s2h)
INSTALL(FILES ${CMAKE_BINARY_DIR}/mm/2ship.o2r DESTINATION ../Resources COMPONENT 2s2h)
elseif(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "NintendoSwitch|CafeOS")
INSTALL(FILES ${CMAKE_BINARY_DIR}/gamecontrollerdb.txt DESTINATION . COMPONENT 2s2h)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch|CafeOS")
    if (NOT TARGET pathconf)
        add_library(pathconf OBJECT platform/pathconf.c)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}" $<TARGET_OBJECTS:pathconf> )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "NintendoSwitch")

nx_generate_nacp(2s2h.nacp
   NAME "2s2h of Harkinian"
   AUTHOR "${PROJECT_TEAM}"
   VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)

nx_create_nro(2s2h
    NACP 2s2h.nacp
    ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/2s2h.nro DESTINATION . COMPONENT 2s2h)

elseif(CMAKE_SYSTEM_NAME MATCHES "CafeOS")

wut_create_rpx(${PROJECT_NAME})

wut_create_wuhb(${PROJECT_NAME}
	NAME       "Two Ship Two Harkinian"
	SHORTNAME  "2S2H"
	AUTHOR     "${PROJECT_TEAM}"
	ICON       ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/2s2h.rpx ${CMAKE_CURRENT_BINARY_DIR}/2s2h.wuhb DESTINATION . COMPONENT 2s2h)

endif()
