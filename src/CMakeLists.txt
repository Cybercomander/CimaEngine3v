project(CimaEngine3v)

add_subdirectory(Main)
add_subdirectory(Motor)
add_subdirectory(Juego)
add_subdirectory(Round12)


target_include_directories(CimaEngine3v PRIVATE ./)
