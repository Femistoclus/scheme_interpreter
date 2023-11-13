#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "tokenizer.h"
#include "error.h"

class Object;
using ObjectPtr = Object*;
using ObjectPtrVector = std::vector<ObjectPtr>;

class Scope;
using ScopePtr = Scope*;
using ScopePtrVector = std::vector<ScopePtr>;

class Context;
using ContextPtr = Context*;

const std::string kTrueTokenName = "#t";
const std::string kFalseTokenName = "#f";
const std::string kEmptyListString = "()";

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;

    virtual ObjectPtr Evaluate(ContextPtr) {
        throw RuntimeError("Not implemented.");
    }

    virtual ObjectPtr Apply(const ObjectPtrVector&) {
        throw RuntimeError("Not implemented.");
    }

    virtual ObjectPtr Clone() {
        throw RuntimeError("Not implemented.");
    }

    virtual void SetContext(ContextPtr) {
        throw RuntimeError("Not implemented.");
    }

    virtual std::string Serialize() {
        throw RuntimeError("Not implemented.");
    }

protected:
    void AddDependency(ObjectPtr object) {
        dependencies_.insert(object);
    }

    void RemoveDependency(ObjectPtr object) {
        dependencies_.erase(object);
    }

    void Mark() {
        is_connected_to_root = true;
        for (ObjectPtr dependence : dependencies_) {
            if (dependence && !dependence->IsConnected()) {
                dependence->Mark();
            }
        }
    }

    bool IsConnected() {
        return is_connected_to_root;
    }

    void ResetMarkFlag() {
        is_connected_to_root = false;
    }

protected:
    friend class Heap;
    bool is_connected_to_root = false;
    std::unordered_set<Object*> dependencies_;
};

///////////////////////////////////////////////////////////////////////////////

// Heap class, will tidy memory

class Heap {
public:
    Heap() = default;

    ~Heap() {
        for (ObjectPtr ptr : heap_) {
            delete ptr;
        }
    }

    void SetRoot(ObjectPtr root) {
        root_ = root;
    }

    template <typename ObjectType, typename... Args>
    ObjectType* Make(Args... args) {
        ObjectType* allocated_object = new ObjectType(args...);
        heap_.push_back(allocated_object);
        return allocated_object;
    }

    static Heap& Instance() {
        static Heap head_ref;
        return head_ref;
    }

    void MarkAndSweep();

private:
    ObjectPtrVector heap_;
    ObjectPtr root_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.

template <class T>
T* As(const ObjectPtr& obj) {
    if (auto p = dynamic_cast<T*>(obj)) {
        return p;
    } else {
        return nullptr;
    }
}

template <class T>
bool Is(const ObjectPtr& obj) {
    return As<T>(obj) != nullptr;
}

///////////////////////////////////////////////////////////////////////////////

// Number-like objects

class Number : public Object {
public:
    Number(int64_t value) : value_(value){};

    Number(const ConstantToken& constant_token) : value_(constant_token.value){};

    int64_t GetValue() const {
        return value_;
    }

    ObjectPtr Evaluate(ContextPtr) override {
        return Heap::Instance().Make<Number>(value_);
    }

    std::string Serialize() override {
        return std::to_string(value_);
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<Number>(value_);
    }

    virtual void SetContext(ContextPtr) override{};

private:
    int64_t value_;
};

///////////////////////////////////////////////////////////////////////////////

// Symbol-like objects

class Symbol : public Object {
public:
    Symbol(const std::string& name) : name_(std::move(name)){};

    Symbol(const SymbolToken& symbol_token) : name_(std::move(symbol_token.name)){};

    const std::string& GetName() const {
        return name_;
    }

    ObjectPtr Evaluate(ContextPtr) override;

    virtual std::string Serialize() override {
        return name_;
    }

    virtual void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        ObjectPtr cloned_symbol = Heap::Instance().Make<Symbol>(name_);
        cloned_symbol->SetContext(context_);
        return cloned_symbol;
    }

private:
    std::string name_;
    ContextPtr context_;
};

