# CMAKE configuration for COEVO-lution1 helpers applications  (from 20th cent.!)
################################################################################
# @author Wojciech Borkowski
# @date: 2026 (last modifications)
#
cmake_minimum_required(VERSION 3.30)

project(coevolution_helpers)
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
    include_directories( ${SYMSHELL} )
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
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( mapa_alt
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( mapa_not
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( mapa_patt
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( meteory
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( paleta
         wbsyshLight X11 Xpm
         rt
         )

    target_link_libraries( realpatt
         wbsyshLight X11 Xpm
         rt
         )
endif(X11)

if(WINDOWS) # only for WINDOWS - NOT TESTED IN THIS VERSION (after 2016)
    target_link_libraries( and_pattern
         wbsyshLight
         )

    target_link_libraries( mapa_alt
         wbsyshLight
         )

    target_link_libraries( mapa_not
         wbsyshLight
         )

    target_link_libraries( mapa_patt
         wbsyshLight
         )

    target_link_libraries( meteory
         wbsyshLight
         )

    target_link_libraries( paleta
         wbsyshLight
         )

    target_link_libraries( realpatt
         wbsyshLight
         )
endif(WINDOWS)





