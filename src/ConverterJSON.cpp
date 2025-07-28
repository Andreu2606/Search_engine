#include <fstream>
#include <iostream>
#include "ConverterJSON.h"

// Вспомогательная функция для получения пути к файлу JSON
std::string GetJsonPath(const std::string& filename)
{
    // Пробуем найти файл JSON в нескольких местах
    const std::string paths[] =
        {
            "JSON/" + filename,    // Папка JSON рядом с исполняемым файлом
            "../JSON/" + filename, // Папка JSON на уровень выше
            filename               // Прямо в текущей директории
        };

    for (const auto& path : paths)
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            file.close();
            return path;
        }
    }
    return "JSON/" + filename; // Возвращаем путь по умолчанию
}

bool ConverterJSON::CheckConfigFiles() const {
    // Явно указываем путь к файлам в папке JSON
    std::string configPath = GetJsonPath("config.json");
    std::string requestsPath = GetJsonPath("requests.json");

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        std::cerr << "Error: config.json not found at: " << std::filesystem::absolute(configPath) << std::endl;
        return false;
    }
    std::ifstream requestFile(requestsPath);
    if (!requestFile.is_open()) {
        std::cerr << "Error: requests.json not found at: " << std::filesystem::absolute(requestsPath) << std::endl;
        return false;
    }
    return true;
}

