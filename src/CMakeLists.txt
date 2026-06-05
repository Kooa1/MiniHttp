file(GLOB_RECURSE CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)
file(GLOB_RECURSE HTTP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/http/*.cpp)
file(GLOB_RECURSE UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)
file(GLOB_RECURSE THREAD_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/thread/*.cpp)
file(GLOB_RECURSE SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/server/*.cpp)
file(GLOB_RECURSE RESOURCES_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.cpp)
file(GLOB_RECURSE ROUTE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/handler/*.cpp)
file(GLOB_RECURSE NET_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/net/*.cpp)

add_library(minihttp_net STATIC
        ${CORE_SOURCES}
        ${HTTP_SOURCES}
        ${UTILS_SOURCES}
        ${THREAD_SOURCES}
        ${SERVER_SOURCES}
        ${RESOURCES_SOURCES}
        ${ROUTE_SOURCES}
        ${NET_SOURCES}
)

target_include_directories(minihttp_net PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(minihttp_server main.cpp)
target_link_libraries(minihttp_server PRIVATE minihttp_net)
