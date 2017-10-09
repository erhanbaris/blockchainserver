set(HTTPPARSERDIR ${CMAKE_SOURCE_DIR}/deps/http_parser)

include_directories(${HTTPPARSERDIR})

add_library(http_parser STATIC
  ${HTTPPARSERDIR}/http_parser.c
)
set(HTTPPARSER_INCLUDE_DIR ${HTTPPARSERDIR})