class BooleanSymbol : public Object {
public:
    BooleanSymbol(const std::string& name) : name_(name) {
        is_true_ = (name_ == kTrueTokenName);
    }
    BooleanSymbol(const SymbolToken& symbol_token) : name_(symbol_token.GetName()) {
        is_true_ = (name_ == kTrueTokenName);
    };

    ObjectPtr Evaluate(ContextPtr) override {
        return Heap::Instance().Make<BooleanSymbol>(name_);
    }

    virtual std::string Serialize() override {
        return name_;
    }

    void SetContext(ContextPtr) override{};

    ObjectPtr Clone() override {
        return Heap::Instance().Make<BooleanSymbol>(name_);
    }

    bool IsTrue() {
        return is_true_;
    }

private:
    std::string name_;
    bool is_true_;
};

///////////////////////////////////////////////////////////////////////////////

// Cell-like objects

class Cell : public Object {
public:
    Cell(ObjectPtr first, ObjectPtr second) : first_(first), second_(second) {
        AddDependency(first);
        AddDependency(second);
    };

    ObjectPtr GetFirst() const {
        return first_;
    }

    ObjectPtr GetSecond() const {
        return second_;
    }

    void SetFirst(ObjectPtr first) {
        RemoveDependency(first_);
        AddDependency(first);
        first_ = first;
    }

    void SetSecond(ObjectPtr second) {
        RemoveDependency(second_);
        AddDependency(second);
        second_ = second;
    }

    ObjectPtr Clone() override {
        ObjectPtr cloned_first = (first_) ? first_->Clone() : nullptr;
        ObjectPtr cloned_second = (second_) ? second_->Clone() : nullptr;
        return Heap::Instance().Make<Cell>(cloned_first, cloned_second);
    }

    void SetContext(ContextPtr) override{};

    std::string Serialize() override;

private:
    ObjectPtr first_;
    ObjectPtr second_;
};

///////////////////////////////////////////////////////////////////////////////

// Declaration of evaluation functions.

ObjectPtr EvaluateExpression(ObjectPtr, ContextPtr);

ObjectPtrVector EvaluateListArguments(const ObjectPtrVector&, ContextPtr);

// Declaration of helper functions.

ObjectPtrVector ListToVector(ObjectPtr cell);

ObjectPtrVector CloneObjectPtrVector [[maybe_unused]] (const ObjectPtrVector& vectorized_list);

template <typename RequiredType>
void ThrowIfMismatchOperandsType(const ObjectPtrVector& vectorized_list,
                                 const std::string& message) {
    for (const auto& ptr : vectorized_list) {
        if (!Is<RequiredType>(ptr)) {
            throw RuntimeError(message);
        }
    }
}

template <typename RequiredType>
void ThrowIfMismatchOperandType(size_t number, const ObjectPtrVector& vectorized_list,
                                const std::string& message) {
    if (!Is<RequiredType>(vectorized_list[number])) {
        throw RuntimeError(message);
    }
}

void ThrowIfZeroDivisors(const ObjectPtrVector&);

void ThrowIfWrongNumberOfArguments(size_t, const ObjectPtrVector&, const std::string&);

void ValidateArgumentsForListTailAndRef(const ObjectPtrVector&);

ObjectPtr CheckIfList(ObjectPtr);

template <typename T>
struct Max {
    T operator()(T a, T b) {
        return (a > b) ? a : b;
    }
};

template <typename T>
struct Min {
    T operator()(T a, T b) {
        return (a > b) ? b : a;
    }
};

///////////////////////////////////////////////////////////////////////////////

// Function-like objects

