cmake_minimum_required(VERSION 3.10)
project(tbx_draw)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE SOURCES "src/*.cpp")
# Add the main executable
add_executable(${PROJECT_NAME} ${SOURCES})
        

add_custom_target(copy_resources ALL
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${PROJECT_SOURCE_DIR}/assets
      ${PROJECT_BINARY_DIR}/assets
      COMMENT "Copying resources into binary directory")


set(GENERATED_META_PROGRAM_HEADER "${CMAKE_SOURCE_DIR}/src/meta_program/meta_program.hpp")

if(EXISTS "${GENERATED_META_PROGRAM_HEADER}" AND NOT IS_DIRECTORY "${GENERATED_META_PROGRAM_HEADER}")
    message(STATUS "Found generated meta_program.hpp, enabling GENERATED_META_PROGRAM")
    target_compile_definitions(${PROJECT_NAME} PRIVATE GENERATED_META_PROGRAM)
endif()

add_dependencies(${PROJECT_NAME} copy_resources)

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install

# Automatically generated from conan install
find_package(glfw3)
find_package(glad)
find_package(fmt)
find_package(OpenAL)
find_package(nlohmann_json)
find_package(SndFile)
find_package(glm)
target_link_libraries(${PROJECT_NAME} glfw glad::glad fmt::fmt OpenAL::OpenAL nlohmann_json::nlohmann_json SndFile::sndfile glm::glm)
