#include "SearchServer.h"
#include "ConverterJSON.h"
#include <sstream>
#include <unordered_set>
#include <algorithm>

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input)
{
    ConverterJSON converter; // Объект для работы с JSON
    // Получаем максимальное количество документов в ответе
    const int response_limit = converter.GetResponsesLimit();

    // Отсортированный список релевантных ответов на запросы
    std::vector<std::vector<RelativeIndex>> result;
    for (const auto& query : queries_input)
    {
        std::vector<RelativeIndex> result_inner; // Список релевантных документов

        // Контейнер для хранения релевантных значений
        std::vector<size_t> absolute_relevance(_index.GetTotalDocuments(), 0);
        // список уникальных слов в запросе
        std::unordered_set<std::string> words_set;

        std::string word;
        std::stringstream buffer_stream(query); // Разбираем на слова

        // разбитие запроса на отдельные слова и формирование списка уникальных
        while (buffer_stream >> word)
        {
            if (!word.empty())
            {
                words_set.insert(word);
            }
        }

        // по doc_id добавляем количество встреч слова
        for (const auto& word : words_set)
        {
            for (const auto& entry : _index.GetWordCount(word))
            {
                absolute_relevance[entry.doc_id] += entry.count; // Увеличиваем количество вхождений
            }
        }
        // Самый релевантный докуммент
        const auto max_it = std::max_element(absolute_relevance.begin(), absolute_relevance.end());

        // рассчитываем относительную релевантность
        if (max_it != absolute_relevance.end() && *max_it != 0)
        {
            // Сохраняем для нормализации
            const float max_relatnce = static_cast<float>(*max_it);

            for (size_t doc_id = 0; doc_id < absolute_relevance.size(); ++doc_id)
            {
                if (absolute_relevance[doc_id] > 0)
                {
                    result_inner.emplace_back(RelativeIndex{ doc_id, static_cast<float>(absolute_relevance[doc_id]) / max_relatnce });
                }
            }
            std::sort(result_inner.begin(), result_inner.end(), [](RelativeIndex& a, RelativeIndex& b)
            {
                const float EPS = 1e-6; // Погрешность = 0.000001
                if (std::abs(a.rank - b.rank) > EPS)
                {
                    return a.rank > b.rank; // Сначала более релевантные
                }
                return a.doc_id < b.doc_id; // При равенстве - по возрастанию doc_id
            });
        }
        if (result_inner.size() > response_limit) // Ограничени количества ответов
        {
            result_inner.resize(response_limit);
        }
        result.emplace_back(std::move(result_inner)); // Перемещаем результат
    }

    std::vector<std::vector<std::pair<int, float>>> result_pairs; // Пары {doc_id, rank} для одного запроса
    for (const auto& vec : result)
    {
        std::vector<std::pair<int, float>> pairs; // Пары {doc_id, rank} для текущего запроса
        for (const auto& rel_index : vec)
        {
            // Заполнение pairs данными из RelativeIndex
            pairs.emplace_back(rel_index.doc_id, rel_index.rank);
        }
        result_pairs.emplace_back(std::move(pairs));
    }
    converter.putAnswers(result_pairs);

    return result;
}