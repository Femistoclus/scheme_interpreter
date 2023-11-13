#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "PairMutations") {
    ExpectNoError("(define x '(1 . 2))");

    ExpectNoError("(set-car! x 5)");
    ExpectEq("(car x)", "5");

    ExpectNoError("(set-cdr! x 6)");
    ExpectEq("(cdr x)", "6");

    ExpectNoError("(define z 1543)");

    ExpectNoError("(set-cdr! x z)");
    ExpectEq("(cdr x)", "1543");

    ExpectNoError("(set-car! x z)");
    ExpectEq("(car x)", "1543");
}

TEST_CASE_METHOD(SchemeTest, "SelfReferenceCar") {
    ExpectNoError("(define x '(1 . 2))");
    ExpectNoError("(set-car! x x)");
    ExpectEq("(cdr (car (car (car x))))", "2");

    ExpectNoError("(set-car! (car x) (car x))");
    ExpectEq("(cdr x)", "2");

    ExpectNoError("(set-cdr! (car (car x)) 1543)");
    ExpectEq("(cdr x)", "1543");

    ExpectNoError("(set-car! (car (car x)) 3)");
    ExpectEq("(car x)", "3");
}

TEST_CASE_METHOD(SchemeTest, "SelfReferenceCdr") {
    ExpectNoError("(define y '(1 . 2))");
    ExpectNoError("(set-cdr! y y)");
    ExpectEq("(car (cdr (cdr (cdr y))))", "1");

    ExpectNoError("(set-cdr! (cdr y) (cdr y))");
    ExpectEq("(car y)", "1");

    ExpectNoError("(set-car! (cdr (cdr y)) 1543)");
    ExpectEq("(car y)", "1543");

    ExpectNoError("(set-cdr! (cdr (cdr y)) 3)");
    ExpectEq("(cdr y)", "3");
}
