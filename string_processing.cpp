#include <iostream>
#include <execution>
#include <functional>

#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string& text)
{
    std::vector<std::string> words;
    std::string word;

    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view str)
{
    std::vector<std::string_view> result;
    const size_t pos_end = str.npos;
    while (true)
    {
        size_t space = str.find(' ', 0);
        result.push_back(space == pos_end ? str.substr(0) : str.substr(0, space));
        if (space == pos_end)
        {
            break;
        }
        else
        {
            str.remove_prefix(space + 1);
        }
    }

    return result;
}

int ReadLineWithNumber()
{
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

std::string ReadLine()
{
    std::string s;
    std::getline(std::cin, s);
    return s;
}

bool IsValidWord(const std::string_view word)
{
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
}

bool IsValidWord(const std::string& word)
{
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
}