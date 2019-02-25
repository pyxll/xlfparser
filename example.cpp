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
#include "xlfparser.h"
#include <string>
#include <vector>
#include <iostream>

using namespace xlfparser;

std::ostream& operator<<(std::ostream& os, Token::Type t)
{
    switch (t)
    {
        case Token::Type::Operand:
            os << "Operand";
            break;
        case Token::Type::Function:
            os << "Function";
            break;
        case Token::Type::Array:
            os << "Array";
            break;
        case Token::Type:: ArrayRow:
            os << "ArrayRow";
            break;
        case Token::Type::Subexpression:
            os << "Subexpression";
            break;
        case Token::Type::Argument:
            os << "Argument";
            break;
        case Token::Type::OperatorPrefix:
            os << "OperatorPrefix";
            break;
        case Token::Type::OperatorInfix:
            os << "OperatorInfix";
            break;
        case Token::Type::OperatorPostfix:
            os << "OperatorPostfix";
            break;
        case Token::Type::Whitespace:
            os << "Whitespace";
            break;
        case Token::Type::Unknown:
        default:
            os << "Unknown";
            break;
    }
    
    return os;
}

std::ostream& operator<<(std::ostream& os, Token::Subtype t)
{
    switch (t)
    {
        case Token::Subtype::None:
            os << "None";
            break;
        case Token::Subtype::Start:
            os << "Start";
            break;
        case Token::Subtype::Stop:
            os << "Stop";
            break;
        case Token::Subtype::Text:
            os << "Text";
            break;
        case Token::Subtype::Number:
            os << "Number";
            break;
        case Token::Subtype::Logical:
            os << "Logical";
            break;
        case Token::Subtype::Error:
            os << "Error";
            break;
        case Token::Subtype::Range:
            os << "Range";
            break;
        case Token::Subtype::Math:
            os << "Math";
            break;
        case Token::Subtype::Concatenation:
            os << "Concatenation";
            break;
        case Token::Subtype::Intersection:
            os << "Intersection";
            break;
        case Token::Subtype::Union:
            os << "Union";
            break;
        default:
            os << "Unknown";
            break;
    }
    
    return os;
}

int main()
{
    std::vector<std::string>inputs {
        "=1E+10+3+5",
        "=3 * 4 + 5",
        "=50",
        "=1+1",
        "=$A1",
        "=$B$2",
        "=SUM(B5:B15)",
        "=SUM(B5:B15,D5:D15)",
        "=SUM(B5:B15 A7:D7)",
        "=SUM(sheet1!$A$1:$B$2)",
        "=[data.xls]sheet1!$A$1",
        "=SUM((A:A 1:1))",
        "=SUM((A:A,1:1))",
        "=SUM((A:A A1:B1))",
        "=SUM(D9:D11,E9:E11,F9:F11)",
        "=SUM((D9:D11,(E9:E11,F9:F11)))",
        "=IF(P5=1.0,\"NA\",IF(P5=2.0,\"A\",IF(P5=3.0,\"B\",IF(P5=4.0,\"C\",IF(P5=5.0,\"D\",IF(P5=6.0,\"E\",IF(P5=7.0,\"F\",IF(P5=8.0,\"G\"))))))))",
        "={SUM(B2:D2*B3:D3)}",
        "=SUM(123 + SUM(456) + (45DATE(2002,1,6),0,IF(ISERROR(R[41]C[2]),0,IF(R13C3>=R[41]C[2],0, IF(AND(R[23]C[11]>=55,R[24]C[11]>=20),R53C3,0))))",
        "=IF(R[39]C[11]>65,R[25]C[42],ROUND((R[11]C[11]*IF(OR(AND(R[39]C[11]>=55, "
            "(R[40]C[11]>=20),AND(R[40]C[11]>=20,R11C3=\"YES\")),R[44]C[11],R[43]C[11]))+(R[14]C[11] "
            "*IF(OR(AND(R[39]C[11]>=55,R[40]C[11]>=20),AND(R[40]C[11]>=20,R11C3=\"YES\")), "
            "R[45]C[11],R[43]C[11])),0))"
    };

    for (const auto& input: inputs)
    {
        auto tokens = xlfparser::tokenize(input);
        std::cout << input << " :" << std::endl;
        for (const auto& token: tokens)
            std::cout << '\t' << token.value(input) << '\t' << token.type() << '\t' << token.subtype() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
