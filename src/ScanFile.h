#pragma once

#include <QByteArray>
#include <QString>

// A saved scan is nmap's own -oX output, byte for byte, so other tools
// (and nmap's own XSL stylesheet) can read it too.

// Returns an error message, or an empty string on success.
QString saveScanFile(const QString &path, const QByteArray &xml);

// Reads the file into xml. Returns an error message, or an empty string on
// success. Does not check that the content is nmap XML; the parser does that.
QString loadScanFile(const QString &path, QByteArray &xml);
