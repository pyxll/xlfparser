/*
The MIT License

Copyright (c) 2019 PyXLL Ltd. https://www.pyxll.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "catch.hpp"
#include "xlfparser.h"

using namespace Catch::Matchers;
using namespace xlfparser;


TEST_CASE("Simple formula can be parsed correctly", "[xlfparser]")
{
    std::string formula("=1+2+3");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 5);
    CHECK_THAT(result[0].value(formula), Equals("1"));
    CHECK(result[0].type() == Token::Type::Operand);
    CHECK_THAT(result[1].value(formula), Equals("+"));
    CHECK(result[1].type() == Token::Type::OperatorInfix);
    CHECK_THAT(result[2].value(formula), Equals("2"));
    CHECK(result[2].type() == Token::Type::Operand);
    CHECK_THAT(result[3].value(formula), Equals("+"));
    CHECK(result[3].type() == Token::Type::OperatorInfix);
    CHECK_THAT(result[4].value(formula), Equals("3"));
    CHECK(result[4].type() == Token::Type::Operand);
}


TEST_CASE("Wide formula can be parsed correctly", "[xlfparser]")
{
    std::wstring formula(L"=1+2+3");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 5);

    auto value0 = result[0].value(formula);
    CHECK(std::wcscmp(value0.c_str(), L"1") == 0);
    CHECK(result[0].type() == Token::Type::Operand);
    CHECK(result[1].type() == Token::Type::OperatorInfix);

    auto value2 = result[2].value(formula);
    CHECK(std::wcscmp(value2.c_str(), L"2") == 0);
    CHECK(result[2].type() == Token::Type::Operand);
    CHECK(result[3].type() == Token::Type::OperatorInfix);

    auto value4 = result[4].value(formula);
    CHECK(std::wcscmp(value4.c_str(), L"3") == 0);
    CHECK(result[4].type() == Token::Type::Operand);
}


TEST_CASE("Formula including a function parses correctly", "[xlfparser]")
{
    std::string formula("=SUM(1,2)");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 5);
    CHECK_THAT(result[0].value(formula), Equals("SUM"));
    CHECK(result[0].type() == Token::Type::Function);
    CHECK(result[0].subtype() == Token::Subtype::Start);

    CHECK_THAT(result[1].value(formula), Equals("1"));
    CHECK(result[1].type() == Token::Type::Operand);

    CHECK_THAT(result[2].value(formula), Equals(","));
    CHECK(result[2].type() == Token::Type::Argument);

    CHECK_THAT(result[3].value(formula), Equals("2"));
    CHECK(result[3].type() == Token::Type::Operand);

    CHECK(result[4].type() == Token::Type::Function);
    CHECK(result[4].subtype() == Token::Subtype::Stop);
}


TEST_CASE("Formula including nested functions parses correctly", "[xlfparser]")
{
    std::string formula("=outer(inner(1,2))");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 7);

    CHECK_THAT(result[0].value(formula), Equals("outer"));
    CHECK(result[0].type() == Token::Type::Function);
    CHECK(result[0].subtype() == Token::Subtype::Start);

    CHECK_THAT(result[1].value(formula), Equals("inner"));
    CHECK(result[1].type() == Token::Type::Function);
    CHECK(result[1].subtype() == Token::Subtype::Start);

    CHECK_THAT(result[2].value(formula), Equals("1"));
    CHECK(result[2].type() == Token::Type::Operand);

    CHECK_THAT(result[3].value(formula), Equals(","));
    CHECK(result[3].type() == Token::Type::Argument);

    CHECK_THAT(result[4].value(formula), Equals("2"));
    CHECK(result[4].type() == Token::Type::Operand);

    CHECK(result[5].type() == Token::Type::Function);
    CHECK(result[5].subtype() == Token::Subtype::Stop);

    CHECK(result[6].type() == Token::Type::Function);
    CHECK(result[6].subtype() == Token::Subtype::Stop);
}


TEST_CASE("Scientific notation parses correctly", "[xlfparser]")
{
    std::string formula("=2.5E+10-3");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 3);

    CHECK_THAT(result[0].value(formula), Equals("2.5E+10"));
    CHECK(result[0].type() == Token::Type::Operand);
    CHECK(result[0].subtype() == Token::Subtype::Number);

    CHECK_THAT(result[1].value(formula), Equals("-"));
    CHECK(result[1].type() == Token::Type::OperatorInfix);

    CHECK_THAT(result[2].value(formula), Equals("3"));
    CHECK(result[2].type() == Token::Type::Operand);
    CHECK(result[2].subtype() == Token::Subtype::Number);
}


TEST_CASE("Errors are parsed correctly", "[xlfparser]")
{
    std::vector<std::string> formulas{
       "=#DIV/0!",
       "=#VALUE!",
       "=#REF!",
       "=#NAME?",
       "=#NUM!",
       "=#N/A",
       "=#SPILL!"
    };

    for (auto formula: formulas)
    {
        auto result = tokenize(formula);
        REQUIRE(result.size() == 1);
        CHECK(result[0].type() == Token::Type::Operand);
        CHECK(result[0].subtype() == Token::Subtype::Error);
    }
}


TEST_CASE("String operands are parsed correctly", "[xlfparser]")
{
    std::string formula(R"(="string1" >= "string2")");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 3);

    CHECK_THAT(result[0].value(formula), Equals("\"string1\""));
    CHECK(result[0].type() == Token::Type::Operand);
    CHECK(result[0].subtype() == Token::Subtype::Text);
    CHECK(result[1].type() == Token::Type::OperatorInfix);
    CHECK(result[1].subtype() == Token::Subtype::Logical);
    CHECK_THAT(result[2].value(formula), Equals("\"string2\""));
    CHECK(result[2].type() == Token::Type::Operand);
    CHECK(result[2].subtype() == Token::Subtype::Text);

    // strings can include quotes
    formula = R"(="a ""b"" c")";
    result = tokenize(formula);

    REQUIRE(result.size() == 1);
    CHECK_THAT(result[0].value(formula), Equals("\"a \"\"b\"\" c\""));
    CHECK(result[0].type() == Token::Type::Operand);
    CHECK(result[0].subtype() == Token::Subtype::Text);
}


TEST_CASE("Implicit intersection parsed correctly", "[xlfparser]")
{
    std::string formula(R"(=@A1:A10)");
    auto result = tokenize(formula);

    REQUIRE(result.size() == 2);

    CHECK_THAT(result[0].value(formula), Equals("@"));
    CHECK(result[0].type() == Token::Type::OperatorPrefix);
    CHECK(result[0].subtype() == Token::Subtype::Intersection);

    CHECK_THAT(result[1].value(formula), Equals("A1:A10"));
    CHECK(result[1].type() == Token::Type::Operand);
    CHECK(result[1].subtype() == Token::Subtype::Range);
}
