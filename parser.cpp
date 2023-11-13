#include "parser.h"

ObjectPtr Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("No tokens to read.");
    }
    Token next = tokenizer->GetToken();
    tokenizer->Next();
    auto& heap_ref = Heap::Instance();
    size_t index_of_cur_token = next.index();
    if (index_of_cur_token == BRACKET_TOKEN) {
        if (std::get<BracketToken>(next) == BracketToken::CLOSE) {
            throw SyntaxError("Close bracket in the start of the expression.");
        }
        Cell* list_ptr = ReadList(tokenizer);
        if (!AtCloseBracket(tokenizer)) {
            throw SyntaxError("Wrong syntax! Not enough close brackets.");
        }
        tokenizer->Next();
        return list_ptr;
    } else if (index_of_cur_token == CONSTANT_TOKEN) {
        return heap_ref.Make<Number>(std::get<ConstantToken>(next));
    } else if (index_of_cur_token == SYMBOL_TOKEN) {
        return SpecifySymbolObject(std::get<SymbolToken>(next));
    } else if (index_of_cur_token == QUOTE_TOKEN) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Wrong syntax for quote.");
        }
        return heap_ref.Make<Cell>(heap_ref.Make<Symbol>("quote"),
                                   heap_ref.Make<Cell>(Read(tokenizer), nullptr));
    } else if (index_of_cur_token == DOT_TOKEN) {
        throw SyntaxError("Wrong syntax! Probably dot in a wrong place.");
    } else {
        throw SyntaxError("Wrong syntax!");
    }
}

bool AtCloseBracket(Tokenizer* tokenizer) {
    if (tokenizer->GetToken().index() == BRACKET_TOKEN) {
        return (std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE);
    }
    return false;
}

Cell* ReadList(Tokenizer* tokenizer) {
    if (AtCloseBracket(tokenizer)) {
        return nullptr;
    }
    ObjectPtr first_elem = Read(tokenizer);
    Token cur_token = tokenizer->GetToken();
    if (cur_token.index() == BRACKET_TOKEN) {
        if (std::get<BracketToken>(cur_token) == BracketToken::CLOSE) {
            return Heap::Instance().Make<Cell>(first_elem, nullptr);
        }
    }
    if (cur_token.index() == DOT_TOKEN) {
        tokenizer->Next();
        return Heap::Instance().Make<Cell>(first_elem, Read(tokenizer));
    } else {
        return Heap::Instance().Make<Cell>(first_elem, ReadList(tokenizer));
    }
}

ObjectPtr SpecifySymbolObject(const SymbolToken& symbol_token) {
    std::string symbol_name = symbol_token.GetName();
    if (symbol_name == kFalseTokenName || symbol_name == kTrueTokenName) {
        return Heap::Instance().Make<BooleanSymbol>(symbol_name);
    }
    return Heap::Instance().Make<Symbol>(symbol_name);
}