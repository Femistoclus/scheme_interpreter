#pragma once

#include <catch.hpp>

#include <error.h>
#include <scheme.h>
#include <allocations_checker.h>

class SchemeTest {
public:
    void ExpectEq(std::string expression, const std::string& result) {
        REQUIRE(interpreter_.Run(expression) == result);
    }

    void ExpectNoError(std::string expression) {
        REQUIRE_NOTHROW(interpreter_.Run(expression));
    }

    void ExpectSyntaxError(std::string expression) {
        REQUIRE_THROWS_AS(interpreter_.Run(expression), SyntaxError);
    }

    void ExpectRuntimeError(std::string expression) {
        REQUIRE_THROWS_AS(interpreter_.Run(expression), RuntimeError);
    }

    void ExpectNameError(std::string expression) {
        REQUIRE_THROWS_AS(interpreter_.Run(expression), NameError);
    }

private:
    Interpreter interpreter_;
};

#define WITH_ALLOCATION_DIFFERENCE_CHECK(max_expected_diff, expression)                            \
    do {                                                                                           \
        alloc_checker::ResetCounters();                                                            \
                                                                                                   \
        expression;                                                                                \
                                                                                                   \
        int64_t alloc_count = alloc_checker::AllocCount(),                                         \
                dealloc_count = alloc_checker::DeallocCount(), diff = alloc_count - dealloc_count; \
                                                                                                   \
        std::cerr << "Allocations: " << alloc_count << "\n";                                       \
        std::cerr << "Deallocations: " << dealloc_count << "\n";                                   \
        std::cerr << "Difference: " << diff << "\n\n";                                             \
                                                                                                   \
        REQUIRE(diff >= 0);                                                                        \
        REQUIRE(diff <= max_expected_diff);                                                        \
    } while (false);
