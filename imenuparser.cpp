#include "imenuparser.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <cctype>

namespace {

void skipWhitespace(std::istream& in) {
    while (true) {
        int ch = in.peek();
        if (ch == EOF || !std::isspace(static_cast<unsigned char>(ch)))
            break;
        in.get();
    }
}

char peekNonWhitespace(std::istream& in) {
    skipWhitespace(in);
    int ch = in.peek();
    if (ch == EOF)
        throw std::runtime_error("Unexpected end of file");
    return static_cast<char>(ch);
}

void expect(std::istream& in, char expected) {
    skipWhitespace(in);
    int ch = in.get();
    if (ch != expected) {
        throw std::runtime_error(
            std::string("Expected '") + expected + "', got " +
            (ch == EOF ? "EOF" : std::string("'") + static_cast<char>(ch) + "'")
            );
    }
}

bool tryConsume(std::istream& in, char expected) {
    skipWhitespace(in);
    if (in.peek() == expected) {
        in.get();
        return true;
    }
    return false;
}

void readLiteral(std::istream& in, const char* literal) {
    while (*literal) {
        int ch = in.get();
        if (ch != *literal)
            throw std::runtime_error("Invalid JSON literal");
        ++literal;
    }
}

QString readJsonString(std::istream& in) {
    skipWhitespace(in);

    if (in.get() != '"')
        throw std::runtime_error("Expected JSON string");

    QString result;

    while (true) {
        int ch = in.get();
        if (ch == EOF)
            throw std::runtime_error("Unexpected end of JSON string");

        char c = static_cast<char>(ch);

        if (c == '"')
            break;

        if (c == '\\') {
            int esc = in.get();
            if (esc == EOF)
                throw std::runtime_error("Unexpected end of escape sequence");

            switch (esc) {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case '/':  result += '/';  break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            default:
                throw std::runtime_error("Unsupported escape sequence");
            }
        } else {
            result += QChar::fromLatin1(c);
        }
    }

    return result;
}

QVariant readJsonValue(std::istream& in);

QVariant readJsonNumber(std::istream& in) {
    skipWhitespace(in);

    std::string number;

    if (in.peek() == '-')
        number += static_cast<char>(in.get());

    if (!std::isdigit(static_cast<unsigned char>(in.peek())))
        throw std::runtime_error("Invalid JSON number");

    if (in.peek() == '0') {
        number += static_cast<char>(in.get());
    } else {
        while (std::isdigit(static_cast<unsigned char>(in.peek())))
            number += static_cast<char>(in.get());
    }

    if (in.peek() == '.') {
        number += static_cast<char>(in.get());

        if (!std::isdigit(static_cast<unsigned char>(in.peek())))
            throw std::runtime_error("Invalid JSON number");

        while (std::isdigit(static_cast<unsigned char>(in.peek())))
            number += static_cast<char>(in.get());
    }

    if (in.peek() == 'e' || in.peek() == 'E') {
        number += static_cast<char>(in.get());

        if (in.peek() == '+' || in.peek() == '-')
            number += static_cast<char>(in.get());

        if (!std::isdigit(static_cast<unsigned char>(in.peek())))
            throw std::runtime_error("Invalid JSON number");

        while (std::isdigit(static_cast<unsigned char>(in.peek())))
            number += static_cast<char>(in.get());
    }

    QString qNumber = QString::fromStdString(number);
    bool ok = false;

    if (qNumber.contains('.') || qNumber.contains('e') || qNumber.contains('E')) {
        double value = qNumber.toDouble(&ok);
        if (!ok)
            throw std::runtime_error("Failed to parse floating-point number");
        return value;
    }

    qlonglong intValue = qNumber.toLongLong(&ok);
    if (ok)
        return intValue;

    double doubleValue = qNumber.toDouble(&ok);
    if (ok)
        return doubleValue;

    throw std::runtime_error("Failed to parse number");
}

QVariantMap readJsonObjectAsVariantMap(std::istream& in) {
    QVariantMap map;

    expect(in, '{');
    if (tryConsume(in, '}'))
        return map;

    do {
        QString key = readJsonString(in);
        expect(in, ':');
        map.insert(key, readJsonValue(in));
    } while(tryConsume(in, ','));

    expect(in, '}');
    return map;
}

QVariantList readJsonArrayAsVariantList(std::istream& in) {
    QVariantList list;

    expect(in, '[');
    if (tryConsume(in, ']'))
        return list;

    do {
        list.append(readJsonValue(in));
    } while (tryConsume(in, ','));

    expect(in, ']');
    return list;
}

QVariant readJsonValue(std::istream& in) {
    char ch = peekNonWhitespace(in);

    if (ch == '"')
        return readJsonString(in);

    if (ch == '{')
        return readJsonObjectAsVariantMap(in);

    if (ch == '[')
        return readJsonArrayAsVariantList(in);

    if (ch == 't') {
        readLiteral(in, "true");
        return true;
    }

    if (ch == 'f') {
        readLiteral(in, "false");
        return false;
    }

    if (ch == 'n') {
        readLiteral(in, "null");
        return QVariant();
    }

    if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)))
        return readJsonNumber(in);

    throw std::runtime_error("Unsupported JSON value");
}

