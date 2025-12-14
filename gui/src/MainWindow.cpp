//
// Created by sanek on 13/12/2025.
//

#include "MainWindow.h"
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

        w->setCentralWidget(central);
        w->setWindowTitle("AntDSB GUI");
        w->resize(500, 400);
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowUi>()) {

    ui->setupUi(this);

    client = std::make_unique<BotClient>(ui->addressEdit->text().toStdString());

    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRefreshStatus);
    timer->start(2000);
    onRefreshStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::onRefreshStatus() {
    if (!client) return;

    auto st = client->GetStatus();
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

    std::string msg = ui->messageEdit->toPlainText().toStdString();
    std::string err;
    if (!client->SendMessage(channelId, msg, err)) {
        ui->statusLabel->setText(
            QString("Status: send failed (%1)").arg(QString::fromStdString(err)));
    } else {
        ui->statusLabel->setText("Status: message sent");
    }
}