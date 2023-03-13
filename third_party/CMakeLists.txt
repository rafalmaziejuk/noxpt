# NOX
set(NOX_BUILD_EXAMPLE OFF CACHE INTERNAL "")
add_subdirectory(nox)
set_target_properties(
    nox
    PROPERTIES FOLDER nox
)

# OpenCL
find_package(OpenCL REQUIRED)

target_link_libraries(noxpt PRIVATE
    nox
    OpenCL::OpenCL
)
