#pragma once

#include <string>
#include <vector>
#include <set>
#include <string_view>
#include <algorithm>

std::vector<std::string> SplitIntoWords(const std::string& text);

std::vector<std::string_view> SplitIntoWordsView(const std::string_view text);

int ReadLineWithNumber();

std::string ReadLine();

/* @brief Check the absence of special characters in the word.
 * @param word - word to check.
 * @return Correct (true) or incorrect (false). */
bool IsValidWord(const std::string_view word);

/* @brief Check the absence of special characters in the word.
 * @param word - word to check.
 * @return Correct (true) or incorrect (false). */
bool IsValidWord(const std::string& word);

/* @brief Method of composing unique words.
 * @param strings - container contains non-unique words.
 * @return Unique words */
template <class StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings)
{
    for (const auto& str : strings)
    {
        if (!IsValidWord(str))
            throw std::invalid_argument("Stop words contain invalid characters");
    }

    std::set<std::string, std::less<>> non_empty_strings;

    for (const auto& str : strings)
    {
        if (!str.empty())
        {
            non_empty_strings.insert(std::string{ str });
        }
    }
    return non_empty_strings;
}
