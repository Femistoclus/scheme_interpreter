#pragma once

#include <sstream>

#include "parser.h"

class Interpreter {
public:
    Interpreter() {
        ScopePtr global_scope = Heap::Instance().Make<Scope>(kValidFunctionsMap);
        context_ = Heap::Instance().Make<Context>();
        context_->AddScope(global_scope);
        Heap::Instance().SetRoot(context_);
    }
    std::string Run(const std::string& expression);

private:
    std::string SerializeAST(ObjectPtr);
    ContextPtr context_;
};
