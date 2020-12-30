#include <cassert>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

struct Token {
    enum class Type {
        // variable
        Variable = 0,

        // braces
        BraceOpen = 1,
        BraceClose = 2,

        // logical operators
        LogicNot = -10,
        LogicAnd = 10,
        LogicOr = 20,
        LogicImplies = 30,

        // temporal operators
        TemporalNext = 100,
        TemporalGlobal = 200,
        TemporalFuture = 300,
        TemporalUntil = 400,
        TemporalRelease = 500,

        // eof
        End = -1,
    } type;

    int arg1{0};
};

enum class ErrorCode {
    BadLexeme = 100,
    BraceBalance = 200,
    IOError = -200,
    ParseError = 1000,
};

class ParserError : public std::exception {
  public:
    ErrorCode ec;

    virtual const char* what() const noexcept
    {
        switch (ec) {
            case ErrorCode::BadLexeme: return "bad lexeme";
            case ErrorCode::BraceBalance: return "wrong brace balance";
            case ErrorCode::IOError: return "I/O error";
            case ErrorCode::ParseError: return "failed to parse";
            default: assert(false && "unknown error code");
        };
    }

    ParserError(ErrorCode ec_) : ec(ec_) {}
};

class VariableMap {
  private:
    std::map<std::string, int> id_{};
    std::map<int, std::string> name_{};
    int last_id_{0};

  public:
    int get_id(const std::string& name)
    {
        if (id_.count(name) == 0U) {
            id_[name] = ++last_id_;
            name_[last_id_] = name;
        }
        return id_[name];
    }

    const std::string& get_name(int id) const { return name_.at(id); }
} vm;

class Lexer {
  private:
    std::istream& in_;

    char try_get_()
    {
        try {
            return in_.get();
        } catch (std::ios_base::failure const&) {
            throw ParserError(ErrorCode::IOError);
        }
    }

    static bool is_name_(char ch)
    {
        return ch == '_' || ('a' <= ch && ch <= 'z');
    }

  public:
    Lexer(std::istream& in) : in_(in) {}

    Lexer(const Lexer&) = delete;
    Lexer(Lexer&&) = delete;

    Lexer& operator=(const Lexer&) = delete;
    Lexer& operator=(Lexer&&) = delete;

    ~Lexer() = default;

    std::istream::pos_type mark() const { return in_.tellg(); }

    void reset(std::istream::pos_type pos) { in_.seekg(pos); }

    Token next()
    {
        char ch;
        while (isspace(ch = try_get_())) {}

        switch (ch) {
            case std::istream::traits_type::eof(): return {Token::Type::End};

            case '(': return {Token::Type::BraceOpen};
            case ')': return {Token::Type::BraceClose};

            case '&':
                if (try_get_() == '&') return {Token::Type::LogicAnd};
                throw ParserError(ErrorCode::BadLexeme);

            case '|':
                if (try_get_() == '|') return {Token::Type::LogicOr};
                throw ParserError(ErrorCode::BadLexeme);

            case '-':
                if (try_get_() == '>') return {Token::Type::LogicImplies};
                throw ParserError(ErrorCode::BadLexeme);

            case 'X': return {Token::Type::TemporalNext};
            case 'F': return {Token::Type::TemporalFuture};
            case 'G': return {Token::Type::TemporalGlobal};
            case 'U': return {Token::Type::TemporalUntil};
            case 'R': return {Token::Type::TemporalRelease};
            case '!': return {Token::Type::LogicNot};

            default: {
                if (!is_name_(ch)) {
                    throw ParserError(ErrorCode::BadLexeme);
                }

                std::string name{ch};
                while (true) {
                    auto mark1 = mark();
                    if (char next_ch = try_get_(); is_name_(next_ch)) {
                        name += next_ch;
                    } else {
                        reset(mark1);
                        break;
                    }
                }
                return {Token::Type::Variable, vm.get_id(name)};
            }
        }
    }
};

struct Term {
    enum class Type {
        // prop var
        Prop = 0,

        // logical operators
        LogicNot = -10,
        LogicAnd = 10,
        LogicOr = 20,
        LogicImplies = 30,

        // temporal operators
        TemporalNext = 100,
        TemporalGlobal = 200,
        TemporalFuture = 300,
        TemporalUntil = 400,
        TemporalRelease = 500,
    } type;

    std::vector<std::unique_ptr<Term>> args{};
    int arg3{0};
};

/*
    // TODO: add Until, Release
    C >> I {-> I}
    D >> C {&& C}*
    E >> D {|| D}*
    I >> P , (E) , XI , FI , GI , !I
    TERM >> I
*/
class Parser {
  private:
    Lexer& lexer_;

    // std::optional<Term> parse_clause_();

    std::optional<Term> parse_implication_()
    {
        auto mark = lexer_.mark();
        Term result{Term::Type::LogicImplies};
        std::optional<Term> lhs, rhs;

        if (lhs = parse_clause_(); !lhs) goto fail;
        if (auto mark1 = lexer_.mark();
            lexer_.next().type != Token::Type::LogicImplies) {
            lexer_.reset(mark1);
            return *std::move(lhs);
        }
        if (rhs = parse_clause_(); !rhs) goto fail;

        result.args.push_back(std::make_unique<Term>(std::move(*lhs)));
        result.args.push_back(std::make_unique<Term>(std::move(*rhs)));
        return result;
    fail:
        lexer_.reset(mark);
        return {};
    }

