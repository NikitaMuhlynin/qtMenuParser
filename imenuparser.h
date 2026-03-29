#ifndef IMENUPARSER_H
#define IMENUPARSER_H

#include "datamodel.h"
#include <QString>
#include <istream>

class IMenuParser
{
public:
    MenuNode parseFile(const QString& filePath);
};

std::istream& operator>>(std::istream& in, ParameterSpec& param);
std::istream& operator>>(std::istream& in, ActionSpec& act);
std::istream& operator>>(std::istream& in, MenuNode& node);

#endif // IMENUPARSER_H