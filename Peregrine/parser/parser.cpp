#include "parser.hpp"
#include "lexer/lexer.hpp"
#include "lexer/tokens.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

std::map<TokenType, Precedence_type> create_map() {
    std::map<TokenType, Precedence_type> precedence_map;

    precedence_map[tk_negative] = pr_prefix;
    precedence_map[tk_bit_not] = pr_prefix;
    precedence_map[tk_not] = pr_prefix;
    precedence_map[tk_and] = pr_and_or;
    precedence_map[tk_or] = pr_and_or;
    precedence_map[tk_not] = pr_not;
    precedence_map[tk_assign] = pr_assign_compare;
    precedence_map[tk_not_equal] = pr_assign_compare;
    precedence_map[tk_greater] = pr_assign_compare;
    precedence_map[tk_less] = pr_assign_compare;
    precedence_map[tk_gr_or_equ] = pr_assign_compare;
    precedence_map[tk_less_or_equ] = pr_assign_compare;
    precedence_map[tk_plus] = pr_sum_minus;
    precedence_map[tk_minus] = pr_sum_minus;
    precedence_map[tk_multiply] = pr_mul_div;
    precedence_map[tk_divide] = pr_mul_div;

    return precedence_map;
}

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {
    m_current_token = tokens[0];
}

Parser::~Parser() {}

void Parser::advance() {
    m_tk_index++;

    if (m_tk_index < m_tokens.size()) {
        m_current_token = m_tokens[m_tk_index];
    }
}

Token Parser::next() {
    Token token;

    if (m_tk_index + 1 < m_tokens.size()) {
        token = m_tokens[m_tk_index + 1];
    }

    return token;
}

Precedence_type Parser::next_precedence() {
    if (precedence_map.count(next().tk_type) > 0) {
        return precedence_map[next().tk_type];
    }

    return pr_lowest;
}

void Parser::error(Token tok, std::string_view msg) {
    PEError err = {{tok.line, tok.start, m_filename, tok.statement},
                   std::string(msg),
                   "",
                   "",
                   ""};

    m_errors.push_back(err);
}

bool Parser::expect(TokenType expected_type) {
    if (next().tk_type != expected_type) {
        error(next(), "expected token of type " +
                          std::to_string(expected_type) + ", got " +
                          std::to_string(next().tk_type) + " instead");
    }

    advance();
}

AstNodePtr Parser::parse() {
    std::vector<AstNodePtr> statements;

    while (m_current_token.tk_type != tk_eof) {
        statements.push_back(ParseStatement());
        advance();
    }

    if (!m_errors.empty()) {
        for (auto& err : m_errors) {
            display(err);
        }

        exit(1);
    }

    return std::make_shared<Program>(statements);
}

AstNodePtr Parser::ParseStatement() {
    AstNodePtr stmt;

    switch (m_current_token.tk_type) {
        case tk_if: {
            stmt = ParseIf();
            break;
        }

        case tk_while: {
            stmt = ParseWhile();
            break;
        }

        case tk_def: {
            stmt = ParseFunctionDef();
            break;
        }

        default: {
            /*
                if it didn't match the statements above, then it must be
                either an expression or invalid
            */
            stmt = ParseExpression(pr_lowest);
            break;
        }
    }

    return stmt;
}

AstNodePtr Parser::ParseIf() {}

AstNodePtr Parser::ParseWhile() {}

AstNodePtr Parser::ParseFunctionDef() {}

AstNodePtr Parser::ParseExpression(Precedence_type curr_precedence) {
    AstNodePtr left;

    switch (m_current_token.tk_type) {
        case tk_integer: {
            left = ParseInteger();
            break;
        }

        case tk_decimal: {
            left = ParseDecimal();
            break;
        }

        case tk_none: {
            left = ParseNone();
            break;
        }

        case tk_string: {
            left = ParseString(false);
            break;
        }

        case tk_true:
        case tk_false: {
            left = ParseBool();
            break;
        }

        case tk_identifier: {
            left = ParseIdentifier();
            break;
        }

        case tk_l_paren: {
            left = ParseGroupedExpr();
            break;
        }

        case tk_negative:
        case tk_not:
        case tk_bit_not: {
            left = ParsePrefixExpression();
            break;
        }

        default: {
            error(m_current_token,
                  m_current_token.keyword + " is not an expression");
            break;
        }
    }

    while (next_precedence() > curr_precedence) {
        advance();
        left = ParseBinaryOperation(left);
    }

    return left;
}

AstNodePtr Parser::ParseBinaryOperation(AstNodePtr left) {
    std::string op = m_current_token.keyword;
    Precedence_type precedence = precedence_map[m_current_token.tk_type];

    advance();

    AstNodePtr right = ParseExpression(precedence);

    return std::make_shared<BinaryOperation>(left, op, right);
}

AstNodePtr Parser::ParsePrefixExpression() {
    std::string prefix = m_current_token.keyword;
    Precedence_type precedence = precedence_map[m_current_token.tk_type];

    advance();

    AstNodePtr right = ParseExpression(precedence);

    return std::make_shared<PrefixExpression>(prefix, right);
}

AstNodePtr Parser::ParseGroupedExpr() {
    advance();

    AstNodePtr expr = ParseExpression(pr_lowest);

    expect(tk_r_paren);

    return expr;
}

AstNodePtr Parser::ParseIdentifier() {
    return std::make_shared<IdentifierExpression>(m_current_token.keyword);
}