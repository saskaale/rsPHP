set(rsphp_SRCS
    main.cpp
    evaluator.cpp
    builtins.cpp
    environment.cpp
    ast.cpp
    aval.cpp
    memorypool.cpp
)

BISON_TARGET(phpParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(phpScanner lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(phpScanner phpParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rsphp
    ${rsphp_SRCS}
    ${BISON_phpParser_OUTPUTS}
    ${FLEX_phpScanner_OUTPUTS}
)

add_definitions(-std=gnu++0x)

#target_link_libraries(rsphp Qt5::Core)
