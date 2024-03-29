# CMakeLists for AparData library

cmake_minimum_required (VERSION 2.8)
project (solo3)

include_directories(.)
include_directories(../include)
include_directories(../perusal)
include_directories(../editor)
include_directories(../translate)

include_directories(${GTK_INCLUDE_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${GDKMM_INCLUDE_DIRS})
include_directories(${GLIBMM_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${CAIRO_INCLUDE_DIRS})
include_directories(${PANGO_INCLUDE_DIRS})
include_directories(/opt/X11/include)

add_library(solo_edit
  ASpeckleCmd.cc
  AbsoluteValueCmd.cc
  AddFieldCmd.cc
  AddValueCmd.cc
  AndBadFlagsCmd.cc
  AppendHistogramToFileCmd.cc
  AreaHistogramCmd.cc
  AssertBadFlagsCmd.cc
  AssignValueCmd.cc
  BBGatesAveragedCmd.cc
  BBMaxNegFoldsCmd.cc
  BBMaxPosFoldsCmd.cc
  BBUnfoldingCmd.cc
  BBUseAcWindCmd.cc
  BBUseFirstGoodGateCmd.cc
  BBUseLocalWindCmd.cc
  ClearBadFlagsCmd.cc
  ComplementBadFlagsCmd.cc
  CopyBadFlagsCmd.cc
  CopyCmd.cc
  CountHistogramCmd.cc
  DataInfo.cc
  DataManager.cc
  DateTime.cc
  DeglitchMinGatesCmd.cc
  DeglitchRadiusCmd.cc
  DeglitchThresholdCmd.cc
  DespeckleCmd.cc
  DoHistogramCmd.cc
  DontAppendHistogramToFileCmd.cc
  DontUseBoundaryCmd.cc
  DuplicateCmd.cc
  EstablishAndResetCmd.cc
  EarthRadiusCalculator.cc
  EWWindCmd.cc
  ExponentiateCmd.cc
  FileInfo.cc
  FirstGoodGateCmd.cc
  FirstSweepCmd.cc
  FixVortexVelocitiesCmd.cc
  FlagFrecklesCmd.cc
  FlagGlitchesCmd.cc
  FlaggedAddCmd.cc
  FlaggedAssignCmd.cc
  FlaggedCopyCmd.cc
  FlaggedMultiplyCmd.cc
  ForEachRayCmd.cc
  ForcedUnfoldingCmd.cc
  FreckleAverageCmd.cc
  FreckleThresholdCmd.cc
  GatesShiftedCmd.cc
  HeaderValueCmd.cc
  HistogramCommentCmd.cc
  HistogramDirectoryCmd.cc
  HistogramFlushCmd.cc
  HistogramMgr.cc
  IgnoreFieldCmd.cc
  IrregularHistogramBinCmd.cc
  LastSweepCmd.cc
  MapBoundaryCmd.cc
  MergeFieldCmd.cc
  MinBadCountCmd.cc
  MinNotchShearCmd.cc
  MultFieldsCmd.cc
  MultiplyCmd.cc
  NewHistogramFileCmd.cc
  NewVersionCmd.cc
  NoNewVersionCmd.cc
  NotchMaxCmd.cc
  NSWindCmd.cc
  NyquistVelocityCmd.cc
  OffsetForRadialShearCmd.cc
  OmitSourceFileInfoCmd.cc
  OneTimeOnlyCmd.cc
  OptimalBeamwidthCmd.cc
  OrBadFlagsCmd.cc
  PointInSpace.cc
  RadarNamesCmd.cc
  RadialShearCmd.cc
  RainRateCmd.cc
  RegularHistogramParametersCmd.cc
  RemoveAircraftMotionCmd.cc
  RemoveOnlySecondTripSurfaceCmd.cc
  RemoveOnlySurfaceCmd.cc
  RemoveRingCmd.cc
  RemoveStormMotionCmd.cc
  RemoveSurfaceCmd.cc
  RescaleFieldCmd.cc
  RewriteCmd.cc
  SaveCommandsCmd.cc
  SeBoundaryList.cc
  SelectSiteListCmd.cc
  SetBadFlagsCmd.cc
  ShiftFieldCmd.cc
  ShowSiteValuesCmd.cc
  SiteListCmd.cc
  SubtractCmd.cc
  SurfaceGateShiftCmd.cc
  SweepDirectoryCmd.cc
  ThresholdCmd.cc
  UiCommand.cc
  UiCommandFactory.cc
  UiCommandToken.cc
  UnconditionalDeleteCmd.cc
  UseBoundaryCmd.cc
  VersionCmd.cc
  VertWindCmd.cc
  XorBadFlagsCmd.cc
  XYDirectoryCmd.cc
  XYListingCmd.cc
  running_avg_que.cc
  se_BB_unfold.cc
  se_add_mult.cc
  se_bnd.cc
  se_catch_all.cc
  se_defrkl.cc
  se_flag_ops.cc
  se_for_each.cc
  se_histog.cc
  se_proc_data.cc
  se_utils.cc
  se_wgt_stf.cc
  solo_list_mgmt.cc
  sxm_examine.cc
  )

target_include_directories (solo_edit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    )

#install(TARGETS solo_edit EXPORT MyLibraryConfig
#    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    )

#install(DIRECTORY ../include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Make sure those using this library also get our LROSE dependencies
#target_link_libraries(${PROJECT_NAME} PUBLIC ${LROSE_LINK_FLAGS} ${LROSE_LIBRARIES})
#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
