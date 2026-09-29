#pragma once

#include "ScanData.h"

#include <QList>
#include <QString>

enum class ChangeKind {
    HostAdded,
    HostRemoved,
    HostStateChanged,
    PortAdded,   // listed now, not listed before
    PortRemoved, // listed before, not listed now
    PortStateChanged,
    ServiceChanged,
};

// One difference between two scans. For host changes, protocol is empty and
// portid is 0. For added and removed items, the missing side is empty.
struct Change
{
    ChangeKind kind = ChangeKind::HostAdded;
    QString address;
    QString protocol;
    int portid = 0;
    QString before;
    QString after;
};

// Lists what changed from one scan to the next. Hosts are matched by address,
// ports by protocol and port number.
//
// A port missing from a scan is "not listed", not "closed": nmap often folds
// closed ports into a single summary line that names no port numbers.
QList<Change> diffScans(const QList<Host> &before, const QList<Host> &after);