template <typename Functor>
class BinaryFoldFunction : public Object {
public:
    BinaryFoldFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector& vectorized_list) override {
        ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
        ThrowIfMismatchOperandsType<Number>(eval_list, "Operands must be numbers.");
        if constexpr (std::is_same_v<Functor, std::divides<int64_t>>) {
            ThrowIfZeroDivisors(eval_list);
        }
        if (eval_list.empty()) {
            return ApplyToEmptyList();
        }
        int64_t result = As<Number>(eval_list.front())->GetValue();
        for (size_t i = 1; i < eval_list.size(); ++i) {
            result = Functor()(result, As<Number>(eval_list[i])->GetValue());
        }
        return Heap::Instance().Make<Number>(result);
    }

    ObjectPtr ApplyToEmptyList() {
        if constexpr (std::is_same_v<Functor, std::plus<int64_t>>) {
            return Heap::Instance().Make<Number>(0);
        } else if (std::is_same_v<Functor, std::multiplies<int64_t>>) {
            return Heap::Instance().Make<Number>(1);
        } else {
            throw RuntimeError("Few arguments.");
        }
    }

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<BinaryFoldFunction<Functor>>();
    }

private:
    ContextPtr context_;
};

template <typename Functor>
class MonotonicFunction : public Object {
public:
    MonotonicFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector& vectorized_list) override {
        ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
        ThrowIfMismatchOperandsType<Number>(eval_list, "Operands must be numbers.");
        for (size_t i = 1; i < eval_list.size(); ++i) {
            if (!Functor()(As<Number>(eval_list[i - 1])->GetValue(),
                           As<Number>(eval_list[i])->GetValue())) {
                return Heap::Instance().Make<BooleanSymbol>("#f");
            }
        }
        return Heap::Instance().Make<BooleanSymbol>("#t");
    }

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<MonotonicFunction<Functor>>();
    }

private:
    ContextPtr context_;
};

template <typename Type>
class PredicateFunction : public Object {
public:
    PredicateFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector& vectorized_list) override {
        ObjectPtrVector eval_list = EvaluateListArguments(vectorized_list, context_);
        ThrowIfWrongNumberOfArguments(1, eval_list, "Predicate");
        return (Is<Type>(eval_list[0])) ? Heap::Instance().Make<BooleanSymbol>("#t")
                                        : Heap::Instance().Make<BooleanSymbol>("#f");
    }

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<PredicateFunction<Type>>();
    }

private:
    ContextPtr context_;
};

class NullPredicateFunction : public Object {
public:
    NullPredicateFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<NullPredicateFunction>();
    }

private:
    ContextPtr context_;
};

class ListPredicateFunction : public Object {
public:
    ListPredicateFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<ListPredicateFunction>();
    }

private:
    ContextPtr context_;
};

// Pair functions

class ConsFunction : public Object {
public:
    ConsFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<ConsFunction>();
    }

private:
    ContextPtr context_;
};

class CarFunction : public Object {
public:
    CarFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<CarFunction>();
    }

private:
    ContextPtr context_;
};

class CdrFunction : public Object {
public:
    CdrFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<CdrFunction>();
    }

private:
    ContextPtr context_;
};

// List functions

class ToListFunction : public Object {
public:
    ToListFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<ToListFunction>();
    }

private:
    ContextPtr context_;
};

class ListRefFunction : public Object {
public:
    ListRefFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<ListRefFunction>();
    }

private:
    ContextPtr context_;
};

class ListTailFunction : public Object {
public:
    ListTailFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<ListTailFunction>();
    }

private:
    ContextPtr context_;
};

// Some other functions

class AbsFunction : public Object {
public:
    AbsFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<AbsFunction>();
    }

private:
    ContextPtr context_;
};

class NegFunction : public Object {
public:
    NegFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<NegFunction>();
    }

private:
    ContextPtr context_;
};

class QuoteFunction : public Object {
public:
    QuoteFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr) override{};

    ObjectPtr Clone() override {
        return Heap::Instance().Make<QuoteFunction>();
    }
};

// Logic functions

class AndFunction : public Object {
public:
    AndFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<AndFunction>();
    }

private:
    ContextPtr context_;
};

class OrFunction : public Object {
public:
    OrFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<OrFunction>();
    }

private:
    ContextPtr context_;
};

// Define & set

class DefineFunction : public Object {
public:
    DefineFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<DefineFunction>();
    }

