[![Build status](https://api.travis-ci.org/pyxll/xlfparser.svg?branch=master)](https://travis-ci.org/pyxll/xlfparser)

# xlfparser

Header only C++ library for tokenizing Excel formulas

Modern C++ port of ewbi's Excel formula parser (http://ewbi.blogs.com/develops/2004/12/excel_formula_p.html).

See also https://github.com/lishen2/Excel_formula_parser_cpp, which this code was originally based on.

Some advantages of xlfparser over Excel_formula_parser_cpp are:

- Implemented in a single header so easier to integrate.
- Support for wide strings.
- Doesn't require any 3rd party dependecies (eg. PCRE).
- Fewer allocations by avoiding string concatenations.


## Example Usage

```cpp
#include <xlfparser.h>
#include <iostream>

int main()
{
    std::string formula("=SUM(1,2,3)");
    auto tokens = xlfparser::tokenize(formula);
    
    for (const auto& token: tokens)
        std::cout << token.value(formula) << std::end;
        
    return 0;
}
```

See also example.cpp.