    std::optional<Term> parse_conjunction_()
    {
        auto mark = lexer_.mark();
        Term result{Term::Type::LogicAnd};

        auto term = parse_implication_();
        if (!term) goto fail;
        result.args.push_back(std::make_unique<Term>(std::move(*term)));

        while (true) {
            if (auto mark1 = lexer_.mark();
                lexer_.next().type != Token::Type::LogicAnd) {
                lexer_.reset(mark1);
                break;
            } else {
                if (term = parse_implication_(); !term) goto fail;
                result.args.push_back(
                    std::make_unique<Term>(std::move(*term)));
            }
        }
        return result.args.size() > 1 ? std::move(result)
                                      : std::move(*result.args[0]);
    fail:
        lexer_.reset(mark);
        return {};
    }

    std::optional<Term> parse_expression_()
    {
        auto mark = lexer_.mark();
        Term result{Term::Type::LogicOr};

        auto term = parse_conjunction_();
        if (!term) goto fail;
        result.args.push_back(std::make_unique<Term>(std::move(*term)));

        while (true) {
            if (auto mark1 = lexer_.mark();
                lexer_.next().type != Token::Type::LogicOr) {
                lexer_.reset(mark1);
                break;
            } else {
                if (term = parse_implication_(); !term) goto fail;
                result.args.push_back(
                    std::make_unique<Term>(std::move(*term)));
            }
        }
        return result.args.size() > 1 ? std::move(result)
                                      : std::move(*result.args[0]);
    fail:
        lexer_.reset(mark);
        return {};
    }

    std::optional<Term> parse_clause_()
    {
        auto mark = lexer_.mark();
        auto lexeme = lexer_.next();
        switch (lexeme.type) {
            case Token::Type::Variable:
                return Term{Term::Type::Prop, {}, lexeme.arg1};
            case Token::Type::BraceOpen: {
                auto term = parse_expression_();
                if (!term || lexer_.next().type != Token::Type::BraceClose) {
                    goto fail;
                }
                return *std::move(term);
            }
            case Token::Type::TemporalNext:
            case Token::Type::TemporalFuture:
            case Token::Type::TemporalGlobal:
            case Token::Type::LogicNot: {
                auto term = parse_clause_();
                if (!term) goto fail;
                Term result{static_cast<Term::Type>(lexeme.type)};
                result.args.push_back(
                    std::make_unique<Term>(std::move(*term)));
                return result;
            }
            default: goto fail;
        }
    fail:
        lexer_.reset(mark);
        return {};
    }

    std::optional<Term> parse_term_() { return parse_expression_(); }

  public:
    Parser(Lexer& lexer) : lexer_(lexer) {}

    ~Parser() = default;

    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;

    Parser& operator=(const Parser&) = delete;
    Parser& operator=(Parser&&) = delete;

    Term parse()
    {
        auto term = parse_term_();
        if (!term) throw ParserError(ErrorCode::ParseError);
        return *std::move(term);
    }
};

const char* op_mnemonic(Term::Type op)
{
    switch (op) {
        case Term::Type::LogicNot: return "!";
        case Term::Type::LogicOr: return "||";
        case Term::Type::LogicAnd: return "&&";
        case Term::Type::LogicImplies: return "->";
        case Term::Type::TemporalNext: return "X";
        case Term::Type::TemporalFuture: return "F";
        case Term::Type::TemporalGlobal: return "G";
        default: assert(0 && "no known mnemonic");
    }
}

void dump_term_recursive(const Term& term, std::ostream& out)
{
    switch (term.type) {
        case Term::Type::Prop:
            out << reinterpret_cast<uintptr_t>(&term) << " "
                << "[label = \"" << vm.get_name(term.arg3) << "\" "
                << "color = blue];" << std::endl;
            break;
        case Term::Type::LogicNot:
        case Term::Type::TemporalNext:
        case Term::Type::TemporalFuture:
        case Term::Type::TemporalGlobal:
            out << reinterpret_cast<uintptr_t>(&term) << " "
                << "[label = \"" << op_mnemonic(term.type) << "\" "
                << "color = red];" << std::endl;
            dump_term_recursive(*term.args[0], out);
            out << reinterpret_cast<uintptr_t>(&term) << " -> { "
                << reinterpret_cast<uintptr_t>(&*term.args[0]) << " } "
                << "[color = black];" << std::endl;
            break;
        case Term::Type::LogicAnd:
        case Term::Type::LogicOr:
        case Term::Type::LogicImplies:
            out << reinterpret_cast<uintptr_t>(&term) << " "
                << "[label = \"" << op_mnemonic(term.type) << "\" "
                << "color = green];" << std::endl;
            for (const auto& arg : term.args) {
                dump_term_recursive(*arg, out);
                out << reinterpret_cast<uintptr_t>(&term) << " -> { "
                    << reinterpret_cast<uintptr_t>(&*arg) << " } "
                    << "[color = black];" << std::endl;
            }
            break;
        default: assert(0);
    }
}

void dump_term(const Term& term, std::ostream& out)
{
    out << "digraph G {" << std::endl;
    dump_term_recursive(term, out);
    out << "}" << std::endl;
}

int main()
{
    std::string prog = "(G (kek -> lol) && F kek) -> F lol";
    std::istringstream in(prog);
    try {
        Lexer lexer(in);
        Parser parser(lexer);
        auto term = parser.parse();
        dump_term(term, std::cout);
    } catch (std::exception const& exc) {
        std::cout << exc.what() << std::endl;
    }
}
