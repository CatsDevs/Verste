#include "scripthistory.h"

#include <QSettings>
#include <QUuid>

void ScriptHistory::save(const QString &source, const QString &label, const QString &reason) {
    QSettings settings;
    QVector<Entry> current = entries();
    Entry entry;
    entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    entry.label = label.isEmpty() ? QString("Untitled") : label;
    entry.reason = reason;
    entry.source = source;
    entry.timestamp = QDateTime::currentDateTime();

    settings.remove("scriptHistory");
    settings.beginWriteArray("scriptHistory", qMin(99, static_cast<int>(current.size())) + 1);
    settings.setArrayIndex(0);
    settings.setValue("id", entry.id);
    settings.setValue("label", entry.label);
    settings.setValue("reason", entry.reason);
    settings.setValue("source", entry.source);
    settings.setValue("timestamp", entry.timestamp);
    const int keep = qMin(99, static_cast<int>(current.size()));
    for (int i = 0; i < keep; ++i) {
        settings.setArrayIndex(i + 1);
        settings.setValue("id", current[i].id);
        settings.setValue("label", current[i].label);
        settings.setValue("reason", current[i].reason);
        settings.setValue("source", current[i].source);
        settings.setValue("timestamp", current[i].timestamp);
    }
    settings.endArray();
    settings.sync();
}

QVector<ScriptHistory::Entry> ScriptHistory::entries() {
    QSettings settings;
    QVector<Entry> result;
    const int count = settings.beginReadArray("scriptHistory");
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        Entry entry;
        entry.id = settings.value("id").toString();
        entry.label = settings.value("label", "Untitled").toString();
        entry.reason = settings.value("reason").toString();
        entry.source = settings.value("source").toString();
        entry.timestamp = settings.value("timestamp").toDateTime();
        result.push_back(entry);
    }
    settings.endArray();
    return result;
}
