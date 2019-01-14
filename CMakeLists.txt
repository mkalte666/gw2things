# CMakeLists.txt
#
# Copyright (C) 2018  Malte Kießling
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or any later version.

# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.

# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

# some cmake default foo
cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 14)

# disable in-source build. found at
# https://stackoverflow.com/questions/1208681/with-cmake-how-would-you-disable-
# in-source-builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(gw2things)
set(_target_name gw2things)

# some more debug/release defines because im paranoid
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG -DDEBUGGING -DNRELEASE)
else()
  add_definitions(-DNDEBUG -DNDEBUGGING -DRELEASE)
endif()

# put stuff into "bin" and "lib" because we can sauce:
# https://stackoverflow.com/questions/7747857/in-cmake-how-do-i-work-around-the-
# debug-and-release-directories-visual-studio-2
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/buildpackage/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/buildpackage/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/buildpackage/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
      ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
      ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)


# this is for the following find package. uncomment what you need.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(CURL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(SDL2_IMAGE REQUIRED) 
# find_package(SDL2_MIXER REQUIRED)

# includes. glm will stay, but should not cause problems. i just use it alot.
# uncomment what you need
include_directories(${GLM_INCLUDE_DIR} ${SDL2_INCLUDE_DIR}
                    ${SDL2_TTF_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    # ${SDL2_MIXER_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/3rdparty/gl3w/include/
                    ${PROJECT_SOURCE_DIR}/3rdparty/imgui
                    ${PROJECT_SOURCE_DIR}/3rdparty/json/single_include/
                    ${CURL_INCLUDE_DIR})

# dear imgui
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)
set(IMGUI_SOURCES
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/imgui.h
    3rdparty/imgui/imgui_demo.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_internal.h
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/imstb_rectpack.h
    3rdparty/imgui/imstb_textedit.h
    3rdparty/imgui/imstb_truetype.h
    3rdparty/imgui/misc/cpp/imgui_stdlib.cpp
    3rdparty/imgui/misc/cpp/imgui_stdlib.h
    3rdparty/imgui/examples/imgui_impl_sdl.cpp
    3rdparty/imgui/examples/imgui_impl_sdl.h
    3rdparty/imgui/examples/imgui_impl_opengl3.cpp
    3rdparty/imgui/examples/imgui_impl_opengl3.h)

add_library(imgui ${IMGUI_SOURCES})

set(API_SOURCES 
    src/api/accountcharacters.cpp
    src/api/accountcharacters.h
    src/api/accountinventory.cpp
    src/api/accountinventory.h
    src/api/accountoverview.cpp
    src/api/accountoverview.h
    src/api/bank.cpp
    src/api/bank.h
    src/api/character.cpp
    src/api/character.h
    src/api/continent.cpp
    src/api/continent.h
    src/api/endpoint.cpp
    src/api/endpoint.h
    src/api/guildpublic.cpp
    src/api/guildpublic.h
    src/api/inventory.cpp
    src/api/inventory.h
    src/api/item.cpp
    src/api/item.h
    src/api/itemcache.cpp
    src/api/itemcache.h
    src/api/map.cpp
    src/api/map.h
    src/api/materialstorage.cpp
    src/api/materialstorage.h
    src/api/price.cpp
    src/api/price.h)

add_library(gw2api ${API_SOURCES})    
    
# sources go here
set(ALL_SOURCES 
    ${PROJECT_SOURCE_DIR}/3rdparty/gl3w/src/gl3w.c 
    src/env.cpp
    src/env.h
    src/main.cpp
    src/fetcher.cpp
    src/fetcher.h
    src/icon.cpp
    src/icon.h
    src/mapview.cpp
    src/mapview.h
    src/mumblefile.cpp
    src/mumblefile.h
    src/overlay.cpp
    src/overlay.h
    src/tracker.cpp
    src/tracker.h
    src/window.cpp
    src/window.h
    src/workpool.cpp
    src/workpool.h)

# more the usual. create executable, add libraries
add_executable(${_target_name} ${ALL_SOURCES})

target_link_libraries(${_target_name}
                      imgui
                      gw2api
                      ${SDL2_LIBRARY}
                      ${SDL2MAIN_LIBRARY}
                      ${SDL2_TTF_LIBRARY}
                      ${CURL_LIBRARY}
                      ${SDL2_IMAGE_LIBRARY}
                      # ${SDL2_MIXER_LIBRARY}
                      )

if (UNIX)
	target_link_libraries(${_target_name} dl rt)
endif (UNIX)

# now the tricky part: we need both the sdl build stuff *and* the whole project
# tree however there are two problems: first, we dont want to copy everything
# every time second, for the sdl2 libs, we need to know if we are 32 or 64 bit.

# first, the copy all bullshit i came to like
add_dependencies(${_target_name} ${_target_name}_datacopy)
add_custom_target(
  ${_target_name}_datacopy
  COMMAND ${CMAKE_COMMAND} -Dsrc_dir="${PROJECT_SOURCE_DIR}"
          -Ddst_dir="${CMAKE_BINARY_DIR}/buildpackage/" -P
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/update_directory.cmake")

# is there a better way? i dont know. found at http://www.saoe.net/blog/778/
if(${CMAKE_SIZEOF_VOID_P} EQUAL "8")
  set(PROJECT_ARCHITECTURE "x64")
else()
  set(PROJECT_ARCHITECTURE "x86")
endif()

if(WIN32)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} /MP")
        SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP")
    endif()

      # copy sdl2 stuff. uncomment what you need
      add_custom_command(TARGET ${_target_name} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy_directory
                                 "${SDL2_PATH}/lib/${PROJECT_ARCHITECTURE}/"
                                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
      add_custom_command(TARGET ${_target_name} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy_directory
                                 "${SDL2_TTF_PATH}/lib/${PROJECT_ARCHITECTURE}/"
                                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
      add_custom_command(TARGET ${_target_name} POST_BUILD 
                         COMMAND ${CMAKE_COMMAND} -E copy_directory
                                 "${SDL2_IMAGE_PATH}/lib/${PROJECT_ARCHITECTURE}/"
                                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) 

    #add_custom_command(TARGET ${_target_name}
      # POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
      # "${SDL2_MIXER_PATH}/lib/${PROJECT_ARCHITECTURE}/"
      # ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif(WIN32)