private:
    ContextPtr context_;
};

class SetFunction : public Object {
public:
    SetFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<SetFunction>();
    }

private:
    ContextPtr context_;
};

class SetCar : public Object {
public:
    SetCar() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<SetCar>();
    }

private:
    ContextPtr context_;
};

class SetCdr : public Object {
public:
    SetCdr() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<SetCdr>();
    }

private:
    ContextPtr context_;
};

// If-condition

class IfFunction : public Object {
public:
    IfFunction() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    }

    ObjectPtr Clone() override {
        return Heap::Instance().Make<IfFunction>();
    }

private:
    ContextPtr context_;
};

// LambdaDeclaration object
// Создаётся в выражении вида lambda (args) (body)
// После применения к нему аргументов и тела, вычисляется уже в Lambda-функцию

class LambdaDeclaration : public Object {
public:
    LambdaDeclaration() = default;

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        context_ = context;
    };

    ObjectPtr Clone() override {
        return Heap::Instance().Make<LambdaDeclaration>();
    }

private:
    ContextPtr context_;
};

class LambdaFunction : public Object {
public:
    LambdaFunction(const ObjectPtrVector& args, const ObjectPtrVector& body, ContextPtr context);

    ObjectPtr Apply(const ObjectPtrVector&) override;

    void SetContext(ContextPtr context) override {
        current_context_ = context;
    }

    ObjectPtr Clone() override {
        ObjectPtr cloned_lambda =
            Heap::Instance().Make<LambdaFunction>(args_, body_, captured_context_);
        cloned_lambda->SetContext(current_context_);
        return cloned_lambda;
    }

private:
    ObjectPtrVector args_;
    ObjectPtrVector body_;
    ContextPtr captured_context_;
    ContextPtr current_context_;
};

// Valid built-in functions map

using PlusFunction = BinaryFoldFunction<std::plus<int64_t>>;
using MinusFunction = BinaryFoldFunction<std::minus<int64_t>>;
using MultiplyFunction = BinaryFoldFunction<std::multiplies<int64_t>>;
using DivisionFunction = BinaryFoldFunction<std::divides<int64_t>>;
using MaxFunction = BinaryFoldFunction<Max<int64_t>>;
using MinFunction = BinaryFoldFunction<Min<int64_t>>;
using LessFunction = MonotonicFunction<std::less<int64_t>>;
using LessEqualFunction = MonotonicFunction<std::less_equal<int64_t>>;
using EqualFunction = MonotonicFunction<std::equal_to<int64_t>>;
using GreaterFunction = MonotonicFunction<std::greater<int64_t>>;
using GrEqualFunction = MonotonicFunction<std::greater_equal<int64_t>>;
using IsNumPred = PredicateFunction<Number>;
using IsBoolPred = PredicateFunction<BooleanSymbol>;
using IsPairPred = PredicateFunction<Cell>;
using SymbolPred = PredicateFunction<Symbol>;

