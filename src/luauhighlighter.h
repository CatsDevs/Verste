#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class LuauHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit LuauHighlighter(QTextDocument *parent = nullptr);
protected:
    void highlightBlock(const QString &text) override;
private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QVector<Rule> rules;
};
