#pragma once

#include "search_server.h"

/* @brief Find and remove duplicates function.
 * @param [in] search_server - the server from which you want to remove duplicates.
 * @param [out] search_server - the server from which the duplicates were removed. */
void RemoveDuplicates(SearchServer& search_server);
