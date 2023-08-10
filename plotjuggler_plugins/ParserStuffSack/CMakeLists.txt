add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  yaml-cpp
  URL https://github.com/jbeder/yaml-cpp/archive/35b4498026b6293bfadc75f9ee325cb16d6975af.zip
  URL_HASH SHA256=03d6bd1e7b99eee4f837f31f539e9051f07e0192171fda6bd23571c51d927e41
)

FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Populate(
  stuff_sack
  URL https://github.com/agoessling/stuff_sack/archive/refs/tags/v0.2.0.zip
  URL_HASH SHA256=5ee8510ee3e61d996fdfee05e546935d1dd6608ed3c37620cb8ebf41bf8b4750
)

add_library(
  stuff_sack STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/stuff_sack-src/src/dynamic/type_descriptors.cc
  ${CMAKE_CURRENT_BINARY_DIR}/stuff_sack-src/src/uid_hash.cc
)

target_include_directories(
  stuff_sack PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/stuff_sack-src
)

target_link_libraries(
  stuff_sack
  yaml-cpp
)

add_library(
  ParserStuffSack SHARED
  stuff_sack_parser.cpp
  stuff_sack_parser.h
  stuff_sack_ui.h
)

target_link_libraries(
  ParserStuffSack
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  plotjuggler_base
  plotjuggler_qwt
  stuff_sack
)

install(
  TARGETS ParserStuffSack
  DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY}
)
