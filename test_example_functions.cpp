#include "test_example_functions.h"
#include "search_server.h"

#include <iostream>
#include <cmath>

using namespace std::string_literals;


namespace search_server_testing
{
    using namespace test_tools;

    SearchServer GetSearchServer()
    {
        SearchServer search_server("и в на"s);

        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

        return search_server;
    }

    void TestExcludeStopWordsFromAddedDocumentContent()
    {
        const int doc_id = 42;
        const std::string content = "cat in the city";
        const std::vector<int> ratings = { 1, 2, 3 };
        {
            SearchServer server(""s);
            server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
            const auto found_docs = server.FindTopDocuments("in");
            ASSERT_EQUAL(found_docs.size(), 1u);
            const Document& doc0 = found_docs[0];
            ASSERT_EQUAL(doc0.id, doc_id);
        }

        {
            SearchServer server("in the"s);
            server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
            ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents");
        }
    }

    void TestDocumentAdd()
    {
        const int control_id = 1;

        SearchServer search_server = GetSearchServer();

        const Document verification_document = search_server.FindTopDocuments("пушистый ухоженный кот")[0];

        ASSERT_EQUAL_HINT(verification_document.id, control_id, "Incorrect addition of documents");
    }

    void TestSupportMinusWords()
    {
        SearchServer search_server = GetSearchServer();

        const Document verification_document = search_server.FindTopDocuments("пушистый ухоженный кот")[0];
        const int control_id = 1;

        ASSERT_HINT(search_server.FindTopDocuments("-ухоженный ухоженный -кот").empty(),
            "Negative keywords should not be included in search results");
        ASSERT_EQUAL_HINT(verification_document.id, control_id,
            "The absence of a valid document as a result of the request");
    }

    void TestMatchingDocuments()
    {
        SearchServer search_server = GetSearchServer();

        std::tuple<std::vector<std::string>, DocumentStatus> match_document =
            search_server.MatchDocument("-ухоженный ухоженный -кот", 0);

        ASSERT_HINT(std::get<0>(match_document).empty(),
            "Incorrect matching of documents to the search query");
    }

    void TestSortedByRelevance()
    {
        const std::string error_message("Incorrect sorting by relevance");

        const double control_relevance0 = 0.866434;
        const double control_relevance1 = 0.173287;
        const double control_relevance2 = 0.173287;

        SearchServer search_server = GetSearchServer();

        const std::vector<Document> verification_documents = search_server.FindTopDocuments("пушистый ухоженный кот");

        const double relevance0 = std::round(verification_documents[0].relevance * 1000000) / 1000000;
        const double relevance1 = std::round(verification_documents[1].relevance * 1000000) / 1000000;
        const double relevance2 = std::round(verification_documents[2].relevance * 1000000) / 1000000;

        ASSERT_HINT(std::fabs(relevance0 - control_relevance0) < std::numeric_limits<double>::epsilon(), error_message);
        ASSERT_HINT(std::fabs(relevance1 - control_relevance1) < std::numeric_limits<double>::epsilon(), error_message);
        ASSERT_HINT(std::fabs(relevance2 - control_relevance2) < std::numeric_limits<double>::epsilon(), error_message);
    }

    void TestDocumentRatingCalculation()
    {
        const std::string error_message("Incorrect rating calculation");

        const int control_rating0 = 5;
        const int control_rating1 = 2;
        const int control_rating2 = -1;

        SearchServer search_server = GetSearchServer();

        std::vector<Document> verification_documents = search_server.FindTopDocuments("пушистый ухоженный кот");

        ASSERT_HINT(verification_documents[0].rating == control_rating0, error_message);
        ASSERT_HINT(verification_documents[1].rating == control_rating1, error_message);
        ASSERT_HINT(verification_documents[2].rating == control_rating2, error_message);
    }

    void TestUserPredicate()
    {
        const std::string error_message("Incorrect user predicate");

        const int control_id0 = 0;
        const int control_id1 = 2;

        SearchServer search_server = GetSearchServer();

        const std::vector<Document> verification_documents = search_server.FindTopDocuments("пушистый ухоженный кот"s,
            [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });

        ASSERT_HINT(verification_documents[0].id == control_id0, error_message);
        ASSERT_HINT(verification_documents[1].id == control_id1, error_message);
    }

    void TestDocumentStatus()
    {
        const int control_id = 3;

        SearchServer search_server = GetSearchServer();

        const Document verification_document =
            search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0];

        ASSERT_HINT(verification_document.id == control_id, "Incorrect document status");
    }

    void TestRelevanceCalculations()
    {
        const std::string error_message("Incorrect calculating relevance");

        const double control_relevance0 = 0.866434;
        const double control_relevance1 = 0.173287;
        const double control_relevance2 = 0.173287;

        SearchServer search_server = GetSearchServer();

        std::vector<Document> verification_document = search_server.FindTopDocuments("пушистый ухоженный кот");

        const double relevance0 = std::round(verification_document[0].relevance * 1000000) / 1000000;
        const double relevance1 = std::round(verification_document[1].relevance * 1000000) / 1000000;
        const double relevance2 = std::round(verification_document[2].relevance * 1000000) / 1000000;

        ASSERT_EQUAL_HINT(relevance0, control_relevance0, error_message);
        ASSERT_EQUAL_HINT(relevance1, control_relevance1, error_message);
        ASSERT_EQUAL_HINT(relevance2, control_relevance2, error_message);
    }


    void TestSearchServer()
    {
        RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
        RUN_TEST(TestDocumentAdd);
        RUN_TEST(TestSupportMinusWords);
        RUN_TEST(TestMatchingDocuments);
        RUN_TEST(TestSortedByRelevance);
        RUN_TEST(TestDocumentRatingCalculation);
        RUN_TEST(TestUserPredicate);
        RUN_TEST(TestDocumentStatus);
        RUN_TEST(TestRelevanceCalculations);
    }
}
