#ifndef COMPILERBUILDMODE_H
#define COMPILERBUILDMODE_H

#include <QWidget>

namespace Ui {
class CompilerBuildMode;
}

class CompilerBuildMode : public QWidget
{
    Q_OBJECT

public:
    explicit CompilerBuildMode(QWidget *parent = nullptr);
    ~CompilerBuildMode();

private:
    Ui::CompilerBuildMode *ui;
};

#endif // COMPILERBUILDMODE_H
