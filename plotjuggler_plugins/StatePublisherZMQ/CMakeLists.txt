
## load in pkg-config support
find_package(PkgConfig QUIET)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

include_directories(../ ${ZeroMQ_INCLUDE_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

add_library(PublisherZMQ SHARED publisher_zmq.cpp )

target_link_libraries(PublisherZMQ
    ${Qt5Widgets_LIBRARIES}
    plotjuggler_base
    ${ZeroMQ_LIBRARY}
    )

install(TARGETS PublisherZMQ DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY}  )

