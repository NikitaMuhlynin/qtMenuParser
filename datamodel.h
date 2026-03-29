#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>
#include <QVariant>
#include <QList>
#include <optional>

enum class NodeType {
    Menu,
    SubMenu,
    Button,
    ParameterGroup,
    Parameter
};

struct ParameterSpec {
    QString name;
    QString type;
    QVariant defaultValue;
    QVariant minValue;
    QVariant maxValue;
};

struct ActionSpec {
    QString id;
    QString command;
};

struct ActionRequest {
    QString actionId;
    QMap<QString, QVariant> parameters;
};

struct MenuNode {
    QString id;
    QString title;
    NodeType type;

    QList<ParameterSpec> parameters;
    QList<MenuNode> children;

    std::optional<ActionSpec> action;
};

#endif // DATAMODEL_H
