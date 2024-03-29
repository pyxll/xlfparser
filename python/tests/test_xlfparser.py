import pytest
from xlfparser import tokenize, stringify, build_ast, Token, use_options


def test_tokenize_formula():
    tokens = tokenize("=3*4+5")
    assert len(tokens) == 5
    assert tokens[0].value == "3"
    assert tokens[0].type == Token.Type.Operand
    assert tokens[1].value == "*"
    assert tokens[1].type == Token.Type.OperatorInfix
    assert tokens[2].value == "4"
    assert tokens[2].type == Token.Type.Operand
    assert tokens[3].value == "+"
    assert tokens[3].type == Token.Type.OperatorInfix
    assert tokens[4].value == "5"
    assert tokens[4].type == Token.Type.Operand


def test_tokenize_function():
    tokens = tokenize("=SUM(1, 2)")
    assert len(tokens) == 5
    assert tokens[0].value == "SUM"
    assert tokens[0].type == Token.Type.Function
    assert tokens[0].sub_type == Token.SubType.Start
    assert tokens[1].value == "1"
    assert tokens[1].type == Token.Type.Operand
    assert tokens[2].value == ","
    assert tokens[2].type == Token.Type.Argument
    assert tokens[3].value == "2"
    assert tokens[3].type == Token.Type.Operand
    assert tokens[4].value == ")"
    assert tokens[4].type == Token.Type.Function
    assert tokens[4].sub_type == Token.SubType.Stop


def test_tokenize_with_options():
    tokens = tokenize("=SUM(1; 2)", list_separator=";")
    assert len(tokens) == 5
    assert tokens[0].value == "SUM"
    assert tokens[0].type == Token.Type.Function
    assert tokens[0].sub_type == Token.SubType.Start
    assert tokens[1].value == "1"
    assert tokens[1].type == Token.Type.Operand
    assert tokens[2].value == ";"
    assert tokens[2].type == Token.Type.Argument
    assert tokens[3].value == "2"
    assert tokens[3].type == Token.Type.Operand
    assert tokens[4].value == ")"
    assert tokens[4].type == Token.Type.Function
    assert tokens[4].sub_type == Token.SubType.Stop


def test_stringify():
    tokens = tokenize("=3*4+5")
    assert stringify(tokens) == "=3*4+5"

    tokens = tokenize("=-1%")
    assert stringify(tokens) == "=-1%"

    tokens = tokenize("=SUM(A1:A2)")
    assert stringify(tokens) == "=SUM(A1:A2)"

    tokens = tokenize("={FUNC(-1, 2*3, 4%, (5 / 6));1,2,3}")
    assert stringify(tokens) == "={FUNC(-1,2*3,4%,(5/6));1,2,3}"
    assert stringify(tokens, list_separator="_") == "={FUNC(-1_2*3_4%_(5/6));1_2_3}"


def test_ast_builder():
    ast = build_ast(tokenize("=3*4+5"))
    assert ast.to_string(separator=" ") == "= 3 * 4 + 5"
    assert stringify(ast.tokens) == "=3*4+5"

    ast = build_ast(tokenize("=-1"))
    assert ast.to_string(separator=" ") == "= - 1"
    assert stringify(ast.tokens) == "=-1"

    ast = build_ast(tokenize("=1%"))
    assert ast.to_string(separator=" ") == "= 1 %"
    assert stringify(ast.tokens) == "=1%"

    ast = build_ast(tokenize("=SUM(1, 2)"))
    assert ast.to_string(separator=" ") == "= SUM( 1 , 2 )"
    assert stringify(ast.tokens) == "=SUM(1,2)"

    ast = build_ast(tokenize("=1+(2*-3)"))
    assert ast.to_string(separator=" ") == "= 1 + ( 2 * - 3 )"
    assert stringify(ast.tokens) == "=1+(2*-3)"

    ast = build_ast(tokenize("={1,2,3;4,5,6}"))
    assert ast.to_string(separator=" ") == "= { 1 , 2 , 3 ; 4 , 5 , 6 }"
    assert stringify(ast.tokens) == "={1,2,3;4,5,6}"

    ast = build_ast(tokenize("={FUNC(-1, 2*3, 4%, (5 / 6))}"))
    assert ast.to_string(separator=" ") == "= { FUNC( - 1 , 2 * 3 , 4 % , ( 5 / 6 ) ) }"
    assert stringify(ast.tokens) == "={FUNC(-1,2*3,4%,(5/6))}"


def test_invalid_formulas():
    with pytest.raises(RuntimeError) as e:
        tokenize("=}")
    assert "Mismatched braces" in str(e)

    with pytest.raises(RuntimeError) as e:
        tokenize("={1,2,3}}")
    assert "Mismatched braces" in str(e)

    with pytest.raises(RuntimeError) as e:
        tokenize("=)")
    assert "Mismatched parentheses" in str(e)

    with pytest.raises(RuntimeError) as e:
        tokenize("=foo())")
    assert "Mismatched parentheses" in str(e)


def test_with_options():
    with use_options(list_separator=";"):
        tokens = tokenize("=FUNC(1; 2; 3)")

    with use_options(list_separator="|"):
        assert stringify(tokens) == "=FUNC(1|2|3)"
