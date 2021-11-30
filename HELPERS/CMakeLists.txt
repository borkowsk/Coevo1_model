# CMAKE configuration for COEVO-lution1 helpers applications
######################################################################
# @Author: Wojciech Borkowski
#
cmake_minimum_required(VERSION 2.9)

project(coevolution_helpers)

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

target_link_libraries( and_pattern 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )

add_executable( mapa_alt
                MAPAALT.cpp
                )

target_link_libraries( mapa_alt 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )

add_executable( mapa_not
                MAPANOT.cpp
                )

target_link_libraries( mapa_not 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )

add_executable( mapa_patt
                MAPAPATT.cpp
                )

target_link_libraries( mapa_patt 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )


add_executable( meteory
                METEORY.cpp
                )

target_link_libraries( meteory 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )

add_executable( paleta
                PALETA.cpp
                )

target_link_libraries( paleta 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )


add_executable( realpatt
                REALPATT.cpp
                )

target_link_libraries( realpatt 
     "-L${SYMSHELL}"
     wbsyshX11 X11 Xpm
     rt
     )








