CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( AniMOL )

SET( EXECUTABLE_OUTPUT_PATH "${AniMOL_BINARY_DIR}/bin" )
SET( LIBRARY_OUTPUT_PATH "${AniMOL_BINARY_DIR}/lib" )

OPTION( CMAKE_VERBOSE_MAKEFILE  "Enable/Disable verbose compiler output" ON )
OPTION( CMAKE_COLOR_MAKEFILE "Enable/Disable color cues when building" ON )
MARK_AS_ADVANCED( CLEAR CMAKE_VERBOSE_MAKEFILE CMAKE_COLOR_MAKEFILE )


#######################################
# Include VTK
#######################################

FIND_PACKAGE( VTK )
IF( VTK_FOUND )
  INCLUDE( ${VTK_USE_FILE} )
ELSE( VTK_FOUND )
  MESSAGE( FATAL_ERROR "Cannot build without VTK.  Please set VTK_DIR." )
ENDIF( VTK_FOUND )

INCLUDE_DIRECTORIES ( ${VTK_INCLUDE_DIRS} )
LINK_DIRECTORIES ( ${VTK_LIBRARY_DIRS} )

SET( VTK_LIBS vtkCommon
              vtkGraphics
              vtkHybrid
              vtkIO
              vtkRendering )   


#######################################
# Include wxWidgets
#######################################

INCLUDE( LocalUsewxWidgets.cmake )
INCLUDE( ${CMAKE_ROOT}/Modules/UsewxWidgets.cmake )
INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDES} )
LINK_DIRECTORIES( ${wxWidgets_LIBRARY_DIRS} )


#######################################
# Include wxVTK
#######################################

FIND_PATH( wxVTK_SRC_DIR wxVTKRenderWindowInteractor.h )
INCLUDE_DIRECTORIES( ${wxVTK_SRC_DIR} )
SET( wxVTK_SRC ${wxVTK_SRC_DIR}/wxVTKRenderWindowInteractor.h ${wxVTK_SRC_DIR}/wxVTKRenderWindowInteractor.cxx )                 


#######################################
# Include vtkMultiChannel
#######################################

FIND_PATH( vtkMultiChannel_SRC_DIR vtkMultiChannelRenderWindowManager.h )
FIND_PATH( vtkMultiChannel_BIN_DIR vtkMultiChannel.sln )

INCLUDE_DIRECTORIES ( ${vtkMultiChannel_SRC_DIR} ${vtkMultiChannel_BIN_DIR} )


#######################################
# Include Haggis
#######################################

FIND_PATH( HAGGIS_SRC_DIR Renci/RenciFrame.h )
FIND_PATH( HAGGIS_BIN_DIR Haggis.sln )

INCLUDE_DIRECTORIES( ${HAGGIS_SRC_DIR}/RenciWxWidgets )
LINK_DIRECTORIES( ${HAGGIS_BIN_DIR}/RenciWxWidgets )

SET( HAGGIS_LIBS RenciWxWidgets.lib)


#######################################
# Include AniMOL code
#######################################

SET( SRC AniMOL.h AniMOL.cpp 
         KeyPressCallback.h KeyPressCallback.cpp
         RenderPipeline.h RenderPipeline.cpp
         vtkPDBTimeSeriesReader.h vtkPDBTimeSeriesReader.cxx )
ADD_EXECUTABLE( AniMOL WIN32 MACOSX_BUNDLE ${SRC} ${wxVTK_SRC} )
TARGET_LINK_LIBRARIES( AniMOL ${VTK_LIBS} ${HAGGIS_LIBS} 
                 debug ${vtkMultiChannel_BIN_DIR}/lib/debug/vtkMultiChannel.lib
             optimized ${vtkMultiChannel_BIN_DIR}/lib/release/vtkMultiChannel.lib )   