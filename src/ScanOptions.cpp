#include "ScanOptions.h"

static QString scanTypeFlag(ScanType scanType)
{
    switch (scanType) {
    case ScanType::Connect:
        return QStringLiteral("-sT");
    case ScanType::Syn:
        return QStringLiteral("-sS");
    case ScanType::Udp:
        return QStringLiteral("-sU");
    case ScanType::PingOnly:
        return QStringLiteral("-sn");
    }
    return QString();
}

QStringList buildNmapArguments(const ScanOptions &options)
{
    QStringList arguments;
    arguments << scanTypeFlag(options.scanType);

    // A ping scan never looks at ports, so port, version and OS options would
    // only be noise in the command.
    if (options.scanType != ScanType::PingOnly) {
        // nmap rejects "22, 80", so spaces typed after commas are dropped.
        QString ports = options.ports;
        ports.remove(QLatin1Char(' '));
        if (!ports.isEmpty())
            arguments << QStringLiteral("-p") << ports;
        if (options.serviceVersion)
            arguments << QStringLiteral("-sV");
        if (options.osDetection)
            arguments << QStringLiteral("-O");
    }

    arguments << QStringLiteral("-T%1").arg(options.timing);
    if (options.skipPing)
        arguments << QStringLiteral("-Pn");

    arguments << options.targets;
    return arguments;
}
