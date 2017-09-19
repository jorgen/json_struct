#find_program(EXTRACTOR_EXECUTABLE extractor)
set(EXTRACTOR_EXECUTABLE $<TARGET_FILE:extractor>)

function(add_json_extract)
    set(TARGET ${ARGV0})
    list(REMOVE_AT ARGV 0)
    set(LIST_OF_OUTPUT_FILES "")
    foreach(f ${ARGV})
        if (NOT IS_ABSOLUTE ${f})
            set(f "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
        endif()
        get_filename_component(filename ${f} NAME)
        set(OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/${filename}.json")
        if (WIN32)
            set(CLANG_ARGS "-std=c++14")
        else()
            set(CLANG_ARGS "-I${PROJECT_SOURCE_DIR}/include/extractor/clang_headers -I${PROJECT_SOURCE_DIR}/include/extractor/c++/v1  -std=c++11")
        endif()
        add_custom_command(OUTPUT "${OUTPUT_NAME}"
                            COMMAND ${EXTRACTOR_EXECUTABLE} -o "${OUTPUT_NAME}" "${f}" -- clang++ -I${PROJECT_SOURCE_DIR}/include ${CLANG_ARGS}
                            IMPLICIT_DEPENDS CXX "${f}")
        list(APPEND LIST_OF_OUTPUT_FILES "${OUTPUT_NAME}")
    endforeach()
    add_custom_command(OUTPUT "${TARGET}.json"
                       COMMAND ${EXTRACTOR_EXECUTABLE} -l -o "${TARGET}.json" ${LIST_OF_OUTPUT_FILES} -- clang++
                       DEPENDS ${LIST_OF_OUTPUT_FILES})
    add_custom_target(${TARGET}
                      DEPENDS "${TARGET}.json")
endfunction()

add_json_extract(json_extract_test ${CMAKE_CURRENT_SOURCE_DIR}/a.cpp b.cpp)
