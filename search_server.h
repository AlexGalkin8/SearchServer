#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdexcept>


const int MAX_RESULT_DOCUMENT_COUNT = 5;


class SearchServer
{
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}

    explicit SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    /* @return Document ñount. */
    inline int GetDocumentCount() const noexcept
    {
        return documents_.size();
    }

    /* @return Const begin iterator. */
    inline auto begin() const
    {
        return document_ids_.cbegin();
    }

    /* @return Const end iterator. */
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
                     const std::string& document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    /* @brief Search method and compilation of the top documents on query.
     * @param query - custom document search query.
     * @param document_predicate - custom sort predicate.
     * @return Vector top documents ranked by rating. */
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query,
        DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;
    
    /* @brief A method that checks which query words are contained in the document.
     *        If the document contains minus words, then the return value will be empty.
     * @param raw_query - custom document search query.
     * @param document_id - ID of the document whose words 
     *    are checked for compliance with the query.
     * @return Query words present in the document and and document status. */
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(
        const std::string& raw_query, int document_id) const;

    /* @brief Method for obtaining word frequency by document id.
     * @param document_id - id of the document in which word frequency is checked.
     * @return Words and their frequency in the document. */
    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    /* @brief Method for removing documents from a search server.
     * @param document_id - id of the deleted document. */
    void RemoveDocument(int document_id);

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord
    {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query
    {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<int> document_ids_;
    const std::set<std::string> stop_words_;

    /* @param std::string - word from document;
     * @param std::map<int, double>]:
     *        @param int - document id;
     *        @param double - word frequency in the document; */
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;

    /* @param int - document id;
     * @param DocumentData: document status and rating; */
    std::map<int, DocumentData> documents_;

    /* @brief Check the absence of special characters in the word.
     * @param word - word to check.
     * @return Correct (true) or incorrect (false). */
    static bool IsValidWord(const std::string& word);

    /* @brief Average rating calculation.
     * @param ratings - vector of ratings.
     * @return Average rating */
    static int ComputeAverageRating(const std::vector<int>& ratings);

    /* @brief Word check is stop word.
     * @param word - word to check.
     * @return true - this is a stop-word false;
     *         false - don't stop-word */
    inline bool IsStopWord(const std::string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    /* @brief Splits the string into a vector and cheks valid.
     * @param text - string to split.
     * @return Vector of words. */
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    /* @brief Forming a QueryWord structure from an input string.
     * @param text - the string from which the QueryWord is formed.
     * @return struct QueryWord. 
     * @see SearchServer::QueryWord */
    QueryWord ParseQueryWord(const std::string& text) const;

    /* @brief Forming a Query structure from an input string.
     * @param text - the string from which the Query is formed.
     * @return struct Query.
     * @see SearchServer::Query. */
    Query ParseQuery(const std::string& text) const;

    /* @brief Calculate IDF — inverse document frequency.
     * @param word - word for composing it IDF.
     * @return IDF word. */
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    /* @brief Lists documents found by query.
     *  1. sort by plus and minus words;
     *  2. sorting by DocumentPredicate.
     * @param query - custom document search query.
     * @param document_predicate - custom sort predicate.
     * @return Vector documents ranked by rating. */
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;

    /* @brief Method of composing unique words.
     * @param strings - container contains non-unique words.
     * @return Unique words */
    template <class StringContainer>
    std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings);
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


template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
    DocumentPredicate document_predicate) const
{
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query, document_predicate);

    std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs)
        {
            if (std::abs(lhs.relevance - rhs.relevance) < 1e-6)
            {
                return lhs.rating > rhs.rating;
            }
            else
            {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}


template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const
{
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }

        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
        {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating))
            {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }

        for (const auto [document_id, _] : word_to_document_freqs_.at(word))
        {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance)
    {
        matched_documents.push_back({ document_id,
                                      relevance,
                                      documents_.at(document_id).rating });
    }
    return matched_documents;
}


template <class StringContainer>
std::set<std::string> SearchServer::MakeUniqueNonEmptyStrings(const StringContainer& strings)
{
    for (const std::string& str : strings)
    {
        if (!IsValidWord(str))
            throw std::invalid_argument("Stop words contain invalid characters");
    }

    std::set<std::string> non_empty_strings;

    for (const std::string str : strings)
    {
        if (!str.empty())
        {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

