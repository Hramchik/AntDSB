//
// Created by sanek on 13/12/2025.
//

#include "MainWindow.h"
#include "BotClient.h"
#include "../../src/logger/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QListWidget>
#include <QFrame>
#include <QDateTime>
#include <QApplication>

class MainWindowUi {
public:
    QWidget* central = nullptr;

    QLineEdit* addressEdit = nullptr;
    QLabel* statusLabel = nullptr;

    QPushButton* startButton = nullptr;
    QPushButton* stopButton = nullptr;
    QPushButton* restartButton = nullptr;

    QPushButton* reloadChannelsButton = nullptr;

    QListWidget* channelList = nullptr;
    QListWidget* messageList = nullptr;

    QPlainTextEdit* messageEdit = nullptr;
    QPushButton* sendButton = nullptr;

    void setupUi(QMainWindow* w) {
        central = new QWidget(w);
        auto* mainLayout = new QVBoxLayout(central);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(8);

        // ===== Верх: gRPC + статус =====
        auto* addrLayout = new QHBoxLayout;
        addrLayout->setSpacing(6);

        auto* addrLabel = new QLabel("gRPC:", central);
        addrLayout->addWidget(addrLabel);

        addressEdit = new QLineEdit("localhost:50051", central);
        addressEdit->setMinimumWidth(260);
        addrLayout->addWidget(addressEdit, 1);

        statusLabel = new QLabel("Status: stopped", central);
        statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statusLabel->setMinimumWidth(160);
        addrLayout->addWidget(statusLabel);

        mainLayout->addLayout(addrLayout);

        auto* line1 = new QFrame(central);
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line1);

        // ===== Кнопки управления ботом =====
        auto* controlLayout = new QHBoxLayout;
        controlLayout->setSpacing(8);

        startButton = new QPushButton("Start", central);
        stopButton = new QPushButton("Stop", central);
        restartButton = new QPushButton("Restart", central);

        for (auto* btn : { startButton, stopButton, restartButton }) {
            btn->setMinimumHeight(30);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            controlLayout->addWidget(btn);
        }

        mainLayout->addLayout(controlLayout);

        auto* line2 = new QFrame(central);
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line2);

        // ===== Средняя панель: каналы слева, чат справа =====
        auto* midLayout = new QHBoxLayout;
        midLayout->setSpacing(8);

        // Левая колонка: заголовок + список каналов + Reload
        auto* leftLayout = new QVBoxLayout;
        leftLayout->setSpacing(4);

        auto* chanHeader = new QHBoxLayout;
        auto* chanLabel = new QLabel("Channels", central);
        chanHeader->addWidget(chanLabel);
        chanHeader->addStretch(1);

        reloadChannelsButton = new QPushButton("Reload", central);
        reloadChannelsButton->setMinimumHeight(24);
        reloadChannelsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        chanHeader->addWidget(reloadChannelsButton);

        leftLayout->addLayout(chanHeader);

        channelList = new QListWidget(central);
        channelList->setSelectionMode(QAbstractItemView::SingleSelection);
        channelList->setMinimumWidth(260);
        channelList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        channelList->setUniformItemSizes(true);
        leftLayout->addWidget(channelList, 1);

        midLayout->addLayout(leftLayout, 3);

        // Правая колонка: заголовок + список сообщений + поле ввода
        auto* rightLayout = new QVBoxLayout;
        rightLayout->setSpacing(6);

        auto* msgHeader = new QHBoxLayout;
        auto* msgLabel = new QLabel("Chat", central);
        msgHeader->addWidget(msgLabel);
        msgHeader->addStretch(1);
        rightLayout->addLayout(msgHeader);

        messageList = new QListWidget(central);
        messageList->setSelectionMode(QAbstractItemView::NoSelection);
        messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        messageList->setUniformItemSizes(false);
        messageList->setWordWrap(true);
        rightLayout->addWidget(messageList, 5);

        messageEdit = new QPlainTextEdit(central);
        messageEdit->setPlaceholderText("Write a message...");
        messageEdit->setMinimumHeight(80);
        rightLayout->addWidget(messageEdit, 0);

        auto* sendRow = new QHBoxLayout;
        sendRow->addStretch(1);
        sendButton = new QPushButton("Send", central);
        sendButton->setMinimumHeight(30);
        sendButton->setFixedWidth(110);
        sendRow->addWidget(sendButton);
        rightLayout->addLayout(sendRow);

        midLayout->addLayout(rightLayout, 7);

        mainLayout->addLayout(midLayout, 1);

        w->setCentralWidget(central);
        w->setWindowTitle("AntDSB GUI");
        w->resize(980, 620);
    }
};

