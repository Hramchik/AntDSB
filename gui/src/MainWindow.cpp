//
// Created by sanek on 13/12/2025.
//

#include "MainWindow.h"
#include "BotClient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>

class MainWindowUi {
public:
    QWidget* central = nullptr;
    QLineEdit* addressEdit = nullptr;
    QLabel* statusLabel = nullptr;
    QLineEdit* channelEdit = nullptr;
    QPlainTextEdit* messageEdit = nullptr;

    QPushButton* sendButton = nullptr;
    QPushButton* startButton = nullptr;
    QPushButton* stopButton = nullptr;
    QPushButton* restartButton = nullptr;

    void setupUi(QMainWindow* w) {
        central = new QWidget(w);
        auto* mainLayout = new QVBoxLayout(central);

        auto* addrLayout = new QHBoxLayout;
        addrLayout->addWidget(new QLabel("gRPC address:", central));
        addressEdit = new QLineEdit("localhost:50051", central);
        addrLayout->addWidget(addressEdit);
        mainLayout->addLayout(addrLayout);

        statusLabel = new QLabel("Status: unknown", central);
        mainLayout->addWidget(statusLabel);

        auto* chanLayout = new QHBoxLayout;
        chanLayout->addWidget(new QLabel("Channel ID:", central));
        channelEdit = new QLineEdit(central);
        chanLayout->addWidget(channelEdit);
        mainLayout->addLayout(chanLayout);

        messageEdit = new QPlainTextEdit(central);
        messageEdit->setPlaceholderText("Message text...");
        mainLayout->addWidget(messageEdit);

        sendButton = new QPushButton("Send", central);
        mainLayout->addWidget(sendButton);

        auto* ctrlLayout = new QHBoxLayout;
        startButton   = new QPushButton("Start bot",   central);
        stopButton    = new QPushButton("Stop bot",    central);
        restartButton = new QPushButton("Restart bot", central);
        ctrlLayout->addWidget(startButton);
        ctrlLayout->addWidget(stopButton);
        ctrlLayout->addWidget(restartButton);
        mainLayout->addLayout(ctrlLayout);

        w->setCentralWidget(central);
        w->setWindowTitle("AntDSB GUI");
        w->resize(500, 400);
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowUi>()) {

    ui->setupUi(this);

    connect(ui->sendButton,    &QPushButton::clicked,
            this,              &MainWindow::onSendClicked);
    connect(ui->startButton,   &QPushButton::clicked,
            this,              &MainWindow::onStartBotClicked);
    connect(ui->stopButton,    &QPushButton::clicked,
            this,              &MainWindow::onStopBotClicked);
    connect(ui->restartButton, &QPushButton::clicked,
            this,              &MainWindow::onRestartBotClicked);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRefreshStatus);
    timer->start(2000);
    onRefreshStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::onRefreshStatus() {
    const std::string addr = ui->addressEdit->text().toStdString();

    if (!client) {
        client = std::make_unique<BotClient>(addr);
    }

    antdsb::StatusReply st;
    bool ok = client->TryGetStatus(st);
    if (!ok) {
        consecutiveErrors++;
        client.reset();
    } else {
        consecutiveErrors = 0;
    }

    QString text = QString("Status: %1").arg(st.running() ? "running" : "stopped");
    if (!st.last_error().empty()) {
        text += QString(" | error: %1").arg(QString::fromStdString(st.last_error()));
    }
    ui->statusLabel->setText(text);
}

void MainWindow::onSendClicked() {
    bool ok = false;
    quint64 channelId = ui->channelEdit->text().toULongLong(&ok);
    if (!ok) {
        ui->statusLabel->setText("Status: invalid channel id");
        return;
    }

    if (!client) {
        client = std::make_unique<BotClient>(ui->addressEdit->text().toStdString());
    }

    std::string msg = ui->messageEdit->toPlainText().toStdString();
    std::string err;
    if (!client->SendMessage(channelId, msg, err)) {
        ui->statusLabel->setText(
            QString("Status: send failed (%1)").arg(QString::fromStdString(err)));
    } else {
        ui->statusLabel->setText("Status: message sent");
    }
}

void MainWindow::onStartBotClicked() {
    if (!client) {
        client = std::make_unique<BotClient>(ui->addressEdit->text().toStdString());
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->StartBot(st, err)) {
        ui->statusLabel->setText(
            QString("Status: start failed (%1)").arg(QString::fromStdString(err)));
        client.reset();
        return;
    }

    QString text = QString("Status: %1").arg(st.running() ? "running" : "stopped");
    if (!st.last_error().empty()) {
        text += QString(" | error: %1").arg(QString::fromStdString(st.last_error()));
    }
    ui->statusLabel->setText(text);
}

void MainWindow::onStopBotClicked() {
    if (!client) {
        client = std::make_unique<BotClient>(ui->addressEdit->text().toStdString());
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->StopBot(st, err)) {
        ui->statusLabel->setText(
            QString("Status: stop failed (%1)").arg(QString::fromStdString(err)));
        client.reset();
        return;
    }

    QString text = QString("Status: %1").arg(st.running() ? "running" : "stopped");
    if (!st.last_error().empty()) {
        text += QString(" | error: %1").arg(QString::fromStdString(st.last_error()));
    }
    ui->statusLabel->setText(text);
}

void MainWindow::onRestartBotClicked() {
    if (!client) {
        client = std::make_unique<BotClient>(ui->addressEdit->text().toStdString());
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->RestartBot(st, err)) {
        ui->statusLabel->setText(
            QString("Status: restart failed (%1)").arg(QString::fromStdString(err)));
        client.reset();
        return;
    }

    QString text = QString("Status: %1").arg(st.running() ? "running" : "stopped");
    if (!st.last_error().empty()) {
        text += QString(" | error: %1").arg(QString::fromStdString(st.last_error()));
    }
    ui->statusLabel->setText(text);
}
