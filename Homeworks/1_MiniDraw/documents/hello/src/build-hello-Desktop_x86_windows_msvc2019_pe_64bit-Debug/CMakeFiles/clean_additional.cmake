# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\hello_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\hello_autogen.dir\\ParseCache.txt"
  "hello_autogen"
  )
endif()
