//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_MAINWINDOW_H
#define ANTDSB_MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <vector>
#include "BotClient.h"

class MainWindowUi;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRefreshStatus();
    void onSendClicked();
    void onChannelSelected(int row);
    void onStartBot();
    void onStopBot();
    void onRestartBot();
    void onReloadChannels();
    void onHistoryTick();

private:
    std::unique_ptr<MainWindowUi> ui;
    std::unique_ptr<BotClient> client;

    int consecutiveErrors = 0;
    uint64_t currentChannelId = 0;
    std::vector<antdsb::ChannelInfo> channelsCache;

    void ensureClient();
    void loadMessagesForCurrentChannel();
    void updateStatusLabel(const QString& extra = {});
};

#endif // ANTDSB_MAINWINDOW_H
