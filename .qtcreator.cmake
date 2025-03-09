# Qt Creator specific CMake configuration
# This file helps Qt Creator handle the project correctly

# Set these variables to help Qt Creator locate the compiler
set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP TRUE)
set(QT_CREATOR_SKIP_COMPILER_CHECK TRUE)

# Set these to match your Qt installation - adjust the path as needed
set(QT_QMAKE_EXECUTABLE "C:/Qt/6.5.3/msvc2019_64/bin/qmake.exe" CACHE FILEPATH "Path to qmake")

# Add the MinGW option if you prefer that compiler
option(USE_MINGW "Use MinGW compiler instead of MSVC" OFF)

if(USE_MINGW)
  # MinGW compiler settings
  set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1120_64/bin/gcc.exe" CACHE FILEPATH "C compiler")
  set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1120_64/bin/g++.exe" CACHE FILEPATH "C++ compiler")
else()
  # Try to detect MSVC automatically
  find_program(CL_EXE cl PATHS 
    "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC" 
    PATH_SUFFIXES "*/bin/Hostx64/x64"
    NO_DEFAULT_PATH
  )
  
  if(CL_EXE)
    set(CMAKE_C_COMPILER "${CL_EXE}" CACHE FILEPATH "C compiler")
    set(CMAKE_CXX_COMPILER "${CL_EXE}" CACHE FILEPATH "C++ compiler")
  endif()
endif()