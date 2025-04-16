cmake_minimum_required(VERSION 3.15)
project(ServerApp)

set(CMAKE_CXX_STANDARD 17)

# Настройки для Windows
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Исходные файлы
set(SOURCE_FILES server.cpp)

# Создание исполняемого файла
add_executable(ServerApp ${SOURCE_FILES})

# Подключение библиотек Winsock2 для Windows
if(WIN32)
    target_link_libraries(ServerApp ws2_32)
endif()