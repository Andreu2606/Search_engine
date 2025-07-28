#include <iostream>
#include <vector>
#include <utility> // для std::pair
#include "ConverterJSON.h"
#include "SearchServer.h"
#include "InvertedIndex.h"

// Вспомогательная функция для преоброзования рузультатов поиска
static std::vector<std::vector<std::pair<int, float>>>
ConvertResults(const std::vector<std::vector<RelativeIndex>>& searchResults)
{
    std::vector<std::vector<std::pair<int, float>>> converted;
    converted.reserve(searchResults.size());

    for (const auto& result : searchResults)
    {
        std::vector<std::pair<int, float>> convertedResult;
        convertedResult.reserve(result.size());

        for (const auto& index : result)
        {
            convertedResult.emplace_back(index.doc_id, index.rank);
        }
        converted.push_back(std::move(convertedResult));
    }
    return converted;
}

int main()
{
    try
    {
        // Инициализация конфигурации
        ConverterJSON converter;

        // Проверка конфигурационных файлов
        if (!converter.CheckConfigFiles())
        {
            std::cerr << "Config files are missing or invalid. Please check config.json and requests.json" << std::endl;
            return 1;
        }
        // Получаем список документов из config.json
        std::vector<std::string> documents = converter.GetTextDocuments();

        if (documents.empty())
        {
            std::cerr << "No documents found in config.json" << std::endl;
            return 1;
        }

        // Создаем и запролняем инвертированный индекс
        InvertedIndex index;
        index.UpdateDocumentBase(documents);

        // Инициализация поискового сервера
        SearchServer searchServer(index);

        // Получаем список запросов из requests.json
        std::vector<std::string> requests = converter.GetRequests();
        if (requests.empty())
        {
            std::cerr << "No requests found in requests.json" << std::endl;
        }

        // Обработка поисковых запросов
        std::vector<std::vector<RelativeIndex>> results = searchServer.search(requests);

        auto convertedResults = ConvertResults(results);

        // Сохранение результатов
        converter.putAnswers(convertedResults);

        std::cout << "Search completed successfully. Results saved to answers.json" << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}