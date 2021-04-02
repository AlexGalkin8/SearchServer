#include "remove_duplicates.h"

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>

void RemoveDuplicates(SearchServer& search_server)
{
    std::vector<int> documents_to_deleted; // @param int - document_id.
    std::map<std::set<std::string>, int> documents;

    for (const int document_id : search_server)
    {
        std::set<std::string> doc_words;
        documents.emplace();
        for (auto& [str, freq] : search_server.GetWordFrequencies(document_id))
        {
            doc_words.insert(str);
        }

        if (!documents.emplace(doc_words, document_id).second)
        {
            documents_to_deleted.push_back(document_id);
            std::cout << "Found duplicate document id " << document_id << std::endl;
        }
    }

    for (const int document_id : documents_to_deleted)
    {
        search_server.RemoveDocument(document_id);
    }
}
