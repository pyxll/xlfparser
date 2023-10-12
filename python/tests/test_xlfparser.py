from xlfparser import tokenize, stringify, build_ast, Token


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
    assert str(ast) == "3 * 4 + 5"

    ast = build_ast(tokenize("=-1"))
    assert str(ast) == "- 1"

    ast = build_ast(tokenize("=1%"))
    assert str(ast) == "1 %"

    ast = build_ast(tokenize("=SUM(1, 2)"))
    assert str(ast) == "SUM( 1 , 2 )"

    ast = build_ast(tokenize("=1+(2*-3)"))
    assert str(ast) == "1 + ( 2 * - 3 )"

    ast = build_ast(tokenize("={1,2,3;4,5,6}"))
    assert str(ast) == "{ 1 , 2 , 3 ; 4 , 5 , 6 }"

    ast = build_ast(tokenize("={FUNC(-1, 2*3, 4%, (5 / 6))}"))
    assert str(ast) == "{ FUNC( - 1 , 2 * 3 , 4 % , ( 5 / 6 ) ) }"
