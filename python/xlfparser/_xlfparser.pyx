# distutils: language = c++

from ._xlfparser cimport _Token, _Type, _Subtype, _Options, _tokenize
from .types import Token, Type, SubType

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
        return Type.Unknown
    elif <int>t == <int>Operand:
        return Type.Operand
    elif <int>t == <int>Function:
        return Type.Function
    elif <int>t == <int>Array:
        return Type.Array
    elif <int>t == <int>ArrayRow:
        return Type.ArrayRow
    elif <int>t == <int>Subexpression:
        return Type.Subexpression
    elif <int>t == <int>Argument:
        return Type.Argument
    elif <int>t == <int>OperatorPrefix:
        return Type.OperatorPrefix
    elif <int>t == <int>OperatorInfix:
        return Type.OperatorInfix
    elif <int>t == <int>OperatorPostfix:
        return Type.OperatorPostfix
    elif <int>t == <int>Whitespace:
        return Type.Whitespace
    raise ValueError("Unexpected token type")


cdef object _from_subtype(_Subtype st):
    if <int>st == <int>_None:
        return SubType._None
    elif <int>st == <int>Start:
        return SubType.Start
    elif <int>st == <int>Stop:
        return SubType.Stop
    elif <int>st == <int>Text:
        return SubType.Text
    if <int>st == <int>Number:
        return SubType.Number
    elif <int>st == <int>Logical:
        return SubType.Logical
    elif <int>st == <int>Error:
        return SubType.Error
    elif <int>st == <int>Range:
        return SubType.Range
    if <int>st == <int>Math:
        return SubType.Math
    elif <int>st == <int>Concatenation:
        return SubType.Concatenation
    elif <int>st == <int>Intersection:
        return SubType.Intersection
    elif <int>st == <int>Union:
        return SubType.Union
    raise ValueError("Unexpected token subtype")


def tokenize(str formula):
    cdef _Options[wchar_t] options
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
