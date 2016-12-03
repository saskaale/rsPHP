#pragma once
#include "ast.h"
#include "parser.hpp"

namespace Parser
{

void parseFile(FILE *file);
void parseString(const char *str);

} // namespace Parser