// Метод получения содержимого файлов
// return Возвращает список с содержимым файлов перечисленных
// в config.json
std::vector<std::string> ConverterJSON::GetTextDocuments()
{
    std::vector<std::string> documents;
    // Открытие и чтение конфигурационного файла
    const std::string configPath(GetJsonPath("config.json")); // Сохраняем путь для сообщений об ошибках
    std::ifstream config_file(configPath);

    if (!config_file.is_open())
    {
        std::cerr << "Error: Could not open config.json at: " << configPath << std::endl;
        return documents;
    }
    try
    {
        // Парсим файл конфигурации и преобразуем его в json объект
        json config = json::parse(config_file);
        config_file.close();

        // Проверка наличия обязательного поля
        if (!config.contains("files"))
        {
            std::cerr << "Warning: Missing 'files' field in config.json" << std::endl;
            return documents;
        }

        documents.reserve(config["files"].size()); // Оптимизация: резервируем память заранее

        // Чтение документов
        for (const auto& filePath : config["files"])
        {
            // Промежуточная переменная выведит путь к документу при ошибке
            const std::string path = filePath.get<std::string>();
            std::ifstream doc_file(path);

            if (doc_file.is_open())
            {
                // Промежуточная переменная с возможностью проверки на пустоту
                std::string content((std::istreambuf_iterator<char>(doc_file)), std::istreambuf_iterator<char>());

                if (!content.empty())
                {
                    documents.push_back(std::move(content));
                }
                doc_file.close();
            } else {
                std::cerr << "Error: Could not open document file: " << path << std::endl;
            }
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON parsing error in GetTextDocuments: " << e.what() << std::endl;
    }
    return documents;
}

// Метод считывает поле max_responses для определения максимального
// количества ответов на один запрос
int ConverterJSON::GetResponsesLimit()
{
    const std::string configPath = GetJsonPath("config.json");
    std::ifstream config_file(configPath);

    if (!config_file.is_open())
    {
        std::cerr << "Warning: Could not open config.json, using default limit 5" << std::endl;
        return 5;
    }
    try
    {
        json config = json::parse(config_file);
        config_file.close();

        if (config.contains("config") && config["config"].contains("max_responses"))
        {
            return config["config"]["max_responses"].get<int>();
        }
        else if (config.contains("max_responses"))
        {
            return config["max_responses"].get<int>();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parsing error in GetResponsesLimit: " << e.what() << std::endl;
    }
    return 5; // Возвращаем значение по умолчанию
}

// Метод получения запросов из файла requests.json
// return Возвращает список запросов из файла requests.json
std::vector<std::string> ConverterJSON::GetRequests()
{
    std::vector<std::string> requests;
    // Открытие файла с проверкой ошибок
    const std::string requestsPath = GetJsonPath("requests.json");
    std::ifstream requests_file(requestsPath);

    if (!requests_file.is_open())
    {
        std::cerr << "Error: Could not open requests.json at: " << requestsPath << std::endl;
        return requests;
    }
    try
    {
        json request_config = json::parse(requests_file);
        requests_file.close();

        // Проверка наличия обязательного поля
        if (request_config.contains("requests"))
        {
            requests.reserve(request_config["requests"].size()); // Оптимизация: резервируем память для запросов
            // Извлечение запросов
            for (const auto& request : request_config["requests"])
            {
                requests.emplace_back(request.get<std::string>());
            }
        } else {
            std::cerr << "Warning: Missing 'requests' field in requests.json" << std::endl;
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON parsing error in GetRequests: " << e.what() << std::endl;
    }
    return requests;
}

// Метод записи ответов
// Положить в файл answers.json результаты поисковых запросов
void ConverterJSON::putAnswers(std::vector<std::vector<std::pair<int, float>>> answers)
{
    const std::string answersPath = GetJsonPath("answers.json");
    std::ofstream output_file;

    try
    {
        output_file.open(answersPath, std::ios::trunc); // добавляем флаг trunc для перезаписи файла
        if (!output_file.is_open())
        {
            throw std::runtime_error("Failed to create answers file at: " + answersPath);
        }
        // Создаем json объект и добавляем поле answers
        ordered_json result = {
            {"answers", ordered_json::object()}
        };

        // Заполняем поле answers соответствующими значениями
        for (size_t i = 0; i < answers.size(); ++i)
        {
            const auto& answer = answers[i]; // Ссылка для избежания копирования

            // Форматируем номер запроса с ведущими нулями (001, 002...)
            std::string requestKey = "request" +
                                     std::string(3 - std::to_string(i + 1).length(), '0') +
                                     std::to_string(i + 1);

            // Создаем json объект для текущего запроса и добавляем его в поле answers
            ordered_json requestResult;
            requestResult["result"] = !answer.empty();

            // Если ответ не пустой, заполняем поле docid и rank
            if (!answer.empty())
            {
                ordered_json relevanceArray = ordered_json::array();
                // Проходим по контейнеру и извлекаем пары
                for (const auto& [docid, rank]: answer)
                {
                    double intPart = static_cast<int>(rank); // выделяем целую часть
                    double fracPart = static_cast<int>((rank - intPart) * 1000); // выделяем дробную часть

                    std::stringstream ss; // поток для формирования строки
                    // форматируем число "целая.дробная" часть
                    ss << intPart << '.' << std::setw(3) << std::setfill('0') << fracPart;
                    std::string rankStr = ss.str(); // получаем отформатированную строку из потока
                    // Удаляем .000 если есть
                    size_t dotPos = rankStr.find('.');
                    if (dotPos != std::string::npos) {
                        while (!rankStr.empty() && rankStr.back() == '0')
                            rankStr.pop_back();
                        if (!rankStr.empty() && rankStr.back() == '.')
                            rankStr.pop_back();
                    }
                    // Добавляем каждую пару в объект relevance
                    relevanceArray.push_back
                        ({
                             {"docid", docid}, {"rank", rankStr}
                         });
                }
                requestResult["relevance"] = std::move(relevanceArray);
            }
            result["answers"][requestKey] = std::move(requestResult);
        }
        // Записываем результат в файл answers.json с отступами в 4 пробела
        output_file << result.dump(4);

        if (output_file.fail()) // Проверка на ошибки
        {
            throw std::runtime_error("Failed to write data to answers.json");
        }
    }
    catch (const std::exception& e) {
        if (output_file.is_open())
        {
            output_file.close();
        }
        std::cerr << "Error in putAnswers: " << e.what() << std::endl;
        throw; // Передаем исключение дальше
    }
    output_file.close();
}