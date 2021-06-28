# CMakeLists for AparData library

cmake_minimum_required (VERSION 2.8)
project (solo3)

add_library(solo_s3
  dd_files.cc
  dda_common.cc
  ddb_common.cc
  dorade_share.cc
  dd_time.cc
  swp_file_acc.cc
  dd_io_mgmt.cc
  ddin.cc
  ddout.cc
  dd_uf.cc
  dd_ncdf.cc
  gneric_dd.cc
  hrd_dd.cc
  sigm_dd.cc
  uf_dd.cc
  nex_dd.cc
  toga_dd.cc
  elda_dd.cc
  eldb_dd.cc
  gpro_data.cc
  ground_echo.cc
  stdhrd.cc
  xwsrqc.cc
  fof_dd.cc
  piraq_dd.cc
  tdwr_dd.cc
  etl_dd.cc
  cimm_dd.cc
  nc_dd.cc
  piraqx_dd.cc
  LittleEndian.cc
  by_products.cc
  dd_catalog.cc
  gecho.cc
  dorade_uf.cc
  product_x.cc
  dd_der_flds.cc
  nssl_mrd.cc
  dd_swp_files.cc
  dorade_tape.cc
  shane.cc
  dd_crackers.cc
  dorade_ncdf.cc
  dd_swpfi.cc
  )

add_library(solo_dap
  dap_common.cc
  qdap.cc
  )

add_executable (xltrs3
  ddmain.cc
  )
target_link_libraries(xltrs3 solo_s3 solo_dap)

add_executable (ddex3
  dd_examine.cc
  )
target_link_libraries(ddex3 solo_s3 solo_dap)

add_executable (nx_reblock3
  nx_reblock.cc
  )
target_link_libraries(nx_reblock3 solo_s3 solo_dap)

target_include_directories (${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    )

install(TARGETS solo_s3 EXPORT MyLibraryConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

install(TARGETS solo_dap EXPORT MyLibraryConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

install(DIRECTORY ../include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

