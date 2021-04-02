#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status)
{
    return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
        {
            return document_status == status;
        });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query)
{
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

void RequestQueue::UpdateRequests(bool is_last_result_empty)
{
    timestamp++;

    QueryResult query_result;
    query_result.timestamp = timestamp;
    query_result.empty = is_last_result_empty;

    requests_.push_back(query_result);
    if (query_result.empty)
    {
        no_result_requests_count++;
    }

    RemoveOldRequests();
}

void RequestQueue::RemoveOldRequests()
{
    if (requests_.empty())
    {
        return;
    }

    long long requests_to_delete_count = (requests_.back().timestamp - requests_.front().timestamp + 1) - sec_in_day_;

    if (requests_to_delete_count <= 0)
    {
        return;
    }

    for (auto i = requests_to_delete_count; i > 0; --i)
    {
        if (requests_.front().empty)
        {
            no_result_requests_count--;
        }
        requests_.pop_front();
    }
}
