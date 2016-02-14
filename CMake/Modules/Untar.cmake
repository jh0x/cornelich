FIND_PROGRAM(TAR_EXECUTABLE NAMES tar)
FIND_PROGRAM(BZIP_EXECUTABLE NAMES bzip2)

FUNCTION(untar _target _what _where)
    ADD_CUSTOM_COMMAND(TARGET ${_target}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_where}
        COMMAND ${TAR_EXECUTABLE} "-xf" ${_what} "-C" ${_where}
        DEPENDS ${_what}
        COMMENT "Unpacking ${_what}"
        )
    SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${_where})
ENDFUNCTION()

