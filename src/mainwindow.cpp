#include "mainwindow.h"
#include "luauhighlighter.h"

#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDateTime>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QRegularExpression>
#include <QMenuBar>
#include <QCloseEvent>

static const char *DARK_STYLE = R"(
QMainWindow, QWidget { background: #111318; color: #d7dae0; font-family: "Segoe UI"; font-size: 10pt; }
QToolBar, QMenuBar { background: #171a21; border: 0; spacing: 6px; padding: 6px; }
QMenuBar::item:selected, QMenu::item:selected { background: #303747; }
QMenu { background: #1a1e27; border: 1px solid #343b49; }
QToolButton, QPushButton { background: #252a35; border: 1px solid #343b49; border-radius: 6px; padding: 6px 11px; color: #e5e9f0; }
QToolButton:hover, QPushButton:hover { background: #303747; }
QTabWidget::pane { border: 1px solid #2a303b; background: #151820; }
QTabBar::tab { background: #1a1e27; border: 1px solid #2a303b; padding: 8px 14px; margin-right: 2px; }
QTabBar::tab:selected { background: #252b38; color: #ffffff; }
QLineEdit, QPlainTextEdit { background: #0e1015; color: #d7dae0; border: 1px solid #252b35; selection-background-color: #34445f; font-family: Consolas, "Cascadia Code"; }
QLabel#Status { color: #8b93a5; padding: 4px 8px; }
)");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Verste");
    resize(1120, 760);
    setStyleSheet(DARK_STYLE);

    auto *fileMenu = menuBar()->addMenu("&File");
    auto *editMenu = menuBar()->addMenu("&Edit");
    auto *viewMenu = menuBar()->addMenu("&View");
    auto *runMenu = menuBar()->addMenu("&Test");

    auto addAction = [this](QMenu *menu, const QString &label, const QKeySequence &shortcut, auto callback) {
        QAction *action = menu->addAction(label);
        action->setShortcut(shortcut);
        connect(action, &QAction::triggered, this, callback);
        return action;
    };
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    auto addTool = [toolbar](const QString &label, const QKeySequence &shortcut, auto callback) {
        QAction *action = toolbar->addAction(label);
        action->setShortcut(shortcut);
        QObject::connect(action, &QAction::triggered, qApp, callback);
    };

    addAction(fileMenu, "New Tab", QKeySequence("Ctrl+N"), [this] { newTab(); });
    addAction(fileMenu, "Open...", QKeySequence("Ctrl+O"), [this] { openFile(); });
    addAction(fileMenu, "Save", QKeySequence("Ctrl+S"), [this] { saveFile(); });
    addAction(fileMenu, "Save As...", QKeySequence("Ctrl+Shift+S"), [this] { saveFileAs(); });
    fileMenu->addSeparator();
    addAction(fileMenu, "Close Tab", QKeySequence("Ctrl+W"), [this] { closeTab(tabs->currentIndex()); });
    addAction(editMenu, "Find...", QKeySequence("Ctrl+F"), [this] { showFindBar(); });
    addAction(editMenu, "Find Next", QKeySequence("F3"), [this] { findNext(); });
    addAction(editMenu, "Replace Current", QKeySequence("Ctrl+H"), [this] { showFindBar(); replaceInput->setFocus(); });
    addAction(viewMenu, "Clear Console", QKeySequence("Ctrl+L"), [this] { clearConsole(); });
    addAction(runMenu, "Run Static Checks", QKeySequence("F5"), [this] { runChecks(); });

    toolbar->addAction("New", this, &MainWindow::newTab);
    toolbar->addAction("Open", this, &MainWindow::openFile);
    toolbar->addAction("Save", this, &MainWindow::saveFile);
    toolbar->addSeparator();
    toolbar->addAction("Find", this, &MainWindow::showFindBar);
    toolbar->addAction("Run Checks", this, &MainWindow::runChecks);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(7);

    findBar = new QWidget(central);
    auto *findLayout = new QHBoxLayout(findBar);
    findLayout->setContentsMargins(0, 0, 0, 0);
    findInput = new QLineEdit(findBar);
    findInput->setPlaceholderText("Find text...");
    replaceInput = new QLineEdit(findBar);
    replaceInput->setPlaceholderText("Replace with...");
    auto *nextButton = new QPushButton("Next", findBar);
    auto *replaceButton = new QPushButton("Replace", findBar);
    auto *closeFind = new QPushButton("×", findBar);
    closeFind->setMaximumWidth(36);
    findLayout->addWidget(findInput, 2);
    findLayout->addWidget(replaceInput, 2);
    findLayout->addWidget(nextButton);
    findLayout->addWidget(replaceButton);
    findLayout->addWidget(closeFind);
    findBar->hide();
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::findNext);
    connect(replaceButton, &QPushButton::clicked, this, &MainWindow::replaceCurrent);
    connect(closeFind, &QPushButton::clicked, findBar, &QWidget::hide);
    connect(findInput, &QLineEdit::returnPressed, this, &MainWindow::findNext);

    tabs = new QTabWidget(central);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    console = new QPlainTextEdit(central);
    console->setReadOnly(true);
    console->setPlaceholderText("Static-check output...");
    console->setMaximumBlockCount(1000);
    console->setMinimumHeight(130);

    auto *splitter = new QSplitter(Qt::Vertical, central);
    splitter->addWidget(tabs);
    splitter->addWidget(console);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 1);
    layout->addWidget(findBar);
    layout->addWidget(splitter);

    status = new QLabel("Ready  |  Static checks only  |  No Roblox attachment or injection", central);
    status->setObjectName("Status");
    layout->addWidget(status);
    setCentralWidget(central);

    applySettings();
    newTab();
    appendConsole("Verste started. F5 runs static checks only; scripts are not executed.");
}

void MainWindow::applySettings() {
    QSettings settings;
    resize(settings.value("window/size", QSize(1120, 760)).toSize());
}

CodeEditor *MainWindow::currentEditor() const {
    return qobject_cast<CodeEditor *>(tabs->currentWidget());
}
QString MainWindow::currentFilePath() const {
    return tabs->currentWidget() ? tabs->currentWidget()->property("filePath").toString() : QString();
}
void MainWindow::setCurrentFilePath(const QString &path) {
    if (!tabs->currentWidget()) return;
    tabs->currentWidget()->setProperty("filePath", path);
    tabs->setTabText(tabs->currentIndex(), path.isEmpty() ? "Untitled*" : QFileInfo(path).fileName());
}
void MainWindow::newTab() {
    auto *editor = new CodeEditor;
    editor->setPlaceholderText("-- Write Luau here\nlocal message = \"Hello, Verste!\"\nprint(message)");
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    new LuauHighlighter(editor->document());
    const int index = tabs->addTab(editor, "Untitled*");
    tabs->setCurrentIndex(index);
    editor->setProperty("filePath", QString());
    connect(editor, &QPlainTextEdit::textChanged, this, [this, editor] {
        const int i = tabs->indexOf(editor);
        if (i >= 0) {
            QString name = tabs->tabText(i);
            if (!name.endsWith('*')) tabs->setTabText(i, name + '*');
        }
    });
    status->setText("New script");
}
bool MainWindow::maybeSave(int index) {
    if (index < 0) return true;
    auto *editor = qobject_cast<CodeEditor *>(tabs->widget(index));
    if (!editor) return true;
    const QString path = tabs->widget(index)->property("filePath").toString();
    if (!tabs->tabText(index).endsWith('*')) return true;
    const auto answer = QMessageBox::question(this, "Unsaved changes",
        "Save changes to " + (path.isEmpty() ? QString("Untitled") : QFileInfo(path).fileName()) + "?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel) return false;
    if (answer == QMessageBox::Save) {
        tabs->setCurrentIndex(index);
        saveFile();
        return !tabs->tabText(index).endsWith('*');
    }
    return true;
}
void MainWindow::closeTab(int index) {
    if (tabs->count() <= 1) { newTab(); index = 0; }
    if (!maybeSave(index)) return;
    QWidget *page = tabs->widget(index);
    tabs->removeTab(index);
    page->deleteLater();
}
void MainWindow::openFile() {
    const QString path = QFileDialog::getOpenFileName(this, "Open Luau file", {}, "Luau files (*.luau *.lua *.txt);;All files (*.*)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open failed", file.errorString());
        return;
    }
    newTab();
    currentEditor()->setPlainText(QString::fromUtf8(file.readAll()));
    setCurrentFilePath(path);
    const QString name = QFileInfo(path).fileName();
    tabs->setTabText(tabs->currentIndex(), name);
    status->setText("Opened " + name);
}
void MainWindow::saveFile() {
    if (currentFilePath().isEmpty()) { saveFileAs(); return; }
    const QString path = currentFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save failed", file.errorString());
        return;
    }
    file.write(currentEditor()->toPlainText().toUtf8());
    tabs->setTabText(tabs->currentIndex(), QFileInfo(path).fileName());
    appendConsole("Saved: " + path);
    status->setText("Saved " + QFileInfo(path).fileName());
}
void MainWindow::saveFileAs() {
    QString path = QFileDialog::getSaveFileName(this, "Save Luau file", "script.luau", "Luau files (*.luau);;Lua files (*.lua);;Text files (*.txt)");
    if (path.isEmpty()) return;
    setCurrentFilePath(path);
    saveFile();
}
void MainWindow::appendConsole(const QString &text) {
    console->appendPlainText("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "] " + text);
}
void MainWindow::clearConsole() { console->clear(); }
void MainWindow::showFindBar() {
    findBar->show();
    findInput->setFocus();
    findInput->selectAll();
}
void MainWindow::findNext() {
    if (!currentEditor() || findInput->text().isEmpty()) return;
    if (!currentEditor()->find(findInput->text())) {
        QTextCursor cursor = currentEditor()->textCursor();
        cursor.movePosition(QTextCursor::Start);
        currentEditor()->setTextCursor(cursor);
        if (!currentEditor()->find(findInput->text()))
            status->setText("Text not found");
    }
}
void MainWindow::replaceCurrent() {
    if (!currentEditor() || findInput->text().isEmpty()) return;
    QTextCursor cursor = currentEditor()->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == findInput->text()) {
        cursor.insertText(replaceInput->text());
        currentEditor()->setTextCursor(cursor);
    }
    findNext();
}
void MainWindow::runChecks() {
    auto *editor = currentEditor();
    if (!editor) return;
    const QString code = editor->toPlainText();
    appendConsole("--- Static checks started ---");
    int issues = 0;
    if (code.trimmed().isEmpty()) {
        appendConsole("ERROR: Editor is empty.");
        status->setText("Static checks: 1 issue");
        return;
    }

    // Lightweight delimiter/string scan, not a complete Luau parser.
    int paren = 0, brace = 0, bracket = 0;
    bool single = false, dbl = false, escaped = false, comment = false;
    for (int i = 0; i < code.size(); ++i) {
        const QChar c = code[i];
        const QChar next = i + 1 < code.size() ? code[i + 1] : QChar();
        if (comment) { if (c == '\n') comment = false; continue; }
        if (!single && !dbl && c == '-' && next == '-') { comment = true; ++i; continue; }
        if (single || dbl) {
            if (escaped) { escaped = false; continue; }
            if (c == '\\') { escaped = true; continue; }
            if (single && c == '\'') single = false;
            else if (dbl && c == '"') dbl = false;
            continue;
        }
        if (c == '\'') { single = true; continue; }
        if (c == '"') { dbl = true; continue; }
        if (c == '(') ++paren; else if (c == ')') --paren;
        else if (c == '{') ++brace; else if (c == '}') --brace;
        else if (c == '[') ++bracket; else if (c == ']') --bracket;
        if (paren < 0 || brace < 0 || bracket < 0) break;
    }
    if (single || dbl) { appendConsole("WARNING: Possible unterminated string."); ++issues; }
    if (paren != 0 || brace != 0 || bracket != 0) { appendConsole("WARNING: Delimiters may be unbalanced."); ++issues; }
    if (code.contains("TODO", Qt::CaseInsensitive)) appendConsole("INFO: TODO marker found.");
    if (code.contains("loadstring", Qt::CaseInsensitive)) {
        appendConsole("WARNING: loadstring found. This editor does not execute code; review untrusted scripts carefully.");
        ++issues;
    }
    appendConsole(QString("Static checks finished. Potential issue count: %1. Not a full Luau parser.").arg(issues));
    status->setText(QString("Checks complete: %1 potential issue(s)").arg(issues));
}
