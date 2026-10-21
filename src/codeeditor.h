#pragma once

#include <QPlainTextEdit>
#include <QCompleter>
#include <QWidget>

class LineNumberArea;

class CodeEditor final : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const { return m_completer; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void insertCompletion(const QString &completion);

private:
    QWidget *m_lineNumberArea{};
    QCompleter *m_completer{};
};
