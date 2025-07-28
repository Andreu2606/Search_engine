#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>

// Структура для хранения информации о вхождении слова в документ
struct Entry
{
    size_t doc_id; // Идентификатор документа

    size_t count; // Количество вхождений слова

    // Оператор сравнения для тестирования
    bool operator ==(const Entry& other) const
    {
        return (doc_id == other.doc_id && count == other.count);
    }
};

class InvertedIndex
{
public:
    InvertedIndex() = default;

    // Обновляет базу документов, передается вектор строк с содержимым документов
    void UpdateDocumentBase(std::vector<std::string> input_docs);

    // Получает частоту слов для конкретного документа по его номеру в базе
    std::vector<Entry> GetWordCount(const std::string& word) const;

    size_t GetTotalDocuments() const
    {
        return docs.size();  // docs - это вектор документов
    }

private:

    std::vector<std::string> docs; // Вектор строк с содержимым документов

    std::map<std::string, std::vector<Entry>> freq_dictionary; // Словарь частот слов в документах

    std::string normalizeWord(const std::string& word) const;


};