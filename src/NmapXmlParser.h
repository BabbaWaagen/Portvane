#pragma once

#include "ScanData.h"

#include <QByteArray>
#include <QList>
#include <QString>

struct ParseResult
{
    QList<Host> hosts;
    // Empty on success. When set, hosts must not be used: a half-read file
    // would look like a scan that found fewer hosts than it did.
    QString error;
};

// Reads nmap's XML output (-oX) into hosts and ports.
ParseResult parseNmapXml(const QByteArray &xml);
