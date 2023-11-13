#include "object.h"

// Realization of methods for working with heap

void Heap::MarkAndSweep() {
    root_->Mark();
    for (size_t i = 0; i < heap_.size(); ++i) {
        while (i < heap_.size() && !heap_[i]->IsConnected()) {
            std::swap(heap_[i], heap_.back());
            delete heap_.back();
            heap_.pop_back();
        }
        if (i < heap_.size()) {
            heap_[i]->ResetMarkFlag();
        }
    }
}

ObjectPtr Symbol::Evaluate(ContextPtr context) {
    if (context->Contains(name_)) {
        ObjectPtr eval_symbol = context->Get(name_);
        eval_symbol->SetContext(context);
        return eval_symbol;
    } else {
        throw NameError("There are no such name.");
    }
}

std::string Cell::Serialize() {
    std::string result;
    result.push_back(OpenBracketChar);
    result += (GetFirst()) ? GetFirst()->Serialize() : kEmptyListString;
    ObjectPtr cell = GetSecond();
    while (Is<Cell>(cell)) {
        result.push_back(SpaceChar);
        result += (cell) ? As<Cell>(cell)->GetFirst()->Serialize() : kEmptyListString;
        cell = As<Cell>(cell)->GetSecond();
    }
    if (cell) {
        result += " . " + cell->Serialize();
    }
    result.push_back(CloseBracketChar);
    return result;
}

// Some predicate functions' realization.

ObjectPtr NullPredicateFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, vectorized_list, "Predicate");
    return (!eval_list[0]) ? Heap::Instance().Make<BooleanSymbol>(kTrueTokenName)
                           : Heap::Instance().Make<BooleanSymbol>(kFalseTokenName);
}

ObjectPtr ListPredicateFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, eval_list, "Predicate");
    return CheckIfList(eval_list[0]);
}

// Some pair functions' realization.

ObjectPtr ConsFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(2, eval_list, "Cons");
    return Heap::Instance().Make<Cell>(eval_list[0], eval_list[1]);
}

ObjectPtr CarFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, eval_list, "Car");
    ThrowIfMismatchOperandsType<Cell>(eval_list, "Operand must be cell.");
    return As<Cell>(eval_list[0])->GetFirst();
}

ObjectPtr CdrFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, eval_list, "Cdr");
    ThrowIfMismatchOperandsType<Cell>(eval_list, "Operand must be cell.");
    return As<Cell>(eval_list[0])->GetSecond();
}

// Some list functions' realization.

ObjectPtr ToListFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    if (eval_list.empty()) {
        return nullptr;
    }
    ObjectPtr last_cell = Heap::Instance().Make<Cell>(eval_list[eval_list.size() - 1], nullptr);
    for (int64_t i = eval_list.size() - 2; i >= 0; --i) {
        last_cell = Heap::Instance().Make<Cell>(eval_list[i], last_cell);
    }
    return last_cell;
}

ObjectPtr ListRefFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ValidateArgumentsForListTailAndRef(eval_list);
    ObjectPtr cell = eval_list[0];
    int64_t required_number = As<Number>(eval_list[1])->GetValue();
    if (!cell) {
        throw RuntimeError("Index for list-ref must less than list size.");
    }
    int64_t count = 0;
    while (count != required_number && Is<Cell>(As<Cell>(cell)->GetSecond())) {
        cell = As<Cell>(cell)->GetSecond();
        ++count;
    }
    if (count != required_number) {
        throw RuntimeError("Index for list-ref must less than list size.");
    }
    return As<Cell>(cell)->GetFirst();
}

ObjectPtr ListTailFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ValidateArgumentsForListTailAndRef(eval_list);
    ObjectPtr cell = eval_list[0];
    int64_t required_number = As<Number>(eval_list[1])->GetValue();
    int64_t count = 0;
    while (count != required_number && cell && Is<Cell>(As<Cell>(cell)->GetSecond())) {
        cell = As<Cell>(cell)->GetSecond();
        ++count;
    }
    if (count == required_number - 1) {
        return nullptr;
    } else if (count == required_number) {
        return cell;
    } else {
        throw RuntimeError("Index for list-tail must less or equal to list size.");
    }
}

