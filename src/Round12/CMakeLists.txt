project(CimaEngine3v)

# ROUND 12: todo el código del juego (las pruebas compilan aparte)
file(GLOB_RECURSE r12_cpp CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER r12_cpp EXCLUDE REGEX "/pruebas/")
target_sources(CimaEngine3v PRIVATE ${r12_cpp})
