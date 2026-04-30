# CMAKE configuration for COEVO-lution1 simulation project (from 20th cent.!)
#############################################################################
# @author: Wojciech Borkowski
# @date: 2026-04-30 (last modifications)
#
cmake_minimum_required(VERSION 3.30)

project(coevolution1)
set(CMAKE_CXX_STANDARD 11) # See: https://matgomes.com/setting-cpp-standard-cmake/
message(${PROJECT_NAME} ": top directory is ${coevolution1_SOURCE_DIR}.")

# Define WITH_TOOLS if you want build helpers applications!
set(WITH_TOOLS true)

if(WIN32)
  # set stuff for windows
  message( ${PROJECT_NAME} ": MS Windows platform assumed.\n\tInside C/C++ code automatic _MSC_VER variable will be used!" )
  set( WINDOWS true )
  #set( MSVC 1 ) #old macro used also somewhere in C code?
  #set( WINDOWS_ONLY_SRC  "${WBRTM}/BASE/wb_assert.c" )
  #add_compile_options( /W4 ) #  /WX
else()
  # set stuff for other systems
  message( ${PROJECT_NAME} ": Linux/unix platform with X11 assumed.")
  set( X11 true )
  #set( X_ONLY_SRC  "" )
  #add_compile_options( -Wall )
endif()

# SymShellLight library is required for compilation.
if(SYMSHELLLIGHT_ALREADY_CONFIGURED)
        message(WARNING ${PROJECT_NAME} ": SymShell used from top level project ( ${SYMSHELL}).")
else()
        if(EXISTS "${coevolution1_SOURCE_DIR}/../SymShellLight")
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../SymShellLight" ABSOLUTE)
        elseif(EXISTS "${coevolution1_SOURCE_DIR}/../symShellLight")
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../symShellLight" ABSOLUTE)  
        elseif(EXISTS "${coevolution1_SOURCE_DIR}/../../public/SymShellLight")
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../../public/SymShellLight" ABSOLUTE)    
        elseif(EXISTS "${coevolution1_SOURCE_DIR}/../../public/symShellLight")
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../../public/symShellLight" ABSOLUTE)
        elseif(EXISTS "${coevolution1_SOURCE_DIR}/../../../public/SymShellLight")
             #set(SYMSHELL "${coevolution1_SOURCE_DIR}/../../public/SymShellLight")    
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../../public/SymShellLight" ABSOLUTE)
        elseif(EXISTS "${coevolution1_SOURCE_DIR}/../../../public/symShellLight")
             get_filename_component(SYMSHELL "${coevolution1_SOURCE_DIR}/../../public/symShellLight" ABSOLUTE)               
        else()
             message(FATAL_ERROR  ${PROJECT_NAME} ": SymShell sources not found!")
        endif()

        message(${PROJECT_NAME} ": SymShell used from ${SYMSHELL}.")
        add_subdirectory( ${SYMSHELL} ${SYMSHELL} )
        include_directories( ${SYMSHELL}/INCL/ )
endif()
        
if(WITH_TOOLS)
   add_subdirectory( HELPERS/ )
endif()

add_executable( coevo1
                coevo.cpp
                )

if(X11) # only for X11 LIB
    add_definitions( -DHAVE_X11 -DUSE_X11 )
    target_link_libraries( coevo1
            wbsyshLight
            X11 Xpm
            rt
         )
endif(X11)

if(WINDOWS) # only for WINDOWS - NOT TESTED IN THIS VERSION (after 2016)
    target_link_libraries( coevo1
         wbsyshLight
         )
endif(WINDOWS)


