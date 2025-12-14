//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_MAINWINDOW_H
#define ANTDSB_MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "BotClient.h"

class MainWindowUi;   // вперёд-объявление

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRefreshStatus();
    void onSendClicked();

private:
    std::unique_ptr<MainWindowUi> ui;
    std::unique_ptr<BotClient> client;
};

#endif //ANTDSB_MAINWINDOW_H