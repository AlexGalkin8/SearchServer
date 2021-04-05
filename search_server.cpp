#include "search_server.h"
#include "string_processing.h"

#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>


/*********************************************************
 ***************   Public class members   ****************
 *********************************************************/

void SearchServer::AddDocument(int document_id,
                               const std::string& document,
                               DocumentStatus status,
                               const std::vector<int>& ratings)
{
    if ((document_id < 0) || (documents_.count(document_id) > 0))
    {
        throw std::invalid_argument("Invalid document_id");
    }

    const auto words = SplitIntoWordsNoStop(document);

    // TF calculation for each word of the document.
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words)
    {
        // Each time a word is repeated in a document, the frequency increases.
        document_to_word_freqs_[document_id][word] += inv_word_count;
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }

    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const
{
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
        {
            return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const
{
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(
    const std::string& raw_query,int document_id) const
{
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;

    for (const std::string& word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
            continue;

        if (word_to_document_freqs_.at(word).count(document_id))
            matched_words.push_back(word);
    }

    // Checking for the absence of minus words in the document.
    for (const std::string& word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
            continue;

        if (word_to_document_freqs_.at(word).count(document_id))
        {
            matched_words.clear();
            break;
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    static std::map<std::string, double> word_frequencies;

    word_frequencies.clear();

    if (documents_.count(document_id) == 0)
        return word_frequencies;

    word_frequencies = document_to_word_freqs_.at(document_id);

    return word_frequencies;
}

void SearchServer::RemoveDocument(int document_id)
{
    if (documents_.count(document_id) == 0)
        return;

    document_ids_.erase(document_id);
    documents_.erase(document_id);

    for (auto& [word, _] : GetWordFrequencies(document_id))
    {
        word_to_document_freqs_.at(word).erase(document_id);

        if (word_to_document_freqs_.at(word).empty())
            word_to_document_freqs_.erase(word);
    }
}


/*********************************************************
 ***************   Private class members   ***************
 *********************************************************/

bool SearchServer::IsValidWord(const std::string& word)
{
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const
{
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text))
    {
        if (!IsValidWord(word))
        {
            throw std::invalid_argument("Word " + word + " is invalid");
        }
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings)
{
    if (ratings.empty())
    {
        return 0;
    }

    int rating_sum = 0;
    for (const int rating : ratings)
    {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const
{
    if (text.empty())
    {
        throw std::invalid_argument("Query word is empty");
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
    {
        throw std::invalid_argument("Query word " + text + " is invalid");
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const
{
    Query result;
    for (const std::string& word : SplitIntoWords(text))
    {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                result.minus_words.insert(query_word.data);
            }
            else
            {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


/*********************************************************
 ************   Functions outside the class   ************
 *********************************************************/

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings)
{
    try
    {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "������ ���������� ��������� " << document_id << ": " << e.what() << std::endl;
    }
}

