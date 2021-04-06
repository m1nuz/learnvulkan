cmake_minimum_required(VERSION 3.0.0)
project(learnvulkan VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CTest)
enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(FMT REQUIRED)
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

set(TARGET_NAME "learnvulkan")
set(APP_NAME ${TARGET_NAME})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

add_executable(${TARGET_NAME} src/main.cpp)

target_compile_features(${TARGET_NAME}
    PUBLIC
        cxx_std_20
)

target_include_directories(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        fmt::fmt
        stdc++
        stdc++fs
        Threads::Threads
        Vulkan::Vulkan
        ${GLFW_STATIC_LIBRARIES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
