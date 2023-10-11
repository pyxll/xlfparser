# distutils: language = c++

from libc.stddef cimport wchar_t
from libcpp.optional cimport optional
from libcpp.vector cimport vector


cdef extern from "<string>" namespace "std":
    cdef cppclass wstring:
        wstring() except +
        wstring(wchar_t*, size_t) except +
        const wchar_t* c_str() except +
        size_t size() except +


cdef extern from "xlfparser.h" namespace "xlfparser::Token":
    cdef cppclass _Type "xlfparser::Token::Type":
        pass

    cdef cppclass _Subtype "xlfparser::Token::Subtype":
        pass


cdef extern from "xlfparser.h" namespace "xlfparser::Token::Type":
    cdef _Type Unknown
    cdef _Type Operand
    cdef _Type Function
    cdef _Type Array
    cdef _Type ArrayRow
    cdef _Type Subexpression
    cdef _Type Argument
    cdef _Type OperatorPrefix
    cdef _Type OperatorInfix
    cdef _Type OperatorPostfix
    cdef _Type Whitespace


cdef extern from "xlfparser.h" namespace "xlfparser::Token::Subtype":
    cdef _Subtype _None "xlfparser::Token::Subtype::None"
    cdef _Subtype Start
    cdef _Subtype Stop
    cdef _Subtype Text
    cdef _Subtype Number
    cdef _Subtype Logical
    cdef _Subtype Error
    cdef _Subtype Range
    cdef _Subtype Math
    cdef _Subtype Concatenation
    cdef _Subtype Intersection
    cdef _Subtype Union


cdef extern from "xlfparser.h" namespace "xlfparser":

    cdef cppclass _Options "xlfparser::Options" [T]:
        optional[T] left_brace
        optional[T] right_brace
        optional[T] left_bracket
        optional[T] right_bracket
        optional[T] list_separator
        optional[T] decimal_separator
        optional[T] row_separator

    cdef cppclass _Token "xlfparser::Token":
        wstring value(const wstring& string) except +
        _Type type() noexcept
        _Subtype subtype() noexcept

    vector[_Token] _tokenize "xlfparser::tokenize" (const wstring &formula, const _Options[wchar_t]& options) except + 
