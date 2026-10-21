#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QLabel>
#include "codeeditor.h"

class QLineEdit;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void newTab();
    void openFile();
    void saveFile();
    void saveFileAs();
    void closeTab(int index);
    void runChecks();
    void clearConsole();
    void showFindBar();
    void findNext();
    void replaceCurrent();
private:
    QTabWidget *tabs{};
    QPlainTextEdit *console{};
    QLabel *status{};
    QWidget *findBar{};
    QLineEdit *findInput{};
    QLineEdit *replaceInput{};
    CodeEditor *currentEditor() const;
    QString currentFilePath() const;
    void setCurrentFilePath(const QString &path);
    void appendConsole(const QString &text);
    bool maybeSave(int index);
    void applySettings();
};
