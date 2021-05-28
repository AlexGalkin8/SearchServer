#include "search_server.h"
#include "read_input_functions.h"
#include "process_queries.h"

#include <execution>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
    SearchServer search_server("and with"s);
    std::vector<string> docs =
    {
       "white cat and yellow hat"s,
       "curly cat curly tail"s,
       "nasty dog with big eyes"s,
       "nasty pigeon john"s
    };

    for (size_t id = 0; id < docs.size(); id++)
    {
        search_server.AddDocument(id, docs[id], DocumentStatus::ACTUAL, { 1, 2 });
    }

    cout << "ACTUAL by default:"s << endl;
    // ���������������� ������
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    // ���������������� ������
    for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    // ������������ ������
    for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }

    return 0;
}