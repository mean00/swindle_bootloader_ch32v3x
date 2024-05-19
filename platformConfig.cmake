IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_SUFFIX ".exe")
ENDIF(WIN32)

  
#SET(PLATFORM_PREFIX riscv-none-embed-) # MRS toolchain
IF(WIN32)    
  SET(PLATFORM_TOOLCHAIN_PATH todo_todo) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
ELSE(WIN32)      
  #--- GCC ---------
  SET(PLATFORM_TOOLCHAIN_PATH "/riscv/tools2/bin" CACHE INTERNAL "")    
  IF(USE_HW_FPU)
    SET(PLATFORM_C_FLAGS "-march=rv32imafc -mabi=ilp32f " CACHE INTERNAL "")
  ELSE()
    SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 " CACHE INTERNAL "")
  ENDIF()
  SET(PLATFORM_PREFIX "riscv64-unknown-elf-" CACHE INTERNAL "") 
  # FOR CLANG  
  
  SET(PLATFORM_TOOLCHAIN_TRIPLET "riscv64-unknown-elf-" CACHE INTERNAL "")      
  #-- CLANG --
  # No FPU
  SET(PLATFORM_CLANG_PATH  "/riscv/tools_llvm/bin" CACHE INTERNAL "")
  SET(PLATFORM_CLANG_VERSION "-17")
  IF(USE_HW_FPU)
      SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_hard_fp/" CACHE INTERNAL "")
      SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imafc -mabi=ilp32f  " CACHE INTERNAL "")      
      SET(PLATFORM_CLANG_EXTRA_LD_ARG "-L${PLATFORM_CLANG_SYSROOT}/lib/rv32imafc/ilp32f" CACHE INTERNAL "")
  ELSE()
      SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_soft_nofp/" CACHE INTERNAL "")
      SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imac -mabi=ilp32  " CACHE INTERNAL "")      
  ENDIF()      
ENDIF(WIN32)
 
