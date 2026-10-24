#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

class ScriptHistory final {
public:
    struct Entry {
        QString id;
        QString label;
        QString reason;
        QString source;
        QDateTime timestamp;
    };

    static void save(const QString &source, const QString &label, const QString &reason);
    static QVector<Entry> entries();
};
