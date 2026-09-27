#include "NmapLocator.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringList>

static bool isExecutableFile(const QString &path)
{
    const QFileInfo info(path);
    return info.isFile() && info.isExecutable();
}

// Places where the official installers put nmap. Needed because a GUI app
// started from the desktop may not see the same PATH as a terminal.
static QStringList fallbackNmapPaths()
{
#ifdef Q_OS_WIN
    QStringList paths;
    const QStringList programDirs = {
        qEnvironmentVariable("ProgramFiles(x86)"),
        qEnvironmentVariable("ProgramFiles"),
    };
    for (const QString &dir : programDirs) {
        if (!dir.isEmpty())
            paths << QDir(dir).filePath(QStringLiteral("Nmap/nmap.exe"));
    }
    return paths;
#elif defined(Q_OS_LINUX)
    return {
        QStringLiteral("/usr/bin/nmap"),
        QStringLiteral("/usr/local/bin/nmap"),
        QStringLiteral("/snap/bin/nmap"),
    };
#else
    return {};
#endif
}

std::optional<QString> findNmap(const QString &overridePath)
{
    if (!overridePath.isEmpty()) {
        if (isExecutableFile(overridePath))
            return overridePath;
        return std::nullopt;
    }

    const QString fromPath = QStandardPaths::findExecutable(QStringLiteral("nmap"));
    if (!fromPath.isEmpty())
        return fromPath;

    for (const QString &path : fallbackNmapPaths()) {
        if (isExecutableFile(path))
            return path;
    }

    return std::nullopt;
}
