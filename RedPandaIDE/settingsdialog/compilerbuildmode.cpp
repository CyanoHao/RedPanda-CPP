#include "compilerbuildmode.h"
#include "ui_compilerbuildmode.h"

CompilerBuildMode::CompilerBuildMode(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CompilerBuildMode)
{
    ui->setupUi(this);
}

CompilerBuildMode::~CompilerBuildMode()
{
    delete ui;
}
