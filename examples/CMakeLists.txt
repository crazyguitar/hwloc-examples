set(target hwloc-example)
set(ENV{PKG_CONFIG_PATH} "/opt/amazon/efa/lib/pkgconfig")
file(GLOB src *.cc)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HWLOC REQUIRED hwloc)
add_executable(${target} ${src})
target_compile_options(${target} PRIVATE -Wall -Werror -O3)
target_include_directories(${target} PRIVATE
  "${HWLOC_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(${target} PRIVATE
  ${HWLOC_LIBRARIES}
)
