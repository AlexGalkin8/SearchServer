#include "search_server.h"
#include "string_processing.h"

#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

/*********************************************************
 ***************   Public class members   ****************
 *********************************************************/

SearchServer::SearchServer(const std::string_view stop_words_text)
    : SearchServer(SplitIntoWordsView(stop_words_text))
{
}

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id,
                               const std::string_view document,
                               DocumentStatus status,
                               const std::vector<int>& ratings)
{
    if ((document_id < 0) || (documents_.count(document_id) > 0))
    {
        throw std::invalid_argument("Invalid document_id");
    }

    const std::vector<std::string> words = SplitIntoWordsNoStop(document);

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

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const
{
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const
{
    return FindTopDocuments(std::execution::seq, raw_query);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const
{
    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;

    for (const std::string_view word : query.plus_words)
    {
        if (word_to_document_freqs_.count(std::string{ word }) == 0)
            continue;

        if (word_to_document_freqs_.at(std::string{ word }).count(document_id))
            matched_words.push_back(word);
    }

    // Checking for the absence of minus words in the document.
    for (const std::string_view word : query.minus_words)
    {
        if (word_to_document_freqs_.count(std::string{ word }) == 0)
            continue;

        if (word_to_document_freqs_.at(std::string{ word }).count(document_id))
        {
            matched_words.clear();
            break;
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const
{
    if (document_ids_.count(document_id) == 0)
        throw std::out_of_range("non-existing document_id");

    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words(word_to_document_freqs_.size());

    size_t words_size = 0;
    std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(),
        matched_words.begin(),
        [this, &words_size, document_id](const auto word)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                return false;
            }
            else if (word_to_document_freqs_.at(std::string{ word }).count(document_id) > 0)
            {
                words_size++;
                return true;
            }
            else
            {
                return false;
            }
        });

    matched_words.resize(words_size);

    if (std::any_of(std::execution::par,
        query.minus_words.begin(), query.minus_words.end(),
        [=](const auto word) {return
        (word_to_document_freqs_.count(word) == 0) ?
        false : (word_to_document_freqs_.at(std::string{ word }).count(document_id) > 0); })
        ) {
        matched_words.clear();
    }

    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    const std::string_view raw_query, int document_id) const
{
    return MatchDocument(std::execution::seq, raw_query, document_id);
}


const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    static std::map<std::string_view, double> word_frequencies;

    word_frequencies.clear();

    if (documents_.count(document_id) == 0)
        return word_frequencies;

    for (const auto& [word, freq] : document_to_word_freqs_.at(document_id))
    {
        std::string_view tmp{ word };
        word_frequencies.emplace(tmp, freq);
    }

    return word_frequencies;
}

void SearchServer::RemoveDocument(int document_id)
{
    RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id)
{
    if (documents_.count(document_id) == 0)
        return;

    document_ids_.erase(document_id);
    documents_.erase(document_id);

    for (auto& [str_view, _] : GetWordFrequencies(document_id))
    {
        std::string word{ str_view };
        word_to_document_freqs_.at(word).erase(document_id);

        if (word_to_document_freqs_.at(word).empty())
            word_to_document_freqs_.erase(word);
    }
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id)
{
    if ((document_id < 0) || (documents_.count(document_id) == 0))
    {
        throw std::invalid_argument("Invalid document_id");
    }
    std::set<std::string_view> doc_words;

    auto doc_frequencies = GetWordFrequencies(document_id);

    std::transform(doc_frequencies.begin(), doc_frequencies.end(),
        std::inserter(doc_words, doc_words.end()), [](auto word) { return word.first; });

    std::for_each(std::execution::par,
        doc_words.begin(),
        doc_words.end(),
        [this, document_id]
    (const auto& word)
        {  this->word_to_document_freqs_.at(std::string{ word }).erase(document_id);
});

    documents_.erase(document_id);
    document_ids_.erase(document_id);
}


/*********************************************************
 ***************   Private class members   ***************
 *********************************************************/

bool SearchServer::IsStopWord(const std::string_view word) const
{
    return stop_words_.count(std::string{ word }) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(std::string_view text) const
{
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(std::string{ text }))
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

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word) const
{
    if (word.empty())
    {
        throw std::invalid_argument("Query word is empty");
    }

    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
    {
        throw std::invalid_argument("");
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view text) const
{
    Query result;

    for (auto& word : SplitIntoWordsView(text))
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

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(std::string{ word }).size());
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
        std::cerr << "Ошибка добавления документа " << document_id << ": " << e.what() << std::endl;
    }
}

