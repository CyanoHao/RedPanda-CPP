#ifndef TOOLCHAINMANAGER_H
#define TOOLCHAINMANAGER_H

#include "toolchain.h"
#include <QList>
#include <QString>

class ToolchainManager {
public:
    ToolchainManager();

    QList<Toolchain> toolchains() const;
    PToolchain findById(const QString& id) const;
    PToolchain defaultToolchain() const;
    int defaultIndex() const;
    qint64 defaultIndexTimestamp() const;

    void setDefaultIndex(int index);
    void setDefaultToolchain(const QString& id);
    void addToolchain(const Toolchain& tc);
    void removeToolchain(const QString& id);
    void updateToolchain(const QString& id, const Toolchain& tc);

    void load(const QString& filename);
    void save(const QString& filename) const;

    int size() const;

    void discover();
    bool discoverFromFolder(const QString& folder);

private:
    QList<Toolchain> mToolchains;
    int mDefaultIndex = -1;
    qint64 mDefaultIndexTimestamp = 0;
};

#endif // TOOLCHAINMANAGER_H
