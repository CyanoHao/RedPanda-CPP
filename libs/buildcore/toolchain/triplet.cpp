#include "triplet.h"

bool tripletMatch(const QString &triplet, const QString &wildcard)
{
    QRegularExpression regex{QRegularExpression::wildcardToRegularExpression(wildcard)};
    return regex.match(triplet).hasMatch();
}

bool tripletMatch(const QString &triplet, const QRegularExpression &regex) {
    return regex.match(triplet).hasMatch();
}
