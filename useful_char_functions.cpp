#include "tokenizer.h"

bool IsOpenBracket(const char c) {
    return c == OpenBracketChar;
}

bool IsCloseBracket(const char c) {
    return c == CloseBracketChar;
}

bool IsDot(const char c) {
    return c == DotChar;
}

bool IsMinus(const char c) {
    return c == MinusChar;
}

bool IsPlus(const char c) {
    return c == PlusChar;
}

bool IsQuote(const char c) {
    return c == QuoteChar;
}

bool IsAsterix(const char c) {
    return c == AsterixChar;
}

bool IsSlash(const char c) {
    return c == SlashChar;
}

bool IsAlphabet(const char c) {
    return std::isalpha(c);
}

bool IsDigit(const char c) {
    return std::isdigit(c);
}

bool IsSpace(const char c) {
    return std::isspace(c);
}

bool IsComparisonSign(const char c) {
    return LessSignChar <= c && c <= GreaterSignChar;
}

bool IsStartOfSymbol(const char c) {
    return IsAlphabet(c) || IsComparisonSign(c) || IsAsterix(c) || IsSlash(c) || c == PoundChar;
}

bool IsPartOfSymbol(const char c) {
    return IsStartOfSymbol(c) || IsDigit(c) || IsMinus(c) || c == QuestionSignChar ||
           c == ExclamationSignChar;
}