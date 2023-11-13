#include "tokenizer.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) {
    token_stream_ = in;
    Next();
}

void Tokenizer::ProcessPlusMinusToken(char cur_char) {
    std::string cur_token_string;
    cur_token_string.push_back(cur_char);
    while (IsDigit(token_stream_->peek())) {
        cur_char = token_stream_->get();
        cur_token_string.push_back(cur_char);
    }
    if (cur_token_string.size() == 1) {
        last_processed_token_ = SymbolToken{cur_token_string};
    } else {
        last_processed_token_ = ConstantToken{std::stoi(cur_token_string)};
    }
}

void Tokenizer::ProcessConstantToken(char cur_char) {
    std::string cur_token_string;
    cur_token_string.push_back(cur_char);
    while (IsDigit(token_stream_->peek())) {
        cur_char = token_stream_->get();
        cur_token_string.push_back(cur_char);
    }
    last_processed_token_ = ConstantToken{std::stoll(cur_token_string)};
}

void Tokenizer::ProcessSymbolToken(char cur_char) {
    std::string cur_token_string;
    cur_token_string.push_back(cur_char);
    while (IsPartOfSymbol(token_stream_->peek())) {
        cur_char = token_stream_->get();
        cur_token_string.push_back(cur_char);
    }
    last_processed_token_ = SymbolToken{cur_token_string};
}

void Tokenizer::Next() {
    if (is_end_) {
        throw SyntaxError("Wrong syntax!");
    }
    char cur_char = token_stream_->get();
    while (IsSpace(cur_char)) {
        cur_char = token_stream_->get();
    }
    if (cur_char == std::char_traits<char>::eof()) {
        is_end_ = true;
        return;
    }
    if (IsQuote(cur_char)) {
        last_processed_token_ = QuoteToken{};
    } else if (IsOpenBracket(cur_char)) {
        last_processed_token_ = BracketToken::OPEN;
    } else if (IsCloseBracket(cur_char)) {
        last_processed_token_ = BracketToken::CLOSE;
    } else if (IsDot(cur_char)) {
        last_processed_token_ = DotToken{};
    } else if (IsPlus(cur_char) || IsMinus(cur_char)) {
        ProcessPlusMinusToken(cur_char);
    } else if (IsDigit(cur_char)) {
        ProcessConstantToken(cur_char);
    } else if (IsStartOfSymbol(cur_char)) {
        ProcessSymbolToken(cur_char);
    } else {
        throw SyntaxError("Cannot tokenize. Wrong syntax.");
    }
}