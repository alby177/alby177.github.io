#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "constants.h"
#include <QHostAddress>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Settings parameters
    ui->ipEdit->setText(settings.value(ipAddress, "127.0.0.1").toString());

    // Graphical settings
    setUI(disconnection);
    ui->plainTextEdit->setReadOnly(true);

    // Setup read
    connect(&socket, &QTcpSocket::readyRead, [this]()
    {
       // Read from server
       QString readData {socket.readAll()};

       // Display data
       ui->plainTextEdit->insertPlainText("From server: " + readData + "\n");
    });

    // Catch return key press
    connect(this, SIGNAL(ReturnPressed()), this, SLOT(on_sendButton_clicked()));

    // Manage disconnection
    connect(ui->discButton, &QPushButton::clicked, [this](){disconnectTcp();});
    connect(&socket, &QTcpSocket::disconnected, [this]()
    {
        // Change button enabled
        setUI(disconnection);

        // Insert log message
        ui->plainTextEdit->insertPlainText("Disconnected from server at address: " + (socket.peerAddress()).toString() + "\n");

        disconnectTcp();
    });
}

MainWindow::~MainWindow()
{
    // Close socket
    socket.close();
    delete ui;
}

void MainWindow::on_ipEdit_editingFinished()
{
    // Store inserted IP value
    settings.setValue(ipAddress, ui->ipEdit->text());
}


void MainWindow::on_connButton_clicked()
{
    // Connect to ip address
    socket.connectToHost(ui->ipEdit->text(), 5001);

    // Check for connection
    if (socket.waitForConnected())
    {
        // Graphical settings
        setUI(connection);

        // Insert log message
        ui->plainTextEdit->insertPlainText("Connected to server at address: " + (socket.peerAddress()).toString() + "\n");
    }
}

void MainWindow::on_sendButton_clicked()
{
    // Save user data
    QString userData    {ui->messageEdit->text()};

    // Send to server data
    socket.write(userData.toUtf8());

    // Insert log message
    ui->plainTextEdit->insertPlainText("Sent to server: " + userData + "\n");

    // Clear message box
    ui->messageEdit->clear();
}

void MainWindow::setUI(MainWindow::SocketState state)
{
    switch (state) {

    // Client disconnection
    case disconnection:
        ui->connButton->setEnabled(true);
        ui->discButton->setEnabled(false);
        ui->sendButton->setEnabled(false);
        break;

    // Client connection
    case connection:
        ui->connButton->setEnabled(false);
        ui->discButton->setEnabled(true);
        ui->sendButton->setEnabled(true);
        ui->statusBar->clearMessage();
        ui->statusBar->showMessage("Connected");
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    {
        // Check for return key press when text to send is inserted and client connected
        if( (event->key() == Qt::Key_Return) && (ui->messageEdit->text() != "") && (socket.state() == QTcpSocket::ConnectedState))
        {
            emit ReturnPressed(); //
        }
    }
}

void MainWindow::disconnectTcp()
{
    // Check for client connected
    if (socket.state() == QTcpSocket::ConnectedState)
    {
        // Send to server data for disconnection
        socket.write(QString("Disconnect").toUtf8());

        // Disconnect client
        socket.disconnectFromHost();
    }
}
