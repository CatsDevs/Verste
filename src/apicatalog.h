#pragma once

#include <QString>
#include <QStringList>

class ApiCatalog final {
public:
    struct Report {
        QStringList detected;
        QString text;
    };

    static QStringList names();
    static Report audit(const QString &source);
};