const std::unordered_map<std::string, ObjectPtr> kValidFunctionsMap = {
    {"+", Heap::Instance().Make<PlusFunction>()},
    {"-", Heap::Instance().Make<MinusFunction>()},
    {"*", Heap::Instance().Make<MultiplyFunction>()},
    {"/", Heap::Instance().Make<DivisionFunction>()},
    {"min", Heap::Instance().Make<MinFunction>()},
    {"max", Heap::Instance().Make<MaxFunction>()},
    {"abs", Heap::Instance().Make<AbsFunction>()},
    {"<", Heap::Instance().Make<LessFunction>()},
    {"<=", Heap::Instance().Make<LessEqualFunction>()},
    {"=", Heap::Instance().Make<EqualFunction>()},
    {">", Heap::Instance().Make<GreaterFunction>()},
    {">=", Heap::Instance().Make<GrEqualFunction>()},
    {"number?", Heap::Instance().Make<IsNumPred>()},
    {"boolean?", Heap::Instance().Make<IsBoolPred>()},
    {"quote", Heap::Instance().Make<QuoteFunction>()},
    {"not", Heap::Instance().Make<NegFunction>()},
    {"and", Heap::Instance().Make<AndFunction>()},
    {"pair?", Heap::Instance().Make<IsPairPred>()},
    {"or", Heap::Instance().Make<OrFunction>()},
    {"list-ref", Heap::Instance().Make<ListRefFunction>()},
    {"list?", Heap::Instance().Make<ListPredicateFunction>()},
    {"cons", Heap::Instance().Make<ConsFunction>()},
    {"car", Heap::Instance().Make<CarFunction>()},
    {"cdr", Heap::Instance().Make<CdrFunction>()},
    {"list", Heap::Instance().Make<ToListFunction>()},
    {"null?", Heap::Instance().Make<NullPredicateFunction>()},
    {"list-tail", Heap::Instance().Make<ListTailFunction>()},
    {"symbol?", Heap::Instance().Make<SymbolPred>()},
    {"define", Heap::Instance().Make<DefineFunction>()},
    {"set!", Heap::Instance().Make<SetFunction>()},
    {"if", Heap::Instance().Make<IfFunction>()},
    {"set-car!", Heap::Instance().Make<SetCar>()},
    {"set-cdr!", Heap::Instance().Make<SetCdr>()},
    {"lambda", Heap::Instance().Make<LambdaDeclaration>()}};

// Scope and context realizations

class Scope : public Object {
public:
    Scope() = default;

    Scope(const std::unordered_map<std::string, ObjectPtr>& scope_map) : scope_map_(scope_map) {
        for (const auto& pair : scope_map) {
            AddDependency(pair.second);
        }
    };

    bool Contains(const std::string& symbol_name) {
        return scope_map_.contains(symbol_name);
    }

    ObjectPtr Get(const std::string& symbol_name) {
        return scope_map_[symbol_name];
    }

    void Define(const std::string& symbol_name, ObjectPtr value) {
        ObjectPtr cloned_value = value->Clone();
        AddDependency(cloned_value);
        scope_map_[symbol_name] = cloned_value;
    }

    void Change(const std::string& symbol_name, ObjectPtr value) {
        RemoveDependency(scope_map_[symbol_name]);
        ObjectPtr cloned_value = value->Clone();
        AddDependency(cloned_value);
        scope_map_[symbol_name] = cloned_value;
    }

private:
    std::unordered_map<std::string, ObjectPtr> scope_map_;
};

class Context : public Object {
public:
    Context() = default;

    Context(const Context& other) {
        context_ = other.context_;
        dependencies_ = other.dependencies_;
    }

    bool Contains(const std::string& symbol_name) {
        for (size_t i = 0; i < context_.size(); ++i) {
            if (context_[i]->Contains(symbol_name)) {
                return true;
            }
        }
        return false;
    }

    void Define(const std::string& symbol_name, ObjectPtr value) {
        context_[context_.size() - 1]->Define(symbol_name, value);
    }

    void Change(const std::string& symbol_name, ObjectPtr value) {
        for (int64_t i = context_.size() - 1; i >= 0; --i) {
            if (context_[i]->Contains(symbol_name)) {
                context_[i]->Change(symbol_name, value);
                break;
            }
        }
    }

    void AddScope(ScopePtr scope_ptr) {
        AddDependency(scope_ptr);
        context_.push_back(scope_ptr);
    }

    void PopScope() {
        RemoveDependency(context_.back());
        context_.pop_back();
    }

    void AddEmptyScope() {
        ScopePtr empty_scope = Heap::Instance().Make<Scope>();
        AddDependency(empty_scope);
        context_.push_back(empty_scope);
    }

    ObjectPtr Get(const std::string& symbol_name) {
        for (int64_t i = context_.size() - 1; i >= 0; --i) {
            if (context_[i]->Contains(symbol_name)) {
                return context_[i]->Get(symbol_name);
            }
        }
        return nullptr;
    }

private:
    ScopePtrVector context_;
};