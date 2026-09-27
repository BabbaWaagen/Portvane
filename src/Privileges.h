#pragma once

#include <QString>

// What the app is allowed to do, as far as nmap's raw-packet options
// (-sS, -sU, -O) are concerned.
struct Privileges
{
    // Running as Administrator (Windows) or root (Linux).
    bool isPrivileged = false;
    // A packet capture driver nmap can use. On Windows that is Npcap; on Linux
    // it is libpcap, which every nmap package already depends on.
    bool hasPacketCapture = false;
};

// Asks the OS. The only place in the app that knows how that works per OS.
Privileges detectPrivileges();

// Whether -sS, -sU and -O can be offered.
bool privilegedOptionsAllowed(const Privileges &privileges);

// Why those options are unavailable, for their tooltips. Empty if they are
// allowed.
QString missingPrivilegesText(const Privileges &privileges);
