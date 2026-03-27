#ifndef IMENUPARSER_H
#define IMENUPARSER_H

#include "datamodel.h"
#include <QString>
#include <QJsonObject>

class IMenuParser
{
public:
    MenuNode parseFile(const QString& filePath);

private:
    MenuNode parseNode(const QJsonObject& obj);
    ParameterSpec parseParameter(const QJsonObject& obj);
    ActionSpec parseAction(const QJsonObject& obj);
    NodeType parseNodeType(const QString& typeStr);
};

#endif // IMENUPARSER_H
