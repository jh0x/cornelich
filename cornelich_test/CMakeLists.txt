SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM
    ${CATCH_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    .
)

SET(UTIL_TEST_SRC
    main.cpp

    cache_test.cpp
    buffer_view_test.cpp
    files_test.cpp
    math_util_test.cpp
    parse_test.cpp
    stop_bit_test.cpp
    streamer_test.cpp
    thread_test.cpp
)

SET(CHRONICLE_TOOLS_SRC
    main.cpp

    formatters_test.cpp
    region_test.cpp
    vanilla_chronicle_settings_test.cpp
    vanilla_date_test.cpp
)

SET(READING_JAVA_CHRONICLE_SRC
    main.cpp

    reading_java_chronicle.cpp
)

SET(VANILLA_CHRONICLE_LONG_SRC
    main.cpp

    write_test_data.h
    vanilla_chronicle_long_test.cpp
)



SET(VANILLA_CHRONICLE_SRC
    main.cpp
    
    write_test_data.h
    vanilla_chronicle_test.cpp
)


ADD_EXECUTABLE(util_test ${UTIL_TEST_SRC})
TARGET_LINK_LIBRARIES(util_test cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

ADD_EXECUTABLE(chronicle_tools_test ${CHRONICLE_TOOLS_SRC})
TARGET_LINK_LIBRARIES(chronicle_tools_test cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

ADD_EXECUTABLE(reading_java_chronicle ${READING_JAVA_CHRONICLE_SRC})
# We need to unpack the test data...
untar(reading_java_chronicle ${CMAKE_CURRENT_SOURCE_DIR}/SampleVanillaChronicle.tar.bz2 ${CMAKE_CURRENT_BINARY_DIR}/test_data)
TARGET_LINK_LIBRARIES(reading_java_chronicle cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

ADD_EXECUTABLE(vanilla_chronicle_test ${VANILLA_CHRONICLE_SRC})
TARGET_LINK_LIBRARIES(vanilla_chronicle_test cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

ADD_EXECUTABLE(vanilla_chronicle_long_test ${VANILLA_CHRONICLE_LONG_SRC})
TARGET_LINK_LIBRARIES(vanilla_chronicle_long_test cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

ADD_EXECUTABLE(cornelich_read_write_cycle main.cpp cornelich_read_write_cycle.cpp)
TARGET_LINK_LIBRARIES(cornelich_read_write_cycle cornelich ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})


# Register the test with CTest
ADD_TEST("util_test" util_test)
ADD_TEST("chronicle_tools_test" chronicle_tools_test)
ADD_TEST("vanilla_chronicle_test" vanilla_chronicle_test)
ADD_TEST("reading_java_chronicle" reading_java_chronicle)
ADD_TEST("vanilla_chronicle_long_test" vanilla_chronicle_long_test)

CONFIGURE_FILE(test_data.h.in test_data.h)

