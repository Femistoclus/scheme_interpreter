#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <string>
#include <cctype>

#include "error.h"

enum AllowedChars : char {
    OpenBracketChar = '(',
    CloseBracketChar = ')',
    DotChar = '.',
    MinusChar = '-',
    PlusChar = '+',
    QuoteChar = '\'',
    LessSignChar = '<',
    GreaterSignChar = '>',
    PoundChar = '#',
    AsterixChar = '*',
    SlashChar = '/',
    ExclamationSignChar = '!',
    QuestionSignChar = '?',
    SpaceChar = ' ',
};

bool IsOpenBracket(const char c);

bool IsCloseBracket(const char c);

bool IsDot(const char c);

bool IsMinus(const char c);

bool IsPlus(const char c);

bool IsQuote(const char c);

bool IsAsterix(const char c);

bool IsSlash(const char c);

bool IsAlphabet(const char c);

bool IsDigit(const char c);

bool IsSpace(const char c);

bool IsComparisonSign(const char c);

bool IsStartOfSymbol(const char c);

bool IsPartOfSymbol(const char c);

struct SymbolToken {
    const std::string& GetName() const {
        return name;
    }
    bool operator==(const SymbolToken& other) const;
    std::string name;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int64_t value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd() {
        return is_end_;
    }

    void Next();

    Token GetToken() {
        return last_processed_token_;
    }

private:
    bool is_end_ = false;
    std::istream* token_stream_;
    Token last_processed_token_;
    void ProcessPlusMinusToken(char cur_char);
    void ProcessConstantToken(char cur_char);
    void ProcessSymbolToken(char cur_char);
};