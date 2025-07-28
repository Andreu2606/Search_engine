#include <cctype>
#include <algorithm>
#include <sstream>
#include "InvertedIndex.h"

// Обновляет базу документов, передается вектор строк с содержимым документов
void InvertedIndex::UpdateDocumentBase(std::vector<std::string> input_docs)
{
    if (input_docs.empty()) // проверка на пустой вектор
    {
        docs.clear();            // очищаем вектор
        freq_dictionary.clear(); // очищаем словарь
        return;                  // выходим из функции
    }
    docs = std::move(input_docs); // перемещаем вектор (вместо копирования)
    freq_dictionary.clear();      // очищаем частотный словарь

    // Обрабатываем каждый документ
    for (size_t doc_id = 0; doc_id < docs.size(); ++doc_id)
    {
        if (docs[doc_id].empty()) continue; // пропускаем пустые документы

        std::istringstream iss(docs[doc_id]); // создаем строковый поток для чтения из содержимого документа
        std::map<std::string, size_t> word_counts; // временный словарь для подсчета количества слов в документе

        std::string word;

        // Разбиваем документ на слова
        // Читаем документ слово за словом
        while (iss >> word)
        {
            // Нормализуем слово(приводим к нижнему регистру и удаляем ненужные символы)
            word = normalizeWord(word);

            if (!word.empty()) // Если после нормализации слово не пустое
            {
                ++word_counts[word]; // Увеличиваем счетчик для этого слова
            }
        }
        // Добавляем результат в частотный словарь
        for (const auto& [word, count] : word_counts)
        {
            freq_dictionary[word].emplace_back(Entry{doc_id, count});
        }
    }
}
// Получает частоту слов для конкретного документа по его номеру в базе
std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word) const
{
    // Нормализуем слово для поиска
    const std::string normalized_word = normalizeWord(word);

    if (normalized_word.empty()) // Если после нормализации слово пустое
    {
        return {}; // Возвращаем пустой вектор
    }
    // Ищем слово в частотном словаре
    if (auto it = freq_dictionary.find(normalized_word); it != freq_dictionary.end())
    {
        return it->second; // Возвращаем вектор с результатами
    }
    return {}; // Возвращаем пустой вектор, если слово не найдено
}

std::string InvertedIndex::normalizeWord(const std::string& word) const {
    std::string result;
    result.reserve(word.size()); // резервируем память для строки по размеру слова

    for (char c: word) {
        if (std::isalnum(c)) // если символ является буквой или цифрой
        {
            result += std::tolower(c); // приводим к нижнему регистру
        }
    }
    return result; // возвращаем нормализованное слово
}