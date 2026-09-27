#include "ScanFile.h"

#include <QFile>
#include <QSaveFile>

QString saveScanFile(const QString &path, const QByteArray &xml)
{
    // QSaveFile writes to a temp file and only replaces the target on
    // commit(), so a failed save never leaves a half-written scan behind.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return file.errorString();
    if (file.write(xml) != xml.size())
        return file.errorString();
    if (!file.commit())
        return file.errorString();
    return QString();
}

QString loadScanFile(const QString &path, QByteArray &xml)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return file.errorString();
    xml = file.readAll();
    return QString();
}
