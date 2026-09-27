#pragma once

#include <QString>
#include <QStringList>

enum class ScanType {
    Connect,  // -sT
    Syn,      // -sS
    Udp,      // -sU
    PingOnly, // -sn
};

// What the user picked in the option builder.
struct ScanOptions
{
    ScanType scanType = ScanType::Connect;
    // As typed, e.g. "22,80,443" or "1-1024". Empty means nmap's default ports.
    QString ports;
    // 0 to 5, for -T0 to -T5. 3 is nmap's own default.
    int timing = 3;
    bool serviceVersion = false;
    bool osDetection = false;
    bool skipPing = false;
    QStringList targets;
};

// Turns the options into nmap arguments. Never adds output options (-oX,
// -oN, ...): the runner owns those.
QStringList buildNmapArguments(const ScanOptions &options);
