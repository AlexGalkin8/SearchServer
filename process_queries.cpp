#include "process_queries.h"

#include <algorithm>
#include <execution>
#include <functional>
#include <numeric>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> result(queries.size());

    std::transform(std::execution::par,
        queries.begin(),
        queries.end(),
        result.begin(),
        [&search_server](const std::string& query)
        { return search_server.FindTopDocuments(query); });

    return result;
}

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> res_queries = ProcessQueries(search_server, queries);

     std::list<Document> result;

     std::for_each(res_queries.begin(), res_queries.end(),
         [&result](std::vector<Document>& val)
         { result.insert(result.end(), val.begin(), val.end()); });

     return result;
}