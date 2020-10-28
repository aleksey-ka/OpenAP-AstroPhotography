#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QMainWindow>

namespace Ui {
class MainFrame;
}

class MainFrame : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = nullptr);
    ~MainFrame();

private slots:
    void on_toggleFullScreenButton_clicked();

    void on_captureFrameButton_clicked();

private:
    Ui::MainFrame *ui;

    ulong render( const ushort* raw, int width, int height );
};

#endif // MAINFRAME_H
