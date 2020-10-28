#include "mainframe.h"
#include "ui_mainframe.h"

MainFrame::MainFrame(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainFrame)
{
    ui->setupUi(this);
}

MainFrame::~MainFrame()
{
    delete ui;
}