void skipJsonValue(std::istream& in) {
    (void)readJsonValue(in);
}

NodeType parseNodeType(const QString& typeStr) {
    if (typeStr == "menu") return NodeType::Menu;
    if (typeStr == "submenu") return NodeType::SubMenu;
    if (typeStr == "button") return NodeType::Button;
    if (typeStr == "parametergroup") return NodeType::ParameterGroup;
    if (typeStr == "parameter") return NodeType::Parameter;
    return NodeType::Menu;
}

void readParameterArray(std::istream& in, QList<ParameterSpec>& parameters){
    parameters.clear();

    expect(in, '[');
    if (tryConsume(in, ']'))
        return;

    do {
        ParameterSpec param;
        in >> param;
        parameters.append(param);
    } while (tryConsume(in, ','));

    expect(in, ']');
}

void readChildrenArray(std::istream& in, QList<MenuNode>& children) {
    children.clear();

    expect(in, '[');
    if (tryConsume(in, ']'))
        return;

    do {
        MenuNode child;
        in >> child;
        children.append(child);
    } while (tryConsume(in, ','));

    expect(in, ']');
}

} // namespace

MenuNode IMenuParser::parseFile(const QString& filePath) {
    std::ifstream file(filePath.toStdString(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open .menucfg file");

    MenuNode root;
    file >> root;

    skipWhitespace(file);
    if (file.peek() != EOF)
        throw std::runtime_error("Unexpected characters after root JSON object");

    return root;
}

std::istream& operator>>(std::istream& in, ParameterSpec& param) {
    param = ParameterSpec{};

    expect(in, '{');
    if (tryConsume(in, '}'))
        return in;

    do {
        QString key = readJsonString(in);
        expect(in, ':');

        if (key == "name") {
            param.name = readJsonString(in);
        } else if (key == "type") {
            param.type = readJsonString(in);
        } else if (key == "default") {
            param.defaultValue = readJsonValue(in);
        } else if (key == "min") {
            param.minValue = readJsonValue(in);
        } else if (key == "max") {
            param.maxValue = readJsonValue(in);
        } else {
            skipJsonValue(in);
        }
    } while (tryConsume(in, ','));

    expect(in, '}');
    return in;
}

std::istream& operator>>(std::istream& in, ActionSpec& act) {
    act = ActionSpec{};

    expect(in, '{');
    if (tryConsume(in, '}'))
        return in;

    do {
        QString key = readJsonString(in);
        expect(in, ':');

        if (key == "id") {
            act.id = readJsonString(in);
        } else if (key == "command") {
            act.command = readJsonString(in);
        } else {
            skipJsonValue(in);
        }
    } while (tryConsume(in, ','));

    expect(in, '}');
    return in;
}

std::istream& operator>>(std::istream& in, MenuNode& node) {
    node = MenuNode{};
    node.type = NodeType::Menu;

    expect(in, '{');
    if (tryConsume(in, '}'))
        return in;

    do {
        QString key = readJsonString(in);
        expect(in, ':');

        if (key == "id") {
            node.id = readJsonString(in);
        } else if (key == "title") {
            node.title = readJsonString(in);
        } else if (key == "type") {
            node.type = parseNodeType(readJsonString(in));
        } else if (key == "parameters") {
            readParameterArray(in, node.parameters);
        } else if (key == "children") {
            readChildrenArray(in, node.children);
        } else if (key == "action") {
            if (peekNonWhitespace(in) == 'n') {
                readLiteral(in, "null");
                node.action.reset();
            } else {
                ActionSpec action;
                in >> action;
                node.action = action;
            }
        } else {
            skipJsonValue(in);
        }
    } while (tryConsume(in, ','));

    expect(in, '}');
    return in;
}





























