#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>

std::string generate_session_id();
std::string process_request(const std::string &request, const std::string &session_id);

#endif
