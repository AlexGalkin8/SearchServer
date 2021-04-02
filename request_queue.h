#pragma once

#include "search_server.h"

#include <vector>
#include <string>
#include <deque>

class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer& search_server)
        : search_server_(search_server) {}

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    inline int GetNoResultRequests() const noexcept
    {
        return no_result_requests_count;
    }

private:
    struct QueryResult
    {
        bool empty = false;
        long long timestamp = 0;
    };

    void UpdateRequests(bool is_last_result_empty);

    void RemoveOldRequests();

    std::deque<QueryResult> requests_;

    // максимальное количество запросов в день
    const static int sec_in_day_ = 1440;
    // количество нулевых результатов за день
    int no_result_requests_count = 0;
    // общее количество запросов за день
    long long timestamp = 0;

    const SearchServer& search_server_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query,
    DocumentPredicate document_predicate)
{
    auto documents = search_server_.FindTopDocuments(raw_query, document_predicate);
    UpdateRequests(documents.empty());
    return documents;
}