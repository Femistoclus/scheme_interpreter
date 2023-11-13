#pragma once

#include "object.h"
#include "error.h"
#include "tokenizer.h"

#include <memory>

enum TokenNames : size_t {
    CONSTANT_TOKEN,
    BRACKET_TOKEN,
    SYMBOL_TOKEN,
    QUOTE_TOKEN,
    DOT_TOKEN,
};

const std::string kQuoteSymbolName = "quote";

///////////////////////////////////////////////////////////////////////////////

// Function to specify concrete types of tokens

Object* SpecifySymbolObject(const SymbolToken& symbol_token);

///////////////////////////////////////////////////////////////////////////////

// Parser functions

Object* Read(Tokenizer* tokenizer);
Cell* ReadList(Tokenizer* tokenizer);
bool AtCloseBracket(Tokenizer* tokenizer);