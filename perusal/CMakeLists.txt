# cmake for solo3 src subdirs

project(solo3)

# compile definitions

add_definitions(-std=c++11)

# includes

include_directories(.)
include_directories(../include)

include_directories(${GTK_INCLUDE_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${GDKMM_INCLUDE_DIRS})
include_directories(${GLIBMM_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${CAIRO_INCLUDE_DIRS})
include_directories(${PANGO_INCLUDE_DIRS})

# Link

#link_directories(${LROSE_PREFIX}/lib)
#link_directories(/usr/lib/x86_64-linux-gnu/hdf5/serial)
#link_directories(/usr/local/lib)

# location of bin as it is built

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/release/bin)

# build app

add_executable (
  ${PROJECT_NAME}
  CRectangle.cc
  Circle.cc
  ClickQueue.cc
  ColorManager.cc
  ColorNamesWindow.cc
  ColorPalettesWindow.cc
  ColorTable.cc
  ColorTableManager.cc
  ColorTablesWindow.cc
  DataWindow.cc
  EditCmdHelpWindow.cc
  EditCmdListHelpWindow.cc
  EditFilesWindow.cc
  EditWindow.cc
  ExamineWindow.cc
  ExamineDisplayWindow.cc
  Graphic.cc
  ImageGenWindow.cc
  ImportColorTableWindow.cc
  Label.cc
  Line.cc
  MainConfigWindow.cc
  PaletteManager.cc
  ParameterWindow.cc
  RGBImage.cc
  SiiPalette.cc
  SoloMainWindow.cc
  SoloState.cc
  SoloSubFrame.cc
  sii_bscan.cc
  sii_callbacks.cc
  sii_colors_stuff.cc
  sii_config_stuff.cc
  sii_exam_widgets.cc
  sii_frame_menu.cc
  sii_links_widget.cc
  sii_param_widgets.cc
  sii_perusal.cc
  sii_png_image.cc
  sii_utils.cc
  sii_xyraster.cc
  solo2.cc
  soloii.cc
  sp_accepts.cc
  sp_basics.cc
  sp_clkd.cc
  sp_dorade.cc
  sp_lists.cc
  sp_save_frms.cc
  SweepfileWindow.cc
  SweepfilesListWindow.cc
  ViewWindow.cc
  )

# link

target_link_libraries(${PROJECT_NAME} solo_edit})
target_link_libraries(${PROJECT_NAME} solo_s3)

target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GDKMM_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GLIBMM_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GLIB_LIBRARIES})


# install

set(INSTALL_PREFIX $ENV{LROSE_INSTALL_DIR})

if(NOT INSTALL_PREFIX)
  set(INSTALL_PREFIX ${LROSE_PREFIX})
endif(NOT INSTALL_PREFIX)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_PREFIX}/bin)