// Some other functions' realization.

ObjectPtr AbsFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, eval_list, "Abs");
    ThrowIfMismatchOperandsType<Number>(eval_list, "Operands must be numbers.");
    int64_t result = std::abs(As<Number>(eval_list[0])->GetValue());
    return Heap::Instance().Make<Number>(result);
}

ObjectPtr NegFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
    ThrowIfWrongNumberOfArguments(1, eval_list, "Not");
    if (auto p = As<BooleanSymbol>(eval_list[0])) {
        return (p->IsTrue()) ? Heap::Instance().Make<BooleanSymbol>(kFalseTokenName)
                             : Heap::Instance().Make<BooleanSymbol>(kTrueTokenName);
    }
    return Heap::Instance().Make<BooleanSymbol>(kFalseTokenName);
}

ObjectPtr QuoteFunction::Apply(const ObjectPtrVector &vectorized_list) {
    ThrowIfWrongNumberOfArguments(1, vectorized_list, "Quote");
    return vectorized_list[0];
}

// Some logic functions' realization.

ObjectPtr AndFunction::Apply(const ObjectPtrVector &vectorized_list) {
    for (size_t i = 0; i < vectorized_list.size(); ++i) {
        ObjectPtr eval_ptr = EvaluateExpression(vectorized_list[i], context_);
        if (auto p = As<BooleanSymbol>(eval_ptr)) {
            if (!p->IsTrue()) {
                return eval_ptr;
            }
        }
        if (i == vectorized_list.size() - 1) {
            return eval_ptr;
        }
    }
    return Heap::Instance().Make<BooleanSymbol>(kTrueTokenName);
}

ObjectPtr OrFunction::Apply(const ObjectPtrVector &vectorized_list) {
    for (size_t i = 0; i < vectorized_list.size(); ++i) {
        ObjectPtr eval_ptr = EvaluateExpression(vectorized_list[i], context_);
        if (!Is<BooleanSymbol>(eval_ptr) || As<BooleanSymbol>(eval_ptr)->IsTrue()) {
            return eval_ptr;
        }
        if (i == vectorized_list.size() - 1) {
            return eval_ptr;
        }
    }
    return Heap::Instance().Make<BooleanSymbol>(kFalseTokenName);
}

// Define & set's realization

ObjectPtr DefineFunction::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() < 2) {
        throw SyntaxError("Wrong syntax for define.");
    }
    if (Is<Symbol>(vectorized_list[0])) {
        std::string var_name = As<Symbol>(vectorized_list[0])->GetName();
        if (vectorized_list.size() != 2) {
            throw SyntaxError("Wrong syntax for define.");
        }
        context_->Define(var_name, EvaluateExpression(vectorized_list[1], context_));
    } else if (Is<Cell>(vectorized_list[0])) {
        ObjectPtrVector signature =
            ListToVector(vectorized_list[0]);  // fn - arg1 - arg2 - arg3 - ...
        std::string func_name = As<Symbol>(signature[0])->GetName();
        ObjectPtrVector args(signature.size() - 1);
        for (size_t i = 1; i < signature.size(); ++i) {
            args[i - 1] = signature[i];
        }
        ObjectPtrVector body(vectorized_list.size() - 1);
        for (size_t i = 1; i < vectorized_list.size(); ++i) {
            body[i - 1] = vectorized_list[i];
        }
        context_->Define(func_name, Heap::Instance().Make<LambdaFunction>(args, body, context_));
    } else {
        throw SyntaxError("Wrong syntax for define.");
    }
    return nullptr;
}

ObjectPtr SetFunction::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() != 2) {
        throw SyntaxError("Wrong syntax for set.");
    }
    ThrowIfMismatchOperandType<Symbol>(0, vectorized_list,
                                       "First argument for define must be a symbol");
    std::string var_name = As<Symbol>(vectorized_list[0])->GetName();
    if (!context_->Contains(var_name)) {
        throw NameError("Variable for set must be defined before.");
    }
    context_->Change(var_name, EvaluateExpression(vectorized_list[1], context_));
    return nullptr;
}

