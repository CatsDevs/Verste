#include "apicatalog.h"

#include <QRegularExpression>
#include <QSet>

QStringList ApiCatalog::names() {
    return {
        // Environment and execution names
        "identifyexecutor", "getexecutorname", "getexecutorversion", "getgenv", "getrenv",
        "getreg", "getgc", "getfenv", "setfenv", "getcallingscript", "checkcaller",
        "loadstring", "getinstances", "getnilinstances", "getloadedmodules",
        // Filesystem
        "readfile", "writefile", "appendfile", "listfiles", "isfile", "isfolder",
        "makefolder", "delfile", "delfolder", "loadfile", "dofile",
        // Closures and debugging
        "newcclosure", "iscclosure", "islclosure", "clonefunction", "hookfunction",
        "restorefunction", "replaceclosure", "cloneclosure", "isexecutorclosure",
        "getconstants", "getconstant", "setconstant", "getupvalues", "getupvalue",
        "setupvalue", "getstack", "setstack", "getinfo",
        // Metatables and Instances
        "getrawmetatable", "setrawmetatable", "setreadonly", "isreadonly",
        "getnamecallmethod", "setnamecallmethod", "gethui", "getcustomasset",
        "cloneref", "compareinstances", "isscriptable", "gethiddenproperty",
        "sethiddenproperty", "getproperties",
        // Drawing and signals
        "Drawing.new", "Drawing.clear", "getconnections", "firesignal",
        "fireclickdetector", "fireproximityprompt", "firetouchinterest",
        // HTTP, sockets, and crypto
        "request", "http_request", "WebSocket.connect", "base64encode", "base64decode",
        "encrypt", "decrypt", "generatekey", "generatebytes", "hash",
        // Input and console
        "setclipboard", "toclipboard", "keypress", "keyrelease", "mouse1click",
        "mouse1press", "mouse1release", "mouse2click", "mouse2press", "mouse2release",
        "mousemoveabs", "mousemoverel", "mousescroll", "rconsoleprint", "rconsoleclear",
        "rconsolecreate", "rconsolesettitle"
    };
}

ApiCatalog::Report ApiCatalog::audit(const QString &source) {
    Report report;
    const QStringList catalog = names();
    QSet<QString> found;
    for (const QString &api : catalog) {
        QString pattern = QRegularExpression::escape(api);
        pattern.replace("\\.", "\\\\s*\\\\.\\\\s*");
        const QRegularExpression expression(
            QString("(?<![A-Za-z0-9_])%1(?![A-Za-z0-9_])").arg(pattern));
        if (expression.match(source).hasMatch()) found.insert(api);
    }
    report.detected = found.values();
    report.detected.sort(Qt::CaseInsensitive);

    QStringList lines;
    lines << "UNC/sUNC STATIC NAME AUDIT";
    lines << "=========================";
    lines << QString("Catalog entries: %1").arg(catalog.size());
    lines << QString("Names detected in source: %1").arg(report.detected.size());
    lines << "";
    lines << "Detected names (reference only):";
    if (report.detected.isEmpty()) lines << "  None";
    for (const QString &api : report.detected)
        lines << QString("  [FOUND IN TEXT] %1 — not provided by Verste").arg(api);
    lines << "";
    lines << "Not detected:";
    for (const QString &api : catalog) {
        if (!found.contains(api))
            lines << QString("  [NOT FOUND] %1").arg(api);
    }
    lines << "";
    lines << "This audit scans source text only. It does not test runtime behavior,";
    lines << "implement an executor, or imply UNC/sUNC compatibility.";
    report.text = lines.join('\n');
    return report;
}
