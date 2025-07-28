cmake_minimum_required(VERSION 3.29)
project(Search_engine VERSION 1.0)

# Установка стандарта C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Добавление поддиректории с nlohmann_json
add_subdirectory(nlohmann_json)

# Сначала определяем основной проект
add_executable(Search_engine
        src/main.cpp
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchServer.cpp
)

# Настройка включения директорий
target_include_directories(Search_engine PUBLIC
        ${CMAKE_SOURCE_DIR}/headers
)

# Связывание с nlohmann_json
target_link_libraries(Search_engine PRIVATE
        nlohmann_json::nlohmann_json
)

# Затем подключаем тесты (если они нужны)
if(BUILD_TESTING)
    # Загрузка Google Test
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL
            https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    set(gtest_disable_pthreads ON)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Добавляем тесты
    add_executable(Search_engine_tests
            tests/test.cpp
            tests/TestCaseInvertedIndex.cpp
            tests/TestCaseSearchServer.cpp
    )

    target_include_directories(Search_engine_tests PUBLIC
            ${CMAKE_SOURCE_DIR}/headers
            ${gtest_SOURCE_DIR}/include
    )

    target_link_libraries(Search_engine_tests PRIVATE
            gtest_main
            Search_engine  # линкуем с основным проектом
    )

    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(Search_engine_tests)
endif()