ObjectPtr SetCar::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() != 2) {
        throw SyntaxError("Wrong syntax for set-car.");
    }
    ObjectPtr eval_first_arg = EvaluateExpression(vectorized_list[0], context_);
    if (!Is<Cell>(eval_first_arg)) {
        throw RuntimeError("First operand for set-car must be a cell.");
    }
    As<Cell>(eval_first_arg)->SetFirst(EvaluateExpression(vectorized_list[1], context_));
    return nullptr;
}

ObjectPtr SetCdr::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() != 2) {
        throw SyntaxError("Wrong syntax for set-cdr.");
    }
    ObjectPtr eval_first_arg = EvaluateExpression(vectorized_list[0], context_);
    if (!Is<Cell>(eval_first_arg)) {
        throw RuntimeError("First operand for set-car must be a cell.");
    }
    As<Cell>(eval_first_arg)->SetSecond(EvaluateExpression(vectorized_list[1], context_));
    return nullptr;
}

// IF's realization

ObjectPtr IfFunction::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() == 2) {
        ObjectPtr evaluated_condition = EvaluateExpression(vectorized_list[0], context_);
        if (!Is<BooleanSymbol>(evaluated_condition) ||
            As<BooleanSymbol>(evaluated_condition)->IsTrue()) {
            return EvaluateExpression(vectorized_list[1], context_);
        }
        return nullptr;
    } else if (vectorized_list.size() == 3) {
        ObjectPtr evaluated_condition = EvaluateExpression(vectorized_list[0], context_);
        if (!Is<BooleanSymbol>(evaluated_condition) ||
            As<BooleanSymbol>(evaluated_condition)->IsTrue()) {
            return EvaluateExpression(vectorized_list[1], context_);
        } else {
            return EvaluateExpression(vectorized_list[2], context_);
        }
    } else {
        throw SyntaxError("Wrong number of arguments for if.");
    }
}

// Lambda's realization

ObjectPtr LambdaDeclaration::Apply(const ObjectPtrVector &vectorized_list) {
    if (vectorized_list.size() < 2) {
        throw SyntaxError("Wrong syntax for lambda declaration.");
    }
    // первый элемент - лист с аргументами
    if (vectorized_list[0] && !Is<Cell>(vectorized_list[0])) {
        throw SyntaxError("Wrong format for list of lambda's arguments.");
    }
    ObjectPtrVector args = ListToVector(vectorized_list[0]);  // аргументы-переменные
    ThrowIfMismatchOperandsType<Symbol>(args, "Args for lambda declaration must be symbols.");
    ObjectPtrVector body(vectorized_list.size() - 1);
    for (size_t i = 1; i < vectorized_list.size(); ++i) {
        body[i - 1] = vectorized_list[i];
    }
    return Heap::Instance().Make<LambdaFunction>(args, body, context_);
}

LambdaFunction::LambdaFunction(const ObjectPtrVector &args, const ObjectPtrVector &body,
                               ContextPtr context)
    : args_(args), body_(body) {
    captured_context_ = Heap::Instance().Make<Context>(*context);
    AddDependency(captured_context_);
    current_context_ = captured_context_;
    for (size_t i = 0; i < args.size(); ++i) {
        AddDependency(args[i]);
    }
    for (size_t i = 0; i < body.size(); ++i) {
        AddDependency(body[i]);
    }
}

ObjectPtr LambdaFunction::Apply(const ObjectPtrVector &vectorized_list) {
    captured_context_->AddEmptyScope();
    if (args_.size() != vectorized_list.size()) {
        throw RuntimeError("Wrong number of args for lambda call.");
    }
    for (size_t i = 0; i < args_.size(); ++i) {
        captured_context_->Define(As<Symbol>(args_[i])->GetName(),
                                  EvaluateExpression(vectorized_list[i], current_context_));
    }
    for (size_t i = 0; i < body_.size() - 1; ++i) {
        EvaluateExpression(body_[i], captured_context_);
    }
    ObjectPtr ans = EvaluateExpression(body_[body_.size() - 1], captured_context_);
    captured_context_->PopScope();
    return ans;
}
