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
+ ()s used for operation ordering and for function argument lists (for both declaring and calling) 
+ []s used for declaring/accessing lists (0-indexed)

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

# lists:
list = ["a","b","c"]
print list[2] # prints out "c"
```

### Progress:
+ Lexing: Feature-complete
+ Parsing: Almost feature-complete
+ Evaluation: To-do
