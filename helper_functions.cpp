#include "object.h"

#include <vector>

ObjectPtrVector EvaluateListArguments(const ObjectPtrVector& vectorized_list, ContextPtr context) {
    ObjectPtrVector evaluated_vector(vectorized_list.size());
    for (size_t i = 0; i < vectorized_list.size(); ++i) {
        evaluated_vector[i] = EvaluateExpression(vectorized_list[i], context);
    }
    return evaluated_vector;
}

ObjectPtr EvaluateExpression(ObjectPtr ast, ContextPtr context) {
    if (Is<Number>(ast) || Is<BooleanSymbol>(ast) || Is<Symbol>(ast)) {
        return ast->Evaluate(context);
    } else if (Is<Cell>(ast)) {
        ObjectPtr head = As<Cell>(ast)->GetFirst();
        ObjectPtr tail = As<Cell>(ast)->GetSecond();
        ObjectPtr symbol_evaluated = EvaluateExpression(head, context);
        if (!symbol_evaluated) {
            throw RuntimeError("First element of pair must be applicable.");
        }
        ObjectPtr evaluation_result = symbol_evaluated->Apply(ListToVector(tail));
        return evaluation_result;
    } else {
        throw RuntimeError("Cannot evaluate AST.");
    }
}

ObjectPtrVector ListToVector(ObjectPtr cell) {
    ObjectPtrVector result;
    if (!Is<Cell>(cell) && cell != nullptr) {
        result.push_back(cell);
        return result;
    }
    while (Is<Cell>(cell)) {
        ObjectPtr first_elem = As<Cell>(cell)->GetFirst();
        result.push_back(first_elem);
        cell = As<Cell>(cell)->GetSecond();
    }
    if (cell != nullptr) {
        result.push_back(cell);
    }
    return result;
}

ObjectPtrVector CloneObjectPtrVector(const ObjectPtrVector& list) {
    ObjectPtrVector cloned_vector(list.size());
    for (size_t i = 0; i < cloned_vector.size(); ++i) {
        cloned_vector[i] = list[i]->Clone();
    }
    return cloned_vector;
}

void ThrowIfZeroDivisors(const ObjectPtrVector& list) {
    for (size_t i = 1; i < list.size(); ++i) {
        if (As<Number>(list[i])->GetValue() == 0) {
            throw RuntimeError("Division by zero.");
        }
    }
}

void ThrowIfWrongNumberOfArguments(size_t number, const ObjectPtrVector& list,
                                   const std::string& func_name) {
    if (list.size() != number) {
        throw RuntimeError(func_name + " takes only " + std::to_string(number) + " argument.");
    }
}

void ValidateArgumentsForListTailAndRef(const ObjectPtrVector& list) {
    ThrowIfWrongNumberOfArguments(2, list, "List-ref (tail)");
    if (!As<BooleanSymbol>(CheckIfList(list[0]))->IsTrue()) {
        throw RuntimeError("First operand for list-ref (tail) must be list.");
    }
    ThrowIfMismatchOperandType<Number>(1, list,
                                       "Second operand for list-ref (tail) must be number.");
    int64_t required_number = As<Number>(list[1])->GetValue();
    if (required_number < 0) {
        throw RuntimeError("Second operand for list-ref (tail) must be non-negative.");
    }
}

ObjectPtr CheckIfList(ObjectPtr ptr) {
    ObjectPtr cell = ptr;
    if (!Is<Cell>(cell)) {
        if (!cell) {
            return Heap::Instance().Make<BooleanSymbol>("#t");
        } else {
            return Heap::Instance().Make<BooleanSymbol>("#f");
        }
    }
    while (Is<Cell>(As<Cell>(cell)->GetSecond())) {
        cell = As<Cell>(cell)->GetSecond();
    }
    return (!As<Cell>(cell)->GetSecond()) ? Heap::Instance().Make<BooleanSymbol>("#t")
                                          : Heap::Instance().Make<BooleanSymbol>("#f");
}