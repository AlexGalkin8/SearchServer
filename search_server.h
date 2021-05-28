#pragma once

#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"
#include "log_duration.h"

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <execution>
#include <string_view>
#include <functional>
#include <type_traits>
#include <future>

const int MAX_RESULT_DOCUMENT_COUNT = 5;


class SearchServer
{
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string_view stop_words_text);

    explicit SearchServer(const std::string& stop_words_text);

    inline int GetDocumentCount() const noexcept
    {
        return documents_.size();
    }

    inline auto begin() const
    {
        return document_ids_.cbegin();
    }

    inline auto end() const
    {
        return document_ids_.cend();
    }

    /* @brief Adding a document to the server.
     * @param document_id - id of the added document.
     * @param document - document content.
     * @param status - document status (see definition of "DocumentStatus").
     * @param ratings - document grades vector. */
    void AddDocument(int document_id,
                     const std::string_view document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    /* @brief Search method and compilation of the top documents on query.
     * @param query - custom document search query.
     * @param document_predicate - custom sort predicate.
     * @return Vector top documents ranked by rating. */
    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query,
        DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query, DocumentStatus status) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;
    
    /* @brief A method that checks which query words are contained in the document.
     *        If the document contains minus words, then the return value will be empty.
     * @param raw_query - custom document search query.
     * @param document_id - ID of the document whose words 
     *    are checked for compliance with the query.
     * @return Query words present in the document and and document status. */
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const;

    /* @brief Method for obtaining word frequency by document id.
     * @param document_id - id of the document in which word frequency is checked.
     * @return Words and their frequency in the document. */
    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    /* @brief Method for removing documents from a search server.
     * @param document_id - id of the deleted document. */
    void RemoveDocument(int document_id);

    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);

    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord
    {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query
    {
        std::set<std::string_view, std::less<>> plus_words;
        std::set<std::string_view, std::less<>> minus_words;
    };

    std::set<int> document_ids_;
    const std::set<std::string, std::less<>> stop_words_;
    std::map<int, std::map<std::string, double, std::less<>>> document_to_word_freqs_;

    /* @param std::string - word from document;
     * @param std::map<int, double>]:
     *        @param int - document id;
     *        @param double - word frequency in the document; */
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;

    /* @param int - document id;
     * @param DocumentData: document status and rating; */
    std::map<int, DocumentData> documents_;

    /* @brief Average rating calculation.
     * @param ratings - vector of ratings.
     * @return Average rating */
    static int ComputeAverageRating(const std::vector<int>& ratings);

    /* @brief Word check is stop word.
     * @param word - word to check.
     * @return true - this is a stop-word false;
     *         false - don't stop-word */
    inline bool IsStopWord(const std::string_view word) const;

    /* @brief Splits the string into a vector and cheks valid.
     * @param text - string to split.
     * @return Vector of words. */
    std::vector<std::string> SplitIntoWordsNoStop(const std::string_view text) const;

    /* @brief Forming a QueryWord structure from an input string.
     * @param text - the string from which the QueryWord is formed.
     * @return struct QueryWord. 
     * @see SearchServer::QueryWord */
    QueryWord ParseQueryWord(const std::string_view text) const;

    /* @brief Forming a Query structure from an input string.
     * @param text - the string from which the Query is formed.
     * @return struct Query.
     * @see SearchServer::Query. */
    Query ParseQuery(const std::string_view text) const;

    /* @brief Calculate IDF — inverse document frequency.
     * @param word - word for composing it IDF.
     * @return IDF word. */
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    /* @brief Lists documents found by query.
     *  1. sort by plus and minus words;
     *  2. sorting by DocumentPredicate.
     * @param query - custom document search query.
     * @param document_predicate - custom sort predicate.
     * @return Vector documents ranked by rating. */
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&,
        const Query& query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::parallel_policy&,
        const Query& query, DocumentPredicate document_predicate) const;
};


/* @brief External function to add documents to search server.
 * @param [in] search_server - the server to which you want to add the document.
 * @param [out] search_server - the server to which the document was added.
 * @param document_id - id of the added document.
 * @param document - document content.
 * @param status - document status (see definition of "DocumentStatus").
 * @param ratings - document grades vector. */
void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings);


template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query,
    DocumentPredicate document_predicate) const
{
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    std::sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs)
        {
            return (std::abs(lhs.relevance - rhs.relevance) < 1e-6) ?
                lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);

    return matched_documents;
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query, DocumentStatus status) const
{
    return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating)
        {
            return document_status == status;
        });
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy& policy, const std::string_view raw_query) const
{
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
    DocumentPredicate document_predicate) const
{
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::sequenced_policy&,
    const Query& query, DocumentPredicate document_predicate) const
{
    std::map<int, double> document_to_relevance;
    std::vector<Document> matched_documents;

    for (const std::string_view& word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
            continue;

        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(std::string{ word }))
        {
            const auto& document_data = documents_.at(document_id);

            if (document_predicate(document_id, document_data.status, document_data.rating))
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
        }
    }

    for (const std::string_view& word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
            continue;

        for (const auto& [document_id, _] : word_to_document_freqs_.at(std::string{ word }))
            document_to_relevance.erase(document_id);
    }

    for (const auto& [document_id, relevance] : document_to_relevance)
    {
        matched_documents.push_back({ document_id,
                                      relevance,
                                      documents_.at(document_id).rating });
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::parallel_policy&,
    const Query& query, DocumentPredicate document_predicate) const
{
    std::map<int, double> document_to_relevance;
    std::vector<Document> matched_documents;
    std::vector<std::string_view> words(word_to_document_freqs_.size());
    size_t words_size = 0;

    std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(),
        words.begin(),
        [this, &words_size](const auto& word)
        { ++words_size;  return word_to_document_freqs_.count(word) != 0; });

    words.resize(words_size);

    for (const std::string_view& word : words)
    {
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(std::string{ word }))
        {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating))
            {
                document_to_relevance[document_id] += term_freq * ComputeWordInverseDocumentFreq(word);
            }
        }
    }

    for (const std::string_view& word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }

        for (const auto& [document_id, _] : word_to_document_freqs_.at(std::string{ word }))
        {
            document_to_relevance.erase(document_id);
        }
    }

    for (const auto& [document_id, relevance] : document_to_relevance)
    {
        matched_documents.push_back({ document_id,
                                      relevance,
                                      documents_.at(document_id).rating });
    }

    return matched_documents;
}
