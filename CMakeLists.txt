cmake_minimum_required(VERSION 3.5)

project(Argus)

include_directories(include/)
link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB PROJECT_HEADERS include/*.h)
file(GLOB PROJECT_SOURCES src/*.cpp)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
						  Readme.md
						  .gitignore)

source_group("include" FILES ${PROJECT_HEADERS})
source_group("src" FILES ${PROJECT_SOURCES})

add_executable(${PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${PROJECT_CONFIGS})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} assimp-vc142-mt)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/lib/assimp-vc142-mt.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)