# distutils: language = c++

from ._xlfparser cimport _Token, _Type, _Subtype, _Options, _tokenize
from .token import Token
from .options import OptionsKwargs, Options

from cython.operator import dereference
from libc.stddef cimport wchar_t
from libcpp.optional cimport optional
from libcpp.vector cimport vector


cdef extern from "Python.h":
     # Not wrapped by cython as a default
     wchar_t* PyUnicode_AsWideCharString(object o, Py_ssize_t* size) except NULL
     object PyUnicode_FromWideChar(const wchar_t *w, Py_ssize_t size)


cdef wstring _to_wstring(s):
    cdef Py_ssize_t size;
    cdef wchar_t* wstr = PyUnicode_AsWideCharString(s, &size)
    return wstring(wstr, size)


cdef object _from_wstring(wstring s):
    return PyUnicode_FromWideChar(s.c_str(), s.size())


cdef object _from_type(_Type t):
    if <int>t == <int>Unknown:
        return Token.Type.Unknown
    elif <int>t == <int>Operand:
        return Token.Type.Operand
    elif <int>t == <int>Function:
        return Token.Type.Function
    elif <int>t == <int>Array:
        return Token.Type.Array
    elif <int>t == <int>ArrayRow:
        return Token.Type.ArrayRow
    elif <int>t == <int>Subexpression:
        return Token.Type.Subexpression
    elif <int>t == <int>Argument:
        return Token.Type.Argument
    elif <int>t == <int>OperatorPrefix:
        return Token.Type.OperatorPrefix
    elif <int>t == <int>OperatorInfix:
        return Token.Type.OperatorInfix
    elif <int>t == <int>OperatorPostfix:
        return Token.Type.OperatorPostfix
    elif <int>t == <int>Whitespace:
        return Token.Type.Whitespace
    raise ValueError("Unexpected token type")


cdef object _from_subtype(_Subtype st):
    if <int>st == <int>_None:
        return Token.SubType._None
    elif <int>st == <int>Start:
        return Token.SubType.Start
    elif <int>st == <int>Stop:
        return Token.SubType.Stop
    elif <int>st == <int>Text:
        return Token.SubType.Text
    if <int>st == <int>Number:
        return Token.SubType.Number
    elif <int>st == <int>Logical:
        return Token.SubType.Logical
    elif <int>st == <int>Error:
        return Token.SubType.Error
    elif <int>st == <int>Range:
        return Token.SubType.Range
    if <int>st == <int>Math:
        return Token.SubType.Math
    elif <int>st == <int>Concatenation:
        return Token.SubType.Concatenation
    elif <int>st == <int>Intersection:
        return Token.SubType.Intersection
    elif <int>st == <int>Union:
        return Token.SubType.Union
    raise ValueError("Unexpected token subtype")


cdef wchar_t _get_option(options, key):
    value = getattr(options, key)
    if not isinstance(value, str) or len(value) != 1:
        raise ValueError("Unexpected value '%s' for '%s'" % (value, key))
    cdef wstring wvalue = _to_wstring(value)
    return dereference(wvalue.c_str())


cdef _Options[wchar_t] _to_options(object kwargs):
    cdef _Options[wchar_t] coptions
    options = Options.from_kwargs(**kwargs)
    coptions.left_brace = _get_option(options, "left_brace")
    coptions.right_brace = _get_option(options, "right_brace")
    coptions.left_bracket = _get_option(options, "left_bracket")
    coptions.right_bracket = _get_option(options, "right_bracket")
    coptions.list_separator = _get_option(options, "list_separator")
    coptions.decimal_separator = _get_option(options, "decimal_separator")
    coptions.row_separator = _get_option(options, "row_separator")
    return coptions


def tokenize(str formula, **kwargs: OptionsKwargs):
    cdef _Options[wchar_t] options = _to_options(kwargs)
    cdef wstring wformula = _to_wstring(formula)
    cdef vector[_Token] tokens = _tokenize(wformula, options)

    cdef wstring value
    cdef _Type type
    cdef _Subtype sub_type

    result = []
    for i in range(tokens.size()):
        value = tokens[i].value(wformula)
        type = tokens[i].type()
        sub_type = tokens[i].subtype()

        result.append(Token(
            value=_from_wstring(value),
            type=_from_type(type),
            sub_type=_from_subtype(sub_type)
        ))

    return result
