
macro_subdir_list(subdirs ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${subdirs})
  add_subdirectory(${subdir})
endforeach()
