#pragma once

#include <iostream>

class parse_expect : public std::runtime_error
{
public:
    explicit parse_expect(const std::string &s) : std::runtime_error(s) {}
};
