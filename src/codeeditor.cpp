#include "codeeditor.h"

#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextBlock>

class LineNumberArea final : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return QSize(m_editor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { m_editor->lineNumberAreaPaintEvent(event); }
private:
    CodeEditor *m_editor;
};

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect &rect, int dy) {
        if (dy) m_lineNumberArea->scroll(0, dy);
        else m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
        if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
    });
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    auto *model = new QStringListModel({
        "local", "function", "end", "if", "then", "else", "elseif", "for",
        "while", "do", "repeat", "until", "return", "break", "continue",
        "and", "or", "not", "in", "type", "export", "typeof", "true", "false",
        "nil", "print", "warn", "error", "pairs", "ipairs", "require",
        "game", "workspace", "script", "task", "math", "string", "table",
        "Vector3", "Vector2", "CFrame", "Color3", "UDim2", "Enum", "Instance"
    }, this);
    auto *completion = new QCompleter(model, this);
    completion->setCaseSensitivity(Qt::CaseInsensitive);
    completion->setWidget(this);
    completion->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completion);
}

int CodeEditor::lineNumberAreaWidth() const {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    return 14 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor("#1b2230"));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#151820"));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(blockNumber == textCursor().blockNumber()
                           ? QColor("#82aaff") : QColor("#667085"));
            painter.drawText(0, top, m_lineNumberArea->width() - 6,
                             fontMetrics().height(), Qt::AlignRight,
                             QString::number(blockNumber + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::setCompleter(QCompleter *completer) {
    if (m_completer) disconnect(m_completer, nullptr, this, nullptr);
    m_completer = completer;
    if (!m_completer) return;
    m_completer->setWidget(this);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

void CodeEditor::insertCompletion(const QString &completion) {
    if (!m_completer || m_completer->widget() != this) return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    if (m_completer && m_completer->popup()->isVisible()) {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Escape || event->key() == Qt::Key_Tab ||
            event->key() == Qt::Key_Backtab) {
            event->ignore();
            return;
        }
    }
    if (event->key() == Qt::Key_Space && (event->modifiers() & Qt::ControlModifier)) {
        const QString prefix = textCursor().block().text().left(textCursor().positionInBlock());
        QRegularExpression re("[A-Za-z_][A-Za-z0-9_]*$");
        const auto match = re.match(prefix);
        m_completer->setCompletionPrefix(match.hasMatch() ? match.captured() : QString());
        QRect cr = cursorRect();
        cr.setWidth(m_completer->popup()->sizeHintForColumn(0) +
                    m_completer->popup()->verticalScrollBar()->sizeHint().width());
        m_completer->complete(cr);
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
    if (m_completer && !event->text().isEmpty() && event->text().back().isLetterOrNumber()) {
        const QString prefix = textCursor().block().text().left(textCursor().positionInBlock());
        QRegularExpression re("[A-Za-z_][A-Za-z0-9_]*$");
        const auto match = re.match(prefix);
        if (match.hasMatch() && match.captured().size() >= 2) {
            m_completer->setCompletionPrefix(match.captured());
            m_completer->complete(cursorRect());
        } else {
            m_completer->popup()->hide();
        }
    } else if (m_completer) {
        m_completer->popup()->hide();
    }
}
