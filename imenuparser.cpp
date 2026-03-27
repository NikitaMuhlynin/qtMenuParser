#include "imenuparser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <stdexcept>

MenuNode IMenuParser::parseFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Cannot open .menucfg file");

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject())
        throw std::runtime_error("Invalid menucfg format");

    return parseNode(doc.object());
}

MenuNode IMenuParser::parseNode(const QJsonObject& obj) {
    MenuNode node;
    node.id = obj.value("id").toString();
    node.title = obj.value("title").toString();
    node.type = parseNodeType(obj.value("type").toString());

    if (obj.contains("parameters") && obj.value("parameters").isArray()) {
        QJsonArray params = obj.value("parameters").toArray();
        for (const QJsonValue& value : params) {
            if (value.isObject()) {
                node.parameters.append(parseParameter(value.toObject()));
            }
        }
    }

    if (obj.contains("children") && obj.value("children").isArray()) {
        QJsonArray children = obj.value("children").toArray();
        for (const QJsonValue& value : children) {
            if (value.isObject()) {
                node.children.append(parseNode(value.toObject()));
            }
        }
    }

    if (obj.contains("action") && obj.value("action").isObject()) {
        node.action = parseAction(obj.value("action").toObject());
    }

    return node;
}

ParameterSpec IMenuParser::parseParameter(const QJsonObject& obj) {
    ParameterSpec param;
    param.name = obj.value("name").toString();
    param.type = obj.value("type").toString();

    if (obj.contains("default")) param.defaultValue = obj.value("default").toVariant();
    if (obj.contains("min")) param.minValue = obj.value("min").toVariant();
    if (obj.contains("max")) param.maxValue = obj.value("max").toVariant();

    return param;
}

ActionSpec IMenuParser::parseAction(const QJsonObject& obj){
    ActionSpec act;
    act.id = obj.value("id").toString();
    act.command = obj.value("command").toString();

    return act;
}

NodeType IMenuParser::parseNodeType(const QString& typeStr) {
    if (typeStr == "menu") return NodeType::Menu;
    if (typeStr == "submenu") return NodeType::SubMenu;
    if (typeStr == "button") return NodeType::Button;
    if (typeStr == "parametergroup") return NodeType::ParameterGroup;
    if (typeStr == "parameter") return NodeType::Parameter;

    return NodeType::Menu;
}