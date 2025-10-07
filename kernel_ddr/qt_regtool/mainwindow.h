#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onReadClicked();
    void onWriteClicked();
    void onReadRangeClicked();
    void onWriteRangeClicked();   // ✅ semicolon
    void onOpenTriggered();       // ✅ semicolon

private:
    QLineEdit *addrEdit;
    QLineEdit *valueEdit;
    QLineEdit *countEdit;
    QTextEdit *rangeEdit;
    QPushButton *readButton;
    QPushButton *writeButton;
    QPushButton *readRangeButton;
    QPushButton *writeRangeButton;

    // menubar
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *openAct;
    QAction *exitAct;

    int fd;
};

#endif // MAINWINDOW_H
