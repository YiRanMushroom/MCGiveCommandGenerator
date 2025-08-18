cmake_minimum_required(VERSION 4.0)
project(MCGiveCommandGenerator)
set(PROJECT_NAME MCGiveCommandGenerator)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE MODULE_FILES src/*.ixx src/*.cppm)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        BASE_DIRS src
        FILES ${MODULE_FILES})

add_subdirectory(vendor/EasyGui)
target_link_libraries(${PROJECT_NAME} PRIVATE EasyGui)