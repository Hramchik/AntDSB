//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_MAINWINDOW_H
#define ANTDSB_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

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
    void onStartBotClicked();
    void onStopBotClicked();
    void onRestartBotClicked();

private:
    std::unique_ptr<MainWindowUi> ui;
    std::unique_ptr<BotClient> client;
    int consecutiveErrors = 0;
};

#endif // ANTDSB_MAINWINDOW_H
