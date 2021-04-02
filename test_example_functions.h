#pragma once

#include "search_server.h"

#include <iostream>

using namespace std::string_literals;

namespace search_server_testing
{
    void TestExcludeStopWordsFromAddedDocumentContent();
    void TestDocumentAdd();
    void TestSupportMinusWords();
    void TestMatchingDocuments();
    void TestSortedByRelevance();
    void TestDocumentRatingCalculation();
    void TestUserPredicate();
    void TestDocumentStatus();
    void TestRelevanceCalculations();

    void TestSearchServer();
}


namespace test_tools
{
    template<typename type0>
    std::ostream& Print(std::ostream& out, type0 container)
    {
        bool spce = true;
        for (const auto element : container)
        {
            if (spce)
                out << element;
            else
                out << ", "s << element;

            spce = false;
        }
        return out;
    }

    template<typename pair_type1, typename pair_type2>
    std::ostream& operator<<(std::ostream& out, const std::pair<pair_type1, pair_type2>& container)
    {
        out << container.first << ": " << container.second;
        return out;
    }

    template<typename type1>
    std::ostream& operator<<(std::ostream& out, const std::vector<type1>& container)
    {
        out << '[';
        Print(out, container);
        out << ']';
        return out;
    }

    template<typename type2>
    std::ostream& operator<<(std::ostream& out, const std::set<type2>& container)
    {
        out << '{';
        Print(out, container);
        out << '}';
        return out;
    }

    template<typename Key, typename Value>
    std::ostream& operator<<(std::ostream& out, const std::map<Key, Value>& container)
    {
        out << '{';
        Print(out, container);
        out << '}';
        return out;
    }

    template <typename T, typename U>
    void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str,
        const std::string& file, const std::string& func, unsigned line, const std::string& hint)
    {
        if (t != u)
        {
            std::cout << std::boolalpha;
            std::cout << file << "("s << line << "): "s << func << ": "s;
            std::cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
            std::cout << t << " != "s << u << "."s;
            if (!hint.empty())
            {
                std::cout << " Hint: "s << hint;
            }
            std::cout << std::endl;
            abort();
        }
    }

    template <typename T, typename U>
    void AssertImpl(const T& t, const U& u, const std::string& t_str, const std::string& file,
        const std::string& func, unsigned line, const std::string& hint = "")
    {
        if (t != u)
        {
            std::cout << std::boolalpha;
            std::cout << file << "("s << line << "): "s << func << ": "s;
            std::cout << "ASSERT("s << t_str << ") failed.";
            if (!hint.empty())
            {
                std::cout << " Hint: "s << hint;
            }
            std::cout << std::endl;
            std::abort();
        }
    }

    template <typename F>
    void RunTestImpl(const F func_test, const std::string& func)
    {
        func_test();
        std::cerr << func << " OK" << std::endl;
    }

    #define ASSERT(expr) AssertImpl(expr, true, #expr, __FILE__, __FUNCTION__, __LINE__)

    #define ASSERT_HINT(expr, hint) AssertImpl(expr, true, #expr, __FILE__, __FUNCTION__, __LINE__, hint)

    #define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

    #define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

    #define RUN_TEST(func) RunTestImpl((func), #func)
}