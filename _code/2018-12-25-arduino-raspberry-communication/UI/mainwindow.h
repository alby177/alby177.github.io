#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    enum SocketState
    {
        connection,
        disconnection
    };

signals:
    void ReturnPressed();

private slots:
    void on_ipEdit_editingFinished();
    void on_connButton_clicked();
    void on_sendButton_clicked();

private:
    void setUI(SocketState state);
    void keyPressEvent(QKeyEvent *event);
    void disconnectTcp();


private:
    Ui::MainWindow *ui;
    QSettings settings;
    QTcpSocket socket {this};
};

#endif // MAINWINDOW_H
