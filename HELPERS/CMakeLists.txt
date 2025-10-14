# CMAKE configuration for COEVO-lution1 helpers applications  (from 20th cent.!)
################################################################################
# @author: Wojciech Borkowski
# @date: 2024-02-09 (last modifications)
#
project(coevolution_helpers)
cmake_minimum_required(VERSION 2.9)
set(CMAKE_CXX_STANDARD 11) # See: https://matgomes.com/setting-cpp-standard-cmake/

IF (WIN32)
  # set stuff for windows
  message( ${PROJECT_NAME} ": MS Windows platform assumed.\n\tInside C/C++ code automatic _MSC_VER variable will be used!" )
  set( WINDOWS true )
  #set( MSVC 1 ) #old macro used also somewhere in C code?
  #set( WINDOWS_ONLY_SRC  "${WBRTM}/BASE/wb_assert.c" )
  #add_compile_options( /W4 ) #  /WX
ELSE()
  # set stuff for other systems
  message( ${PROJECT_NAME} ": Linux/unix platform with X11 assumed.")
  set( X11 true )
  #set( X_ONLY_SRC  "" )
  #add_compile_options( -Wall )
ENDIF()

if(!SYMSHELL)
    set(SYMSHELL "../../SymShellLight")
    add_subdirectory( ${SYMSHELL} ${SYMSHELL} )
    include_directories( ${SYMSHELL} )
else()
    include_directories( "../../SymShellLight" )
endif()

add_executable( and_pattern
                AND_PATT.cpp
                )

add_executable( mapa_alt
                MAPAALT.cpp
                )

add_executable( mapa_not
                MAPANOT.cpp
                )

add_executable( mapa_patt
                MAPAPATT.cpp
                )

add_executable( meteory
                METEORY.cpp
                )

add_executable( paleta
                PALETA.cpp
                )

add_executable( realpatt
                REALPATT.cpp
                )

if(X11) # only for X11 LIB
    add_definitions( -DHAVE_X11 -DUSE_X11 )
    target_link_libraries( and_pattern 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( mapa_alt 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( mapa_not 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( mapa_patt 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( meteory 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( paleta 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )

    target_link_libraries( realpatt 
         "-L${SYMSHELL}"
         wbsyshX11 X11 Xpm
         rt
         )
endif(X11)

if(WINDOWS) # only for WINDOWS - NOT TESTED IN THIS VERSION (after 2016)
    target_link_libraries( and_pattern 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( mapa_alt 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( mapa_not 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( mapa_patt 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( meteory 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( paleta 
         "-L${SYMSHELL}"
         wbsyshLight
         )

    target_link_libraries( realpatt 
         "-L${SYMSHELL}"
         wbsyshLight
         )
endif(WINDOWS)





