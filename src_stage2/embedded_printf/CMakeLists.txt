set(EPR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${EPR})


add_library(embeddedPrintf OBJECT printf.c)
target_compile_options(embeddedPrintf PRIVATE
              -DPRINTF_DISABLE_SUPPORT_DOUBLE -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
              -DPRINTF_DISABLE_SUPPORT_LONG_LONG -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T
              -DPRINTF_DISABLE_SUPPORT_FLOAT)

target_include_directories(embeddedPrintf PUBLIC ${EPR})
