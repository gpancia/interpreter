# Custom Interpreter
A WIP custom programming language interpreter written using C. It will be a Turing-complete language.

### Planned Features: (subject to change)
+ OCaml-style type inferrence
+ Turing completeness
+ Eager evaluation

### Syntax (subject to change)
+ python-style newlines
+ python-style lack of main() function
+ {}s denote code blocks (sequences of commands), last command's return value is return value for block
++ used for function defs and if statements
+ ()s denote lists or bool expressions, depending on context
++ used for predicates, regular lists and for passing/defining function arguments

#### Example:
```
# Lisp-style comments
x = 4 # set x to 4
y = x + y * 2 # set y to expression, order of operation left-to-right when not following PEMDAS and other stuff like that
str = "hello" + ' ' + "world" # C++-style concats, with seamless integration between chars wrapped in 's and strings wrapped in "s
if (bool_expression_1 >= bool_expression_2 && bool_expression_3) # C-style bool expressions, Lisp-style if-statement behavior
# code executed if true
{
  code
  code
  code
}
{
  code
  code
  code
}
# defining functions:
foo (x y) = 
{
  x + y
}
# calling functions:
bar = foo(3 5) + foo (5 6)-3 # whitespace doesn't matter outside parens
```

### Progress:
+ Lexing: Feature-complete
+ Parsing: Almost feature-complete
+ Evaluation: To-do
