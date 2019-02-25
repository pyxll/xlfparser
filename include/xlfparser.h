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
#ifndef _XLFPARSER_H_
#define _XLFPARSER_H_

#include <cstring>
#include <vector>
#include <stack>
#include <tuple>
#include <regex>
#include <stdexcept>


namespace xlfparser {

    /*
     * Helpers for setting constant string and char literals
     * for char and wchar_t specializations of tokenize.
     */
    #define XLFP_STRING(x) _choose_string<char_type>(x, L##x)
    #define XLFP_CHAR(x) _choose_char<char_type>(x, L##x)

    template <typename char_type>
    constexpr const char_type* _choose_string(const char* c, const wchar_t* w)
    {
        static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, wchar_t>::value);
        if (std::is_same<char_type, char>::value) { return reinterpret_cast<const char_type*>(c); }
        if (std::is_same<char_type, wchar_t>::value) { return reinterpret_cast<const char_type*>(w); }
    }

    template <typename char_type>
    constexpr char_type _choose_char(char c, wchar_t w)
    {
        static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, wchar_t>::value);
        if (std::is_same<char_type, char>::value) { return c; }
        if (std::is_same<char_type, wchar_t>::value) { return w; }
    }

    inline int _tcsncmp(const wchar_t *str1, const wchar_t *str2, size_t n)
    {
        return std::wcsncmp(str1, str2, n);
    }

    inline int _tcsncmp(const char* str1, const char* str2, size_t n)
    {
        return std::strncmp(str1, str2, n);
    }

    /**
     * Token class representing the tokens in an Excel formula.
     * See also tokenize.
     */
    class Token
    {
    public:
        enum class Type
        {
            Unknown,
            Operand,
            Function,
            Array,
            ArrayRow,
            Subexpression,
            Argument,
            OperatorPrefix,
            OperatorInfix,
            OperatorPostfix,
            Whitespace
        };

        enum class Subtype
        {
            None,
            Start,
            Stop,
            Text,
            Number,
            Logical,
            Error,
            Range,
            Math,
            Concatenation,
            Intersection,
            Union
        };

        Token(size_t start, size_t end, Type type, Subtype subtype):
                m_start(start), m_end(end), m_type(type), m_subtype(subtype) {};

        Token(const Token& other):
                m_start(other.m_start), m_end(other.m_end), m_type(other.m_type), m_subtype(other.m_subtype) {}

        Token& operator=(const Token& other) {
            m_start = other.m_start;
            m_end = other.m_end;
            m_type = other.m_type;
            m_subtype = other.m_subtype;
            return *this;
        }

        /**
         * Get the string value of the token.
         *
         * @param string The original formula used to create the token via tokenize.
         * @param size Number of characters in string.
         * @return String value of the token.
         */
        template <typename char_type,
                  typename traits_type = std::char_traits<char_type>,
                  typename alloc_type = std::allocator<char_type>>
        auto value(const char_type* string, size_t size) const
        {
            if (m_end >= size || m_start > m_end)
                throw std::out_of_range("Token index out of range");

            typedef std::basic_string<char_type, traits_type, alloc_type> string_type;
            return string_type(&string[m_start], m_end + 1 - m_start);
        }

        /**
         * Get the string value of the token.
         *
         * @param string The original formula used to create the token via tokenize.
         * @return String value of the token.
         */
        template <typename string_type>
        string_type value(const string_type& string) const
        {
            if (m_end >= string.size() || m_start > m_end)
                throw std::out_of_range("Token index out of range");

            return string.substr(m_start, m_end + 1 - m_start);
        }

        Type type() const { return m_type; }
        void type(Type t) { m_type = t; }

        Subtype subtype() const { return m_subtype; }
        void subtype(Subtype s) { m_subtype = s; }

        size_t start() const { return m_start; }
        void start(size_t start) { m_start = start; }

        size_t end() const { return m_end; }
        void end(size_t end) { m_end = end; }

    private:
        size_t m_start;
        size_t m_end;
        Type m_type;
        Subtype m_subtype;
    };

    template <typename char_type>
    inline std::vector<Token> _fix_whitespace_tokens(const std::vector<Token> tokens,
                                                     const char_type* formula,
                                                     size_t size)
    {
        std::vector<Token> new_tokens;
        new_tokens.reserve(tokens.size());

        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
        {
            auto& token = *iter;
            if (token.type() != Token::Type::Whitespace)
            {
                new_tokens.push_back(token);
                continue;
            }

            // Examine the previous and next tokens to see if the whitepsace is actually an intersection operator
            if (iter == tokens.begin() || iter == tokens.end()-1)
                continue;

            auto& previous = *(iter-1);
            auto& next = *(iter+1);

            // If the previous token is not the end of a function, subexpression or operand skip the whitespace
            if (!((previous.type() == Token::Type::Function && previous.subtype() == Token::Subtype::Stop) ||
                  (previous.type() == Token::Type::Subexpression && previous.subtype() == Token::Subtype::Stop) ||
                  (previous.type() == Token::Type::Operand)))
                continue;

            // If the next token is not the start of a function, subexpression or operand skip the whitespace
            if (!((next.type() == Token::Type::Function && next.subtype() == Token::Subtype::Start) ||
                  (next.type() == Token::Type::Subexpression && next.subtype() == Token::Subtype::Start) ||
                  (next.type() == Token::Type::Operand)))
                continue;

            // Space between functions, subexpressions or operands is an intersection operator
            new_tokens.push_back({token.start(),
                                  token.end(),
                                  Token::Type::OperatorInfix,
                                  Token::Subtype::Intersection});
        }

        return new_tokens;
    }

    template <typename char_type>
    inline void _infer_token_subtypes(std::vector<Token>& tokens,
                                      const char_type* formula,
                                      size_t size)
    {
        // regular expression for numbers (normal and scientific notation)
        const std::basic_regex<char_type> number_re(XLFP_STRING(R"(^\d+(:?.\d+)?(:?E[+-]\d+)?$)"),
                                                   std::regex_constants::ECMAScript |
                                                   std::regex_constants::icase);

        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
        {
            auto& token = *iter;

            if (token.start() >= size || token.end() >= size)
                throw std::out_of_range("Token index out of range");

            if (token.type() == Token::Type::OperatorInfix && (
                    formula[token.start()] == XLFP_CHAR('-') ||
                    formula[token.start()] == XLFP_CHAR('+')))
            {
                // If the previous token  was function, expression, postfix operator or operand, this token
                // is an infix operator of subtype math.
                if (iter > tokens.begin())
                {
                    auto& previous = *(iter-1);
                    if ((previous.type() == Token::Type::Function && previous.subtype() == Token::Subtype::Stop) ||
                        (previous.type() == Token::Type::Subexpression && previous.subtype() == Token::Subtype::Stop) ||
                        (previous.type() == Token::Type::OperatorPostfix) ||
                        (previous.type() == Token::Type::Operand))
                    {
                        token.subtype(Token::Subtype::Math);
                        continue;
                    }
                }

                // Otherwise assume it's a prefix operator
                token.type(Token::Type::OperatorPrefix);
                token.subtype(Token::Subtype::Math);
                continue;
            }

            if (token.type() == Token::Type::OperatorInfix && formula[token.start()] == XLFP_CHAR('+'))
            {
                // If the previous token  was function, expression, postfix operator or operand, this token
                // is an infix operator of subtype math.
                if (iter > tokens.begin())
                {
                    auto& previous = *(iter-1);
                    if ((previous.type() == Token::Type::Function && previous.subtype() == Token::Subtype::Stop) ||
                        (previous.type() == Token::Type::Subexpression && previous.subtype() == Token::Subtype::Stop) ||
                        (previous.type() == Token::Type::OperatorPostfix) ||
                        (previous.type() == Token::Type::Operand))
                    {
                        token.subtype(Token::Subtype::Math);
                        continue;
                    }
                }

                // Otherwise assume it's a prefix operator
                token.type(Token::Type::OperatorPrefix);
                token.subtype(Token::Subtype::Math);
                continue;
            }

            if (token.type() == Token::Type::OperatorInfix && token.subtype() == Token::Subtype::None)
            {
                if (formula[token.start()] == XLFP_CHAR('<') ||
                    formula[token.start()] == XLFP_CHAR('>') ||
                    formula[token.start()] == XLFP_CHAR('='))
                {
                    token.subtype(Token::Subtype::Logical);
                }
                else if (formula[token.start()] == XLFP_CHAR('&'))
                {
                    token.subtype(Token::Subtype::Concatenation);
                }
                else
                {
                    token.subtype(Token::Subtype::Math);
                }

                continue;
            }

            // Set the operand type to Number or Range
            if (token.type() == Token::Type::Operand && token.subtype() == Token::Subtype::None)
            {
                if (std::regex_match(&formula[token.start()], &formula[token.end()]+1, number_re))
                {
                    token.subtype(Token::Subtype::Number);
                }
                else
                {
                    token.subtype(Token::Subtype::Range);
                }
            }

            // Trim '@' from the start of any function names
            if (token.type() == Token::Type::Function)
            {
                if (token.end() > token.start() && formula[token.start()] == XLFP_CHAR('@'))
                    token.start(token.start() + 1);
            }
        }

    }

    /**
     * Generate a vector of Tokens from an Excel formula.
     *
     * @param formula The Excel formula to tokenize.
     * @param size Number of characters in the formula string.
     * @return A vector of tokens.
     */
    template <typename char_type>
    inline std::vector<Token> tokenize(const char_type *formula, size_t size)
    {
        // Basic checks to make sure it's a valid formula
        if (size < 2 || formula[0] != '=')
            throw std::runtime_error("Invalid Excel formula.");

        //! keywords used in parsing excel formual
        const char_type QUOTE_DOUBLE  = XLFP_CHAR('"');
        const char_type QUOTE_SINGLE  = XLFP_CHAR('\'');
        const char_type BRACKET_CLOSE = XLFP_CHAR(']');
        const char_type BRACKET_OPEN  = XLFP_CHAR('[');
        const char_type BRACE_OPEN    = XLFP_CHAR('{');
        const char_type BRACE_CLOSE   = XLFP_CHAR('}');
        const char_type PAREN_OPEN    = XLFP_CHAR('(');
        const char_type PAREN_CLOSE   = XLFP_CHAR(')');
        const char_type SEMICOLON     = XLFP_CHAR(';');
        const char_type WHITESPACE    = XLFP_CHAR(' ');
        const char_type COMMA         = XLFP_CHAR(',');
        const char_type ERROR_START   = XLFP_CHAR('#');

        const char_type* OPERATORS_INFIX   = XLFP_STRING("+-*/^&=><");
        const char_type* OPERATORS_POSTFIX = XLFP_STRING("%");

        // This matches numbers in scientific notation (e.g '1.5e+10')
        // and also partial matches of the form 'x.xE+' (e.g. '1.5E+') .
        const std::basic_regex<char_type> REGEX_SN(XLFP_STRING(R"(^[1-9]{1}(:?.[0-9]+)?E[+-]\d*$)"),
                                                   std::regex_constants::ECMAScript |
                                                   std::regex_constants::icase);

        const char_type* ERRORS[] = {
                XLFP_STRING("#NULL!"),
                XLFP_STRING("#DIV/0!"),
                XLFP_STRING("#VALUE!"),
                XLFP_STRING("#REF!"),
                XLFP_STRING("#NAME?"),
                XLFP_STRING("#NUM!"),
                XLFP_STRING("#N/A"),
                NULL
        };

        const char_type* COMPARATORS_MULTI[] = {
                XLFP_STRING(">="),
                XLFP_STRING("<="),
                XLFP_STRING("<>"),
                NULL
        };



        bool inString = false;
        bool inPath = false;
        bool inRange = false;
        bool inError = false;

        std::vector<Token> tokens;
        std::stack<Token::Type> stack;

        size_t index = 1;  // first char is always '='
        size_t start = index;  // start of the current token
        while(index < size && formula[index] != L'\0')
        {
            // state-dependent character evaluation (order is important)

            // double-quoted strings
            // embeds are doubled
            // end marks token
            if (inString) {
                if (formula[index] == QUOTE_DOUBLE)
                {
                    if (((index + 2) <= size) && (formula[index + 1] == QUOTE_DOUBLE))
                    {
                        // '""' is a quoted '"' so skip both
                        index += 2;
                        continue;
                    }

                    // add the string token, exit the string and continue
                    tokens.push_back(Token(start, index, Token::Type::Operand, Token::Subtype::Text));
                    start = ++index;
                    inString = false;
                    continue;
                }

                ++index;
                continue;
            }

            // single-quoted strings (links)
            // embeds are double
            // end does not mark a token
            if (inPath)
            {
                if (formula[index] == QUOTE_SINGLE)
                {
                    if (((index + 2) <= size) && (formula[index + 1] == QUOTE_SINGLE))
                    {
                        // '' is a quoted ' so skip both
                        index += 2;
                        continue;
                    }

                    inPath = false;
                }

                ++index;
                continue;
            }

            // bracketed strings (R1C1 range index or linked workbook name)
            // no embeds (changed to "()" by Excel)
            // end does not mark a token
            if (inRange)
            {
                if (formula[index] == BRACKET_CLOSE)
                    inRange = false;

                index++;
                continue;
            }

            // error values
            // end marks a token, determined from absolute list of values
            if (inError)
            {
                for (auto err = &ERRORS[0]; *err != NULL; ++err)
                {
                    if (_tcsncmp(*err, &formula[index], index - start) == 0)
                    {
                        // add the string token, exit the string and continue
                        tokens.push_back(Token(start, index, Token::Type::Operand, Token::Subtype::Error));
                        start = index + 1;
                        inError = false;
                        break;
                    }
                }

                ++index;
                continue;
            }

            // scientific notation check
            if (index > start)
            {
                if (std::regex_search(&formula[start], &formula[index]+1, REGEX_SN))
                {
                    ++index;
                    continue;
                }
            }

            // independent character evaluation (order not important)
            // establish state-dependent character evaluations
            if (formula[index] == QUOTE_DOUBLE) {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Unknown, Token::Subtype::None));
                    start = index;
                }

                inString = true;
                ++index;
                continue;
            }

            if (formula[index] == QUOTE_SINGLE)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Unknown, Token::Subtype::None));
                    start = index;
                }

                inPath = true;
                ++index;
                continue;
            }

            if (formula[index] == BRACKET_OPEN)
            {
                inRange = true;
                ++index;
                continue;
            }

            if (formula[index] == ERROR_START)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Unknown, Token::Subtype::None));
                    start = index;
                }

                inError = true;
                ++index;
                continue;
            }

            // mark start and end of arrays and array rows
            if (formula[index] == BRACE_OPEN)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Unknown, Token::Subtype::None));
                    start = index;
                }

                tokens.push_back(Token(start, index, Token::Type::Array, Token::Subtype::Start));
                tokens.push_back(Token(start, index, Token::Type::ArrayRow, Token::Subtype::Start));

                stack.push(Token::Type::Array);
                stack.push(Token::Type::ArrayRow);

                start = ++index;
                continue;
            }

            if (formula[index] == SEMICOLON)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                    start = index;
                }

                tokens.push_back(Token(start, index, stack.top(), Token::Subtype::Stop));
                stack.pop();

                tokens.push_back(Token(start, index, Token::Type::ArrayRow, Token::Subtype::Start));
                stack.push(Token::Type::ArrayRow);

                start = index++;
                continue;
            }

            if (formula[index] == BRACE_CLOSE)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                    start = index;
                }

                tokens.push_back(Token(start, index, stack.top(), Token::Subtype::Stop));
                stack.pop();

                tokens.push_back(Token(start, index, stack.top(), Token::Subtype::Stop));
                stack.pop();

                start = ++index;
                continue;
            }

            // trim white-space
            if (formula[index] == WHITESPACE)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                    start = index;
                }

                while ((formula[index] == WHITESPACE) && (index < size))
                    index++;

                tokens.push_back(Token(start, index-1, Token::Type::Whitespace, Token::Subtype::None));

                start = index;
                continue;
            }

            // multi-character comparators
            if ((index + 2) <= size)
            {
                bool foundOp = false;
                for (auto op = &COMPARATORS_MULTI[0]; *op != NULL; ++op)
                {
                    if (_tcsncmp(*op, &formula[index], 2) == 0)
                    {
                        if (index > start)
                        {
                            tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                            start = index;
                        }

                        tokens.push_back(Token(start, index+1, Token::Type::OperatorInfix, Token::Subtype::Logical));
                        foundOp = true;
                        break;
                    }
                }

                if (foundOp)
                {
                    index += 2;
                    start = index;
                    continue;
                }
            }

            // standard infix operators
            bool foundOp = false;
            for (auto op = OPERATORS_INFIX; *op != L'\0'; ++op)
            {
                if (*op == formula[index])
                {
                    if (index > start)
                    {
                        tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                        start = index;
                    }

                    tokens.push_back(Token(start, index, Token::Type::OperatorInfix, Token::Subtype::None));
                    foundOp = true;
                    break;
                }
            }

            if (foundOp)
            {
                start = ++index;
                continue;
            }

            // standard postfix operators
            for (auto op = OPERATORS_POSTFIX; *op != L'\0'; ++op)
            {
                if (*op == formula[index])
                {
                    if (index > start)
                    {
                        tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                        start = index;
                    }

                    tokens.push_back(Token(start, index, Token::Type::OperatorPostfix, Token::Subtype::None));
                    foundOp = true;
                    break;
                }
            }

            if (foundOp)
            {
                start = ++index;
                continue;
            }

            // start subexpression or function
            if (formula[index] == PAREN_OPEN)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Function, Token::Subtype::Start));
                    stack.push(Token::Type::Function);
                }
                else
                {
                    tokens.push_back(Token(start, index, Token::Type::Subexpression, Token::Subtype::Start));
                    stack.push(Token::Type::Subexpression);
                }

                start = ++index;
                continue;
            }

            // function, subexpression, or array parameters, or operand unions
            if (formula[index] == COMMA)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                    start = index;
                }

                auto type = (!stack.empty() && stack.top() == Token::Type::Function)
                                ? std::make_tuple(Token::Type::Argument, Token::Subtype::None)
                                : std::make_tuple(Token::Type::OperatorInfix, Token::Subtype::Union);

                tokens.push_back(Token(start, index, std::get<0>(type), std::get<1>(type)));

                start = ++index;
                continue;
            }

            // stop subexpression
            if (formula[index] == PAREN_CLOSE)
            {
                if (index > start)
                {
                    tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));
                    start = index;
                }

                tokens.push_back(Token(start, index, stack.top(), Token::Subtype::Stop));
                stack.pop();

                start = ++index;
                continue;
            }

            // token accumulation
            ++index;
        }

        // dump remaining accumulation, if any
        if (index > start && (tokens.empty() || tokens.back().end() < start))
            tokens.push_back(Token(start, index-1, Token::Type::Operand, Token::Subtype::None));

        // label intersection operators specified as whitespace correctly
        tokens = _fix_whitespace_tokens(tokens, formula, size);

        // set the token subtypes correctly
        _infer_token_subtypes(tokens, formula, size);

        return tokens;
    }


   /**
    * Generate a vector of Tokens from an Excel formula.
    *
    * @param formula The Excel formula to tokenize.
    * @return A vector of tokens.
    */
    template<typename string_type>
    inline std::vector<Token> tokenize(const string_type &formula)
    {
        return tokenize(formula . c_str(), formula . size());
    }
}


#endif // _PYXLL_XLPARSER_PARSER_H_
