#### Scheme_interpreter

В этом задании я реализовал интерпретатор для LISP-подобного языка программирования из подмножества Scheme. 

Язык будет состоять из:
 - Примитивных типов: целых чисел, bool-ов и _символов_ (идентификаторов).
 - Составных типов: пар и списков.
 - Переменных с синтаксической областью видимости.
 - Функций и лямбда-выражений.

## Сборщик мусора

В финальной части проекта был реализован сборщик мусора, работающий по алгоритму Mark-and-Sweep, т.е. строится граф зависимостей между объектами и после каждого вызова Run удаляет недостижимые от корня ноды.
Для этого был реализован Singleton класс Heap, который выделяет память под Object через метод Heap::Make.

## Выполнение выражений
Выполнение языка происходит в 3 этапа:

**Токенизация** - преобразует текст программы в последовательность атомарных лексем. 

**Синтаксический анализ** - преобразует последовательность токенов в AST.
   
**Вычисление** - рекурсивно обходит AST программы и преобразует его в соответствии с набором правил.

## Пример

Выражение 
```
    (+ 2 (/ -3 +4))
``` 
в результате токенизации превратится в список токенов:
```
    { 
        OpenParen(),
        Symbol("+"),
        Number(2),
        OpenParen(),
        Symbol("/"),
        Number(-3),
        Number(4),
        CloseParen(),
        CloseParen()
    }
```
     
 Последовательность токенов в результате синтаксического анализа
 превратится в дерево:
     
```
    Cell{
        Symbol("+"),
        Cell{
            Number(2),
            Cell{
                Cell{
                    Symbol("/"),
                    Cell{
                        Number(-3),
                        Cell{
                            Number(4),
                            nullptr
                        }
                    }
                }
                nullptr
            }
        }
    }
```
Результатом же выполнения выражения будет 

```
    (+ 2 (/ -3 +4)) => 1
```

Помимо базовых арифметических операций, реализована поддержка:

- If

Работает как if :D Возможны 2 формы записи.

* `(if condition true-branch)`
* `(if condition true-branch false-branch)`

Сначала вычисляет `condition` и проверяет значение на истинность (см. определение истинности). Затем вычисляет либо `true-branch`, либо `false-branch` и возвращает как результат всего `if`-а.

- Переменных

Поддержка переменных реализована с помощью особых форм `define` и `set!`.

```scheme
$ (define x 1)
> ()
$ x
> 1
```

Команда `set!` используется для изменения значения **существующей** переменной:

```scheme
$ (define x 1)
> ()
$ (set! x 2)
$ x
> 2
$ (set! y 1)
> NameError
```

- Лямбда-функции

Синтаксис:

* `(lambda (x) (+ 1 x))`
* `(lambda (x y) (* y x))`
* `(lambda (x) (set! x (* x 2)) (+ 1 x))`
* `(lambda () 1)`

Создаёт новую функцию. Сначала перечисляется список аргументов функции, затем её тело. Тело может состоять из нескольких выражений, в этом случае они вычисляются по порядку а результат последнего выражения становится результатом функции.

Запись `(define (fn-name <args>) <body>)` эквивалентна `(define fn-name (lambda (<args>) <body>))`. То есть, запись `(define (inc x) (+ x 1))` создаёт новую функцию `inc`.

- Захват контекста

Также возможен и захват контекста. Синтаксис примерно совпадает с C++:

```c++
auto Range(int x) {
  return [&x] () {
    ++x;
    return x;
  };
}

void F() {
  auto r = Range(10);

  std::cout << r() << std::endl; // 11
  std::cout << r() << std::endl; // 12
}
```

В **Scheme**:

```scheme
$ (define range
    (lambda (x)
      (lambda ()
        (set! x (+ x 1))
        x)))

$ (define my-range (range 10))

$ (my-range)
> 11

$ (my-range)
> 12
```
## Ещё примеры лямбда-функций:

```
$ (define (fib x) (if (< x 3) 1 (+ (fib (- x 1)) (fib (- x 2)) )))
$ (fib 8)
> 21

$ (define (foo x) (if (< x 2) 42 (bar (- x 1))))
$ (define (bar x) (if (< x 2) 24 (foo (/ x 2))))
$ (foo 3)
> 42
$ (foo 6)
> 24
$ (bar 7)
> 42
(bar 13)
> 24
```

## Обработка ошибок

Интерпретатор различает 3 вида ошибок:

1. `SyntaxError`: ошибки синтаксиса. Возникают, когда программа не соответствует формальному синтаксису языка. Или когда программа неправильно использует особые формы.

2. `NameError`: ошибки обращения к неопределённым переменным.

3. `RuntimeError`: ошибки времени исполнения. К этим ошибкам относятся все остальные ошибки, которые могут возникнуть во время выполнения программы.
Например: неправильное количество аргументов передано в функцию, неправильный тип аргумента.






