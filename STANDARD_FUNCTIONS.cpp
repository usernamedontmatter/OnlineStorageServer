#pragma once

#include "HEADERS.h"

static std::vector<std::string>* split(const std::string* text, char delimiter = ' ') {
    auto arr = new std::vector<std::string>();
    size_t left = 0, right = text->find(delimiter, left);

    while (right != std::string::npos) {
        arr->push_back(text->substr(left, right - left));
        left = right + 1;
        right = text->find(delimiter, left);
    }
    arr->push_back(text->substr(left, text->size() - left));

    return arr;
}
static std::vector<std::string>* split_with_delimiter_removing(const std::string* text, const char delimiter = ' ') {
    auto arr = new std::vector<std::string>();
    size_t left = 0, right = text->find(delimiter, left);

    while (right != std::string::npos) {
        if (right != left) {
            arr->push_back(text->substr(left, right - left));
        }
        left = right + 1;
        right = text->find(delimiter, left);
    }
    if (left != text->size()) {
        arr->push_back(text->substr(left, right - left));
    }

    return arr;
}

static bool is_number(const std::string& text) {
    if (text.empty()) return false;

    for (const char letter : text) {
        if (not isdigit(letter)) return false;
    }

    return true;
}

static long long div_with_round_up(const long long num1, const long long num2) {
    return num1/num2 + (num1%num2 != 0);
}