// ===== MainWindow =====

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowUi>()) {
    ui->setupUi(this);

    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);
    connect(ui->channelList, &QListWidget::currentRowChanged,
            this, &MainWindow::onChannelSelected);
    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::onStartBot);
    connect(ui->stopButton, &QPushButton::clicked,
            this, &MainWindow::onStopBot);
    connect(ui->restartButton, &QPushButton::clicked,
            this, &MainWindow::onRestartBot);
    connect(ui->reloadChannelsButton, &QPushButton::clicked,
            this, &MainWindow::onReloadChannels);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRefreshStatus);
    timer->start(2000);

    auto* historyTimer = new QTimer(this);
    connect(historyTimer, &QTimer::timeout, this, &MainWindow::onHistoryTick);
    historyTimer->start(2000);

    onRefreshStatus();
    onReloadChannels();

    // Dark‑стиль оставляем как у тебя
    QString qss = R"(
        QMainWindow {
            background-color: #18181b;
        }

        QWidget {
            color: #f4f4f5;
            background-color: #18181b;
            font-family: "Segoe UI";
            font-size: 9pt;
        }

        QLabel {
            color: #e4e4e7;
        }

        QLineEdit, QPlainTextEdit {
            background-color: #111827;
            border: 1px solid #27272f;
            border-radius: 4px;
            padding: 4px;
            selection-background-color: #4b5563;
            selection-color: #f9fafb;
        }

        QLineEdit:focus, QPlainTextEdit:focus {
            border: 1px solid #6366f1;
        }

        QListWidget {
            background-color: #020617;
            border: 1px solid #27272f;
            border-radius: 4px;
        }

        QListWidget::item {
            padding: 6px 8px;
        }

        QListWidget::item:selected {
            background-color: #1d4ed8;
        }

        QListWidget::item:hover {
            background-color: #111827;
        }

        QPushButton {
            background-color: #27272f;
            border: 1px solid #3f3f46;
            border-radius: 4px;
            padding: 4px 12px;
        }

        QPushButton:hover {
            background-color: #3f3f46;
        }

        QPushButton:pressed {
            background-color: #52525b;
        }

        QPushButton:disabled {
            background-color: #18181b;
            border-color: #27272f;
            color: #6b7280;
        }

        QFrame[frameShape="4"] {
            color: #27272f;
            background-color: #27272f;
            max-height: 1px;
        }

        QScrollBar:vertical {
            background: #020617;
            width: 10px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background: #4b5563;
            min-height: 24px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #6b7280;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }
    )";
    qApp->setStyleSheet(qss);
}

MainWindow::~MainWindow() = default;

// ===== Внутренние вспомогательные =====

void MainWindow::ensureClient() {
    std::string addr = ui->addressEdit->text().toStdString();
    static std::string lastAddress;

    if (!client || addr != lastAddress) {
        try {
            client = std::make_unique<BotClient>(addr);
            lastAddress = addr;
        } catch (...) {
            client.reset();
        }
    }
}

void MainWindow::updateStatusLabel(const QString& extra) {
    if (!client) {
        ui->statusLabel->setText("Status: no client");
        return;
    }

    antdsb::StatusReply st;
    bool ok = client->TryGetStatus(st);
    QString text = QString("Status: %1").arg(ok && st.running() ? "running" : "stopped");
    if (!st.last_error().empty()) {
        text += QString(" | error: %1").arg(QString::fromStdString(st.last_error()));
    }
    if (!extra.isEmpty()) {
        text += " | " + extra;
    }
    ui->statusLabel->setText(text);
}

// ===== Слоты =====

void MainWindow::onRefreshStatus() {
    ensureClient();
    if (!client) {
        ui->statusLabel->setText("Status: no client");
        return;
    }
    updateStatusLabel();
}

void MainWindow::onStartBot() {
    ensureClient();
    if (!client) {
        ui->statusLabel->setText("Status: no client");
        return;
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->StartBot(st, err)) {
        updateStatusLabel(QString("start failed: %1").arg(QString::fromStdString(err)));
    } else {
        updateStatusLabel("started");
    }
}

