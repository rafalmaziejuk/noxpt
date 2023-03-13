cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR
            "In-source builds are not permitted!\n"
            "Please use build_nox_visual.bat script to build solution!\n"
    )
endif()

include(cmake/macros.cmake)

project(noxpt LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(noxpt "")
target_include_directories(noxpt
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(src)
add_subdirectory(third_party)
create_project_source_tree(noxpt)

add_custom_command(
    TARGET noxpt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:noxpt>/assets
)

if (MSVC)
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT noxpt)
endif()
