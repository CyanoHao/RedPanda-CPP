#ifndef TOOLCHAIN_TRIPLET_H
#define TOOLCHAIN_TRIPLET_H

#include <QString>
#include <QRegularExpression>

bool tripletMatch(const QString &triplet, const QString &wildcard);
bool tripletMatch(const QString &triplet, const QRegularExpression &regex);

#endif // TRIPLET_HPP
