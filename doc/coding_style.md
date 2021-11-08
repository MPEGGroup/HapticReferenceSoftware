# Guidelines for reference software development

This document is adapted from *JVET-N1003 - Guidelines for VVC reference software development*.

The use of the following rules is mandatory for all software implementation work. Changes that don’t comply with these rules will not be accepted by the software coordinators.

## Copyright/License statements

Copyright disclaimers shall not be altered, except to include the current year. When adding a file, the same copyright disclaimer, as reproduced below, shall be included at the beginning of the file.

```cpp
/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2021, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
```

## Syntax usage
### Language
Use C++17.

### Templates
Do not make excessive usage of templates. Use them only for simple functions like `min()` or `max()` which are easy to understand and where code duplication can be avoided.

### Pre-processor macros
Do not use `#define` for function definitions. Use (`static` or `static inline`) functions instead. They are much less prone to logical errors and compilers can even better optimize them.

Do not use `#define` to encapsulate bug fixes. If bug fixes ever need to be removed again, they can be tracked back using the revision control system (see paragraph *Unused code*).

Avoid using `#define` for simply enabling or disabling a tool. Use a config file option instead unless the check would significantly slow down the code.

Avoid using `#define` to define constants. Use `const` global variables or `constexpr`.

### Header multiple include safety
All header files will include a mechanism to prevent multiple inclusion by using a `#define`. The name of the `#define` is created using the header file name in capital letters and a trailing underscore. All special characters in the file name are replaced by underscores. For a header file named *myheader.h* the structure would be the following:

```cpp
#ifndef _MYHEADER_H_
#define _MYHEADER_H_

<content of header file>

#endif
```

### Private and protected
Make member variables and methods as private as possible. Only methods that have to be called from outside should be public. Private and protected member variables should be named including the “m_” prefix.

Storage classes (and structs) may contain simple member variables not requiring encapsulation. These may be defined as public. In such case the “m_” prefix should be omitted.

### Data types
Don’t use aliases for basic data types, unless the name describes the variable of such type and not the type itself (e.g. `double` being redefined as `Distortion`) or the type is not machine-independent (e.g. long integers). Define custom types in *TypeDef.h*.

Use the appropriate size for data types. Using `<cstdint>` types is encouraged, e.g. `int16_t` for 16 bit or `int32_t` for 32 bit.

Use the (signed) `int` type for integer variables unless there is a good reason not to (e.g., excessive memory usage). Avoid using unsigned types as they can be a source of bugs (e.g., when comparing signed with unsigned integers).

Do not use the `long` type as its width is not machine-independent (may be 32 or 64 bits wide). If a 64-bit variable is required, use type `int64_t`.

Use `size_t` where appropriate.

 using the `NULL` macro. Use `nullptr` instead.

### Unused code
Unused code will be completely removed from the source code. Commenting out (or using `#defines` that evaluate to false) is not allowed. Old revisions are still available in the revision control system.

### Code duplication
Avoid duplication of code. Define functions where appropriate, and localize the scope of `#defines` as much as possible.

### Optimizations
Do not try to do the compiler’s work by trying to optimize the code. Make the code easy to read and maintain. 

Avoid usage of explicit function inlining, and define functions and methods in .cpp files unless they are really short. When using explicit inlining, use the `inline` keyword (without leading underscores).

## Naming
Classes, methods, functions and variables shall be given descriptive and meaningful names describing their functionality and purpose.

All type names (classes, structs, enums, unions, typedefs) shall be named using nouns and have an initial upper-case letter.

Methods and functions shall be named using verbs describing the activity and have an initial lower-case letter.

Variable names must have an initial lower-case letter.

Do not use type prefixes (like iVar, uiVar, etc.) on new variable names. These tend to be confusing if the data type changes and the variable is not renamed (e.g. if it is used differently depending on different macros).

In all cases, “Camel Case” shall be used to connect multiple words, where each subsequent word in a name starts with an upper-case letter.

Example:
```ccp
class CodingTool
{
  void doSomething()
  {
    <code here>
  }

  void* m_privateMember;
};

const int g_someTable[] = { 1, 2, 3, 4 };
```
Global variables shall be named using the prefix ‘g_’.

Non-public member variables shall be named using the prefix ‘m_’.

Example:
```ccp
unsigned g_globalVariable;

struct Foo
{
  int memberVariable;
}

class Bar
{
  int m_memberVariable;
}
```

## Formatting
### Indentation
All code blocks that are included into braces have to be indented by two (2) space characters. Tabulator (TAB) characters are never used.

Example:
```ccp
void doSomething()
{
  <code here>
}
```
### Usage and placement of braces
Code following conditionals (e.g. if, else, do, while) shall always be enclosed in braces, even if it is only a single statement.


The opening brace is placed on a new line on the same indentation level as the defining keyword (e.g. void, if, for, while, etc.). The included code block starts at the following line and is indented. The closing brace is placed on the same indentation level as the opening brace.

Example:
```ccp
void doSomething()
{
  if (<expression>)
  {
    <code here>
  }
}
```

### Automated formatting
All code will be formatted using the **clang-format** tool and the configuration file to be found at the root of the repository (*.clang-format*).

## Code comments
Do not use comments or other means to mark code as yours. The revision control system allows tracking back who submitted which code.

Use comments to explain the intent (or implementation detail) of code sections that are not obvious. Note that obvious may have a different meaning for users who are reading this section for the first time.

## Compiler warnings
Contributed code must **NOT** produce compiler warnings when compiled with the following compilers, after default CMake project generation: 
* Visual Studio 2019,
* gcc-10.x
* clang-11.x

By default, CMake is configured to run [**clang-tidy**](https://clang.llvm.org/extra/clang-tidy/) and will raise errors, if any, at compilation time. The list of tests is configured in *.clang-tidy* file. Please refer to the [official website](https://clang.llvm.org/extra/clang-tidy/checks/list.html) for more details. The tests can be locally disabled in cmake but they will be checked before a branch merge by the software coordinator.

## Some recommendations for a better coding style
### Variable declaration
Variables that are not declared inside of functions (global or file local) should be declared at the beginning of the corresponding file preceding all function definitions.

Variables should be declared as near as possible to the lexical scope of their first usage.

Global variables should not be used without good reason.

Use the `const` qualifier whenever possible

### Switch
The code following a case label should be terminated with a break statement. If it's necessary to handle multiple cases, there should be a comment explaining the reason.

Every switch statement should contain a default branch that handles unexpected cases.

### Functions
Functions should be kept to a reasonable size. If a function exceeds 100 lines of code, consider splitting it into multiple smaller functions.
