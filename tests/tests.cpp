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


TEST_CASE("Simple formula can be parsed correctly", "[xlfparser]")
{
    std::string formula("=1+2+3");
    auto result = xlfparser::tokenize(formula);

    REQUIRE(result.size() == 5);
    CHECK_THAT(result[0].value(formula), Equals("1"));
    CHECK(result[0].type() == xlfparser::Token::Type::Operand);
    CHECK_THAT(result[1].value(formula), Equals("+"));
    CHECK(result[1].type() == xlfparser::Token::Type::OperatorInfix);
    CHECK_THAT(result[2].value(formula), Equals("2"));
    CHECK(result[2].type() == xlfparser::Token::Type::Operand);
    CHECK_THAT(result[3].value(formula), Equals("+"));
    CHECK(result[3].type() == xlfparser::Token::Type::OperatorInfix);
    CHECK_THAT(result[4].value(formula), Equals("3"));
    CHECK(result[4].type() == xlfparser::Token::Type::Operand);
}


TEST_CASE("Wide formula can be parsed correctly", "[xlfparser]")
{
    std::wstring formula(L"=1+2+3");
    auto result = xlfparser::tokenize(formula);

    REQUIRE(result.size() == 5);

    auto value0 = result[0].value(formula);
    CHECK(std::wcscmp(value0.c_str(), L"1") == 0);
    CHECK(result[0].type() == xlfparser::Token::Type::Operand);
    CHECK(result[1].type() == xlfparser::Token::Type::OperatorInfix);

    auto value2 = result[2].value(formula);
    CHECK(std::wcscmp(value2.c_str(), L"2") == 0);
    CHECK(result[2].type() == xlfparser::Token::Type::Operand);
    CHECK(result[3].type() == xlfparser::Token::Type::OperatorInfix);

    auto value4 = result[4].value(formula);
    CHECK(std::wcscmp(value4.c_str(), L"3") == 0);
    CHECK(result[4].type() == xlfparser::Token::Type::Operand);
}


TEST_CASE("Formula including a function parses correctly", "[xlfparser]")
{
    std::string formula("=SUM(1,2)");
    auto result = xlfparser::tokenize(formula);

    REQUIRE(result.size() == 5);
    CHECK_THAT(result[0].value(formula), Equals("SUM"));
    CHECK(result[0].type() == xlfparser::Token::Type::Function);
    CHECK(result[0].subtype() == xlfparser::Token::Subtype::Start);

    CHECK_THAT(result[1].value(formula), Equals("1"));
    CHECK(result[1].type() == xlfparser::Token::Type::Operand);

    CHECK_THAT(result[2].value(formula), Equals(","));
    CHECK(result[2].type() == xlfparser::Token::Type::Argument);

    CHECK_THAT(result[3].value(formula), Equals("2"));
    CHECK(result[3].type() == xlfparser::Token::Type::Operand);

    CHECK(result[4].type() == xlfparser::Token::Type::Function);
    CHECK(result[4].subtype() == xlfparser::Token::Subtype::Stop);
}


TEST_CASE("Formula including nested functions parses correctly", "[xlfparser]")
{
    std::string formula("=outer(inner(1,2))");
    auto result = xlfparser::tokenize(formula);

    REQUIRE(result.size() == 7);

    CHECK_THAT(result[0].value(formula), Equals("outer"));
    CHECK(result[0].type() == xlfparser::Token::Type::Function);
    CHECK(result[0].subtype() == xlfparser::Token::Subtype::Start);

    CHECK_THAT(result[1].value(formula), Equals("inner"));
    CHECK(result[1].type() == xlfparser::Token::Type::Function);
    CHECK(result[1].subtype() == xlfparser::Token::Subtype::Start);

    CHECK_THAT(result[2].value(formula), Equals("1"));
    CHECK(result[2].type() == xlfparser::Token::Type::Operand);

    CHECK_THAT(result[3].value(formula), Equals(","));
    CHECK(result[3].type() == xlfparser::Token::Type::Argument);

    CHECK_THAT(result[4].value(formula), Equals("2"));
    CHECK(result[4].type() == xlfparser::Token::Type::Operand);

    CHECK(result[5].type() == xlfparser::Token::Type::Function);
    CHECK(result[5].subtype() == xlfparser::Token::Subtype::Stop);

    CHECK(result[6].type() == xlfparser::Token::Type::Function);
    CHECK(result[6].subtype() == xlfparser::Token::Subtype::Stop);
}


TEST_CASE("Scientific notation parses correctly", "[xlfparser]")
{
    std::string formula("=2.5E+10-3");
    auto result = xlfparser::tokenize(formula);

    REQUIRE(result.size() == 3);

    CHECK_THAT(result[0].value(formula), Equals("2.5E+10"));
    CHECK(result[0].type() == xlfparser::Token::Type::Operand);

    CHECK_THAT(result[1].value(formula), Equals("-"));
    CHECK(result[1].type() == xlfparser::Token::Type::OperatorInfix);

    CHECK_THAT(result[2].value(formula), Equals("3"));
    CHECK(result[2].type() == xlfparser::Token::Type::Operand);
}
