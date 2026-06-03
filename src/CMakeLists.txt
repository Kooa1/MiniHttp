file(GLOB_RECURSE CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)
file(GLOB_RECURSE HTTP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/http/*.cpp)
file(GLOB_RECURSE UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)

add_library(minihttp_net STATIC
    ${CORE_SOURCES}
    ${HTTP_SOURCES}
    ${UTILS_SOURCES}
)

target_include_directories(minihttp_net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(minihttp_server main.cpp)
target_link_libraries(minihttp_server PRIVATE minihttp_net)
