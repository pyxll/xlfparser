from dataclasses import dataclass
from typing import TypedDict


class OptionsKwargs(TypedDict):
    left_brace: str
    right_brace: str
    left_bracket: str
    right_bracket: str
    list_separator: str
    decimal_separator: str
    row_separator: str


@dataclass
class Options:
    left_brace: str
    right_brace: str
    left_bracket: str
    right_bracket: str
    list_separator: str
    decimal_separator: str
    row_separator: str

    class Defaults:
        left_brace = "{"
        right_brace = "}"
        left_bracket = "["
        right_bracket = "]"
        list_separator = ","
        decimal_separator = "."
        row_separator = ";"

    @classmethod
    def from_kwargs(cls, **kwargs: OptionsKwargs):
        return cls(
            left_brace=kwargs.get("left_brace") or cls.Defaults.left_brace, 
            right_brace=kwargs.get("right_brace") or cls.Defaults.right_brace, 
            left_bracket=kwargs.get("left_bracket") or cls.Defaults.left_bracket, 
            right_bracket=kwargs.get("right_bracket") or cls.Defaults.right_bracket, 
            list_separator=kwargs.get("list_separator") or cls.Defaults.list_separator, 
            decimal_separator=kwargs.get("decimal_separator") or cls.Defaults.decimal_separator, 
            row_separator=kwargs.get("row_separator") or cls.Defaults.row_separator
        )