void MainWindow::onStopBot() {
    ensureClient();
    if (!client) {
        ui->statusLabel->setText("Status: no client");
        return;
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->StopBot(st, err)) {
        updateStatusLabel(QString("stop failed: %1").arg(QString::fromStdString(err)));
    } else {
        updateStatusLabel("stopped");
    }
}

void MainWindow::onRestartBot() {
    ensureClient();
    if (!client) {
        ui->statusLabel->setText("Status: no client");
        return;
    }

    antdsb::StatusReply st;
    std::string err;
    if (!client->RestartBot(st, err)) {
        updateStatusLabel(QString("restart failed: %1").arg(QString::fromStdString(err)));
    } else {
        updateStatusLabel("restarted");
    }
}

void MainWindow::onReloadChannels() {
    ensureClient();
    if (!client) {
        updateStatusLabel("no client");
        return;
    }

    channelsCache.clear();
    ui->channelList->clear();

    std::string err;
    if (!client->ListChannels(channelsCache, err)) {
        updateStatusLabel(QString("channels error: %1").arg(QString::fromStdString(err)));
        return;
    }

    for (const auto& ch : channelsCache) {
        QString title = QString::fromStdString(ch.name());
        if (title.isEmpty())
            title = QString::number(ch.id());

        auto* item = new QListWidgetItem(title, ui->channelList);
        item->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(ch.id()));
    }

    if (!channelsCache.empty()) {
        ui->channelList->setCurrentRow(0);
    } else {
        currentChannelId = 0;
        ui->messageList->clear();
    }
}

void MainWindow::onChannelSelected(int row) {
    if (row < 0 || row >= ui->channelList->count()) {
        currentChannelId = 0;
        ui->messageList->clear();
        return;
    }

    auto* item = ui->channelList->item(row);
    bool ok = false;
    qulonglong id = item->data(Qt::UserRole).toULongLong(&ok);
    if (!ok) {
        currentChannelId = 0;
        ui->messageList->clear();
        return;
    }

    currentChannelId = static_cast<uint64_t>(id);
    loadMessagesForCurrentChannel();
}

void MainWindow::onHistoryTick() {
    loadMessagesForCurrentChannel();
}

void MainWindow::loadMessagesForCurrentChannel() {
    if (currentChannelId == 0)
        return;

    ensureClient();
    if (!client) {
        ui->messageList->clear();
        auto* item = new QListWidgetItem("No client", ui->messageList);
        item->setForeground(QColor("#9ca3af"));
        return;
    }

    std::vector<antdsb::MessageInfo> messages;
    std::string error;
    if (!client->ListMessages(currentChannelId, 50, messages, error)) {
        ui->messageList->clear();
        auto* item = new QListWidgetItem(
            QString("Error loading messages: %1").arg(QString::fromStdString(error)),
            ui->messageList);
        item->setForeground(QColor("#f97373"));
        return;
    }

    ui->messageList->clear();
    for (const auto& m : messages) {
        QDateTime dt =
            QDateTime::fromSecsSinceEpoch(static_cast<qint64>(m.timestamp()));
        QString header = QString("%1  ·  %2")
            .arg(QString::fromStdString(m.author()))
            .arg(dt.toString("dd.MM.yy hh:mm"));
        QString body = QString::fromStdString(m.content());
        QString text = header + "\n" + body;

        auto* item = new QListWidgetItem(text, ui->messageList);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

    ui->messageList->scrollToBottom();
}

void MainWindow::onSendClicked() {
    if (currentChannelId == 0) {
        updateStatusLabel("no channel selected");
        return;
    }

    ensureClient();
    if (!client) {
        updateStatusLabel("no client");
        return;
    }

    std::string msg = ui->messageEdit->toPlainText().toStdString();
    if (msg.empty()) {
        updateStatusLabel("empty message");
        return;
    }

    std::string err;
    if (!client->SendMessage(currentChannelId, msg, err)) {
        updateStatusLabel(QString("send failed: %1").arg(QString::fromStdString(err)));
    } else {
        updateStatusLabel("message sent");
        ui->messageEdit->clear();
        loadMessagesForCurrentChannel();
    }
}
