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
#include <optional>
#include <sstream>


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
        static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, wchar_t>::value,
                     "Only char* and wchar_t* types are supported.");
        return std::is_same<char_type, char>::value
            ? reinterpret_cast<const char_type*>(c)
            : reinterpret_cast<const char_type*>(w);
    }

    template <typename char_type>
    constexpr char_type _choose_char(char c, wchar_t w)
    {
        static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, wchar_t>::value,
                     "Only char and wchar_t types are supported.");
        return std::is_same<char_type, char>::value ? c : w;
    }

    inline bool _str_equals(const wchar_t *str1, size_t n1, const wchar_t *str2, size_t n2)
    {
        return n1 == n2 && std::wcsncmp(str1, str2, n1) == 0;
    }

    inline bool _str_equals(const char* str1, size_t n1, const char* str2, size_t n2)
    {
        return n1 == n2 && std::strncmp(str1, str2, n1) == 0;
    }

    inline size_t _tcslen(const wchar_t *str)
    {
        return std::wcslen(str);
    }

    inline size_t _tcslen(const char *str)
    {
        return std::strlen(str);
    }

    /**
     * Options to the tokenize function.
     * See also tokenize.
     */
    template <typename char_type>
    struct Options
    {
        // Character used instead of the left brace ({) in array literals.
        std::optional<char_type> left_brace;

        // Character used instead of the right brace (}) in array literals.
        std::optional<char_type> right_brace;

        // Character used instead of the left bracket ([) in R1C1-style relative references.
        std::optional<char_type> left_bracket;

        // Character used instead of the right bracket (]) in R1C1-style references.
        std::optional<char_type> right_bracket;

        // Separator character used between arguments in a function (,).
        std::optional<char_type> list_separator;

        // Decimal point separator (.).
        std::optional<char_type> decimal_separator;

        // Character used to separate rows in array literals (;).
        std::optional<char_type> row_separator;
    };

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
                                      const Options<char_type>& options,
                                      const char_type* formula,
                                      size_t size)
    {
        std::basic_stringstream<char_type> number_re_ss;
        number_re_ss << R"(^\d+(\)" << options.decimal_separator.value_or(XLFP_CHAR('.')) << R"(\d+)?(E[+-]\d+)?$)";
        const std::basic_regex<char_type> number_re(number_re_ss.str(),
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
                // If the previous token was function, expression, postfix operator or operand, this token
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

            if (token.type() == Token::Type::OperatorInfix && formula[token.start()] == XLFP_CHAR('@'))
            {
                // Implicit intersection operator is always a prefix operator
                token.type(Token::Type::OperatorPrefix);
                token.subtype(Token::Subtype::Intersection);
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
        }
    }

    /**
     * Generate a vector of Tokens from an Excel formula.
     *
     * @param formula The Excel formula to tokenize.
     * @param size Number of characters in the formula string.
     * @param options Optional tokenize options.
     * @return A vector of tokens.
     */
    template <typename char_type>
    inline std::vector<Token> tokenize(const char_type *formula, size_t size, const Options<char_type>& options)
    {
        // Basic checks to make sure it's a valid formula
        if (size < 2 || formula[0] != '=')
            throw std::runtime_error("Invalid Excel formula.");

        // Chars used in parsing excel formual
        const char_type QUOTE_DOUBLE  = XLFP_CHAR('"');
        const char_type QUOTE_SINGLE  = XLFP_CHAR('\'');
        const char_type PAREN_OPEN    = XLFP_CHAR('(');
        const char_type PAREN_CLOSE   = XLFP_CHAR(')');
        const char_type WHITESPACE    = XLFP_CHAR(' ');
        const char_type ERROR_START   = XLFP_CHAR('#');

        // Some chars can be changed in the options
        const auto left_brace = options.left_brace.value_or(XLFP_CHAR('{'));
        const auto right_brace = options.right_brace.value_or(XLFP_CHAR('}'));
        const auto left_bracket = options.left_bracket.value_or(XLFP_CHAR('['));
        const auto right_bracket = options.right_brace.value_or(XLFP_CHAR(']'));
        const auto list_separator = options.list_separator.value_or(XLFP_CHAR(','));
        const auto decimal_separator = options.decimal_separator.value_or(XLFP_CHAR('.'));
        const auto row_separator = options.row_separator.value_or(XLFP_CHAR(';'));

        const char_type* OPERATORS_INFIX   = XLFP_STRING("+-*/^&=><@");
        const char_type* OPERATORS_POSTFIX = XLFP_STRING("%");

        // This matches a number in scientific notation with or without numbers after the + or -.
        // It's used to test for SN numbers before checking for +/- operators.
        std::basic_stringstream<char_type> sn_regex_ss;
        sn_regex_ss << R"(^[1-9](\)" << decimal_separator << R"(\d+)?E[+-]\d*$)";
        const std::basic_regex<char_type> sn_regex(sn_regex_ss.str(),
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
                XLFP_STRING("#SPILL!"),
                NULL
        };

        const char_type* COMPARATORS_MULTI[] = {
                XLFP_STRING(">="),
                XLFP_STRING("<="),
                XLFP_STRING("<>"),
                NULL
        };

        bool in_string = false;
        bool in_path = false;
        bool in_range = false;
        bool in_error = false;

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
            if (in_string) {
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
                    in_string = false;
                    continue;
                }

                ++index;
                continue;
            }

            // single-quoted strings (links)
            // embeds are double
            // end does not mark a token
            if (in_path)
            {
                if (formula[index] == QUOTE_SINGLE)
                {
                    if (((index + 2) <= size) && (formula[index + 1] == QUOTE_SINGLE))
                    {
                        // '' is a quoted ' so skip both
                        index += 2;
                        continue;
                    }

                    in_path = false;
                }

                ++index;
                continue;
            }

            // bracketed strings (R1C1 range index or linked workbook name)
            // no embeds (changed to "()" by Excel)
            // end does not mark a token
            if (in_range)
            {
                if (formula[index] == right_bracket)
                    in_range = false;

                index++;
                continue;
            }

            // error values
            // end marks a token, determined from absolute list of values
            if (in_error)
            {
                for (auto err = &ERRORS[0]; *err != NULL; ++err)
                {
                    if (_str_equals(*err, _tcslen(*err), &formula[start], 1 + index - start))
                    {
                        // add the string token, exit the string and continue
                        tokens.push_back(Token(start, index, Token::Type::Operand, Token::Subtype::Error));
                        start = index + 1;
                        in_error = false;
                        break;
                    }
                }

                ++index;
                continue;
            }

            // scientific notation check
            if (index > start)
            {
                if (std::regex_match(&formula[start], &formula[index]+1, sn_regex))
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

                in_string = true;
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

                in_path = true;
                ++index;
                continue;
            }

            if (formula[index] == left_bracket)
            {
                in_range = true;
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

                in_error = true;
                ++index;
                continue;
            }

            // mark start and end of arrays and array rows
            if (formula[index] == left_brace)
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

            if (formula[index] == row_separator && !stack.empty() && stack.top() == Token::Type::ArrayRow)
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

                start = ++index;
                continue;
            }

            if (formula[index] == right_brace)
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
                    if (_str_equals(*op, _tcslen(*op), &formula[index], 2))
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
            if (formula[index] == list_separator)
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
        _infer_token_subtypes(tokens, options, formula, size);

        return tokens;
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
        return tokenize(formula, size, {});
    }

   /**
    * Generate a vector of Tokens from an Excel formula.
    *
    * @param formula The Excel formula to tokenize.
    * @param options Options controlling how the Excel formula is tokenized.
    * @return A vector of tokens.
    */
    template<typename string_type>
    inline std::vector<Token> tokenize(const string_type &formula, const Options<typename string_type::value_type>& options)
    {
        return tokenize(formula.c_str(), formula.size(), options);
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
        return tokenize(formula.c_str(), formula.size(), {});
    }

   /**
    * Generate a vector of Tokens from an Excel formula.
    *
    * @param formula The Excel formula to tokenize.
    * @param options Options controlling how the Excel formula is tokenized.
    * @return A vector of tokens.
    */
    template<typename char_type>
    inline std::vector<Token> tokenize(std::basic_string_view<char_type> formula, const Options<char_type>& options)
    {
        return tokenize(formula.data(), formula.size(), options);
    }

   /**
    * Generate a vector of Tokens from an Excel formula.
    *
    * @param formula The Excel formula to tokenize.
    * @return A vector of tokens.
    */
    template<typename char_type>
    inline std::vector<Token> tokenize(std::basic_string_view<char_type> formula)
    {
        return tokenize(formula.data(), formula.size(), {});
    }
}


#endif // _XLFPARSER_H_
