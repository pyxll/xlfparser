from dataclasses import dataclass
from typing import TypedDict
from contextlib import contextmanager


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

    _user_defaults = {}

    @classmethod
    def __get_option(cls, option, kwargs: OptionsKwargs):
        return kwargs.get(option) or cls._user_defaults.get(option) or getattr(cls.Defaults, option)

    @classmethod
    def from_kwargs(cls, **kwargs: OptionsKwargs):
        return cls(
            left_brace=cls.__get_option("left_brace", kwargs),
            right_brace=cls.__get_option("right_brace", kwargs),
            left_bracket=cls.__get_option("left_bracket", kwargs),
            right_bracket=cls.__get_option("right_bracket", kwargs),
            list_separator=cls.__get_option("list_separator", kwargs),
            decimal_separator=cls.__get_option("decimal_separator", kwargs),
            row_separator=cls.__get_option("row_separator", kwargs),
        )


def set_defaults(**kwargs: OptionsKwargs):
    """Override the default options used by all functions."""
    for key, value in kwargs.items():
        if value is None:
            Options._user_defaults.pop(key, None)
        else:
            Options._user_defaults[key] = value


def reset_defaults():
    """Reset the default options back to their initial state"""
    Options._user_defaults.clear()


@contextmanager
def use_options(**kwargs: OptionsKwargs):
    """Context manager to set and restore default options"""
    prev_defaults = dict(Options._user_defaults)
    set_defaults(**kwargs)
    try:
        yield
    finally:
        reset_defaults()
        set_defaults(**prev_defaults)
