#include "remove_duplicates.h"

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>

void RemoveDuplicates(SearchServer& search_server)
{
    std::vector<int> documents_to_deleted; // @param int - document_id.
    std::map<std::set<std::string_view>, int> documents;

    for (const int document_id : search_server)
    {
        std::set<std::string_view> doc_words;

        auto doc_frequencies = search_server.GetWordFrequencies(document_id);

        std::transform(doc_frequencies.begin(), doc_frequencies.end(),
            std::inserter(doc_words, doc_words.end()), [](auto word) {return word.first; });

        if (documents.count(doc_words))
        {
            documents_to_deleted.push_back(document_id);
            std::cout << "Found duplicate document id " << document_id << std::endl;
        }
        else
        {
            documents.emplace(doc_words, document_id);
        }
    }

    for (const int document_id : documents_to_deleted)
    {
        search_server.RemoveDocument(document_id);
    }
}
