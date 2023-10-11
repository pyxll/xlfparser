from dataclasses import dataclass
from enum import Enum


class Type(Enum):
    Unknown = 0
    Operand = 1
    Function = 2
    Array = 3
    ArrayRow = 4
    Subexpression = 5
    Argument = 6
    OperatorPrefix = 7
    OperatorInfix = 8
    OperatorPostfix = 9
    Whitespace = 10


class SubType(Enum):
    _None = 0
    Start = 1
    Stop = 2
    Text = 3
    Number = 4
    Logical = 5
    Error = 6
    Range = 7
    Math = 8
    Concatenation = 9
    Intersection = 10
    Union = 11


@dataclass
class Token:
    value: str
    type: Type
    sub_type: SubType
