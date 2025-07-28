#pragma once
#include <utility> // Для std::pair
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

class ConverterJSON
{
public:
    // Конструктор по умолчанию
    ConverterJSON() = default;

    // Проверяет наличие и корректность config.json и requests.json
    bool CheckConfigFiles() const;

    // Метод получения содержимого файлов
    // return Возвращает список с содержимым файлов перечисленных
    // в config.json
    std::vector<std::string> GetTextDocuments();

    // Метод считывает поле max_responses для определения максимального
    // количества ответов на один запрос
    int GetResponsesLimit();

    // Метод получения запросов из файла requests.json
    // return Возвращает список запросов из файла requests.json
    std::vector<std::string> GetRequests();

    // Метод записи ответов.
    // Положить в файл answers.json результаты поисковых запросов
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);
};