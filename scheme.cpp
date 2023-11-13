#include "scheme.h"

std::string Interpreter::Run(const std::string& expression) {
    std::stringstream expression_stream{expression};
    Tokenizer tokenizer{&expression_stream};
    ObjectPtr ast = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Wrong syntax!");
    }
    ObjectPtr evaluated_ast = EvaluateExpression(ast, context_);
    std::string serialized_result = SerializeAST(evaluated_ast);
    Heap::Instance().MarkAndSweep();
    return serialized_result;
}

std::string Interpreter::SerializeAST(ObjectPtr ast) {
    if (!ast) {
        return kEmptyListString;
    }
    return ast->Serialize();
}