SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    .
)

SET(CHRONICLE_HDR
    util/buffer_view.h
    util/cache.h
    util/files.h
    util/math_util.h
    util/parse.h
    util/spin_lock.h
    util/streamer.h
    util/stop_bit.h
    util/test_helpers.h
    util/thread.h

    region.h
    region_utils.h
    vanilla_chronicle.h
    vanilla_chronicle_settings.h
    vanilla_index.h
    vanilla_data.h
    vanilla_date.h
    vanilla_utils.h
    excerpt_appender.h
    excerpt_tailer.h
    formatters.h
)


SET(CHRONICLE_SRC
    util/buffer_view.cpp
    util/files.cpp
    util/thread.cpp
    util/stop_bit.cpp

    region.cpp
    vanilla_chronicle.cpp
    vanilla_chronicle_settings.cpp
    vanilla_index.cpp
    vanilla_data.cpp
    vanilla_date.cpp
    vanilla_utils.cpp
    excerpt_appender.cpp
    excerpt_tailer.cpp
)

ADD_LIBRARY(cornelich ${CHRONICLE_HDR} ${CHRONICLE_SRC})
TARGET_LINK_LIBRARIES(cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(cornelich PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)

EXPORT(TARGETS cornelich
       FILE ${CMAKE_CURRENT_BINARY_DIR}/cornelich-build.cmake
)

INSTALL(TARGETS cornelich
        EXPORT cornelich-install
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

INSTALL(DIRECTORY 
        EXPORT cornelich-install
        ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)


INSTALL(EXPORT cornelich-install
        DESTINATION lib
        COMPONENT dev
)

EXPORT(PACKAGE cornelich-package)
