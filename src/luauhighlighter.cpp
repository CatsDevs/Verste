#include "luauhighlighter.h"
#include <QColor>
#include <QFont>

LuauHighlighter::LuauHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    auto add = [this](const QString &pattern, const QColor &color, bool bold = false) {
        QTextCharFormat fmt;
        fmt.setForeground(color);
        if (bold) fmt.setFontWeight(QFont::Bold);
        rules.push_back({QRegularExpression(pattern), fmt});
    };
    add(R"(\b(local|function|end|if|then|else|elseif|for|while|do|repeat|until|return|break|continue|and|or|not|in|type|export|typeof)\b)", QColor("#c792ea"), true);
    add(R"(\b(true|false|nil)\b)", QColor("#f78c6c"));
    add(R"(\b(game|workspace|script|math|string|table|task|coroutine|Instance|Vector3|Vector2|CFrame|Color3|UDim2|Enum|print|warn|error|pairs|ipairs|select|require)\b)", QColor("#82aaff"));
    add(R"(\b[0-9]+(\.[0-9]+)?\b)", QColor("#f78c6c"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#c3e88d"));
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#697386"));

    // These common single-line forms are highlighted; long-bracket strings/comments
    // are intentionally left to a full Luau parser integration.
    stringFormat = stringFmt;
    commentFormat = commentFmt;
}

void LuauHighlighter::highlightBlock(const QString &text) {
    for (const auto &rule : rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    QRegularExpression strings(R"("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')");
    auto it = strings.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), stringFormat);
    }
    const int comment = text.indexOf("--");
    if (comment >= 0) setFormat(comment, text.size() - comment, commentFormat);
}
