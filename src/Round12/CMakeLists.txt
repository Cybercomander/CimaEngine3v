project(CimaEngine3v)

# ROUND 12: todo el código del juego (las pruebas compilan aparte)
file(GLOB_RECURSE r12_cpp CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER r12_cpp EXCLUDE REGEX "/pruebas/")
target_sources(CimaEngine3v PRIVATE ${r12_cpp})

# Pruebas de la lógica pura (sin SFML): ctest --test-dir build -C Debug
file(GLOB r12_logica CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Logica/*.cpp")
add_executable(Round12Pruebas ${r12_logica} pruebas/pruebas_logica.cpp)
target_include_directories(Round12Pruebas PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(Round12Pruebas PRIVATE cxx_std_17)
if(NOT MSVC)
    target_compile_options(Round12Pruebas PRIVATE -Wall -Wextra -Werror)
endif()
add_test(NAME round12_logica COMMAND Round12Pruebas)
