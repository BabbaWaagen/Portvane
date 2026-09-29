#include "ScanDiff.h"

static const Host *findHost(const QList<Host> &hosts, const QString &address)
{
    for (const Host &host : hosts) {
        if (host.address == address)
            return &host;
    }
    return nullptr;
}

static const Port *findPort(const QList<Port> &ports, const Port &wanted)
{
    for (const Port &port : ports) {
        if (port.portid == wanted.portid && port.protocol == wanted.protocol)
            return &port;
    }
    return nullptr;
}

static QString serviceText(const Port &port)
{
    return (port.service + QLatin1Char(' ') + port.version).trimmed();
}

static Change portChange(ChangeKind kind, const QString &address, const Port &port,
                         const QString &before, const QString &after)
{
    return Change{kind, address, port.protocol, port.portid, before, after};
}

// A host that is only in one scan is compared against an empty port list, so
// all of its ports show up as added or removed too.
static void diffPorts(const QString &address, const QList<Port> &before,
                      const QList<Port> &after, QList<Change> &changes)
{
    for (const Port &old : before) {
        const Port *now = findPort(after, old);
        if (!now) {
            changes << portChange(ChangeKind::PortRemoved, address, old, old.state, QString());
            continue;
        }
        if (old.state != now->state)
            changes << portChange(ChangeKind::PortStateChanged, address, old, old.state, now->state);
        if (serviceText(old) != serviceText(*now))
            changes << portChange(ChangeKind::ServiceChanged, address, old, serviceText(old),
                                  serviceText(*now));
    }

    for (const Port &now : after) {
        if (!findPort(before, now))
            changes << portChange(ChangeKind::PortAdded, address, now, QString(), now.state);
    }
}

QList<Change> diffScans(const QList<Host> &before, const QList<Host> &after)
{
    QList<Change> changes;

    for (const Host &old : before) {
        const Host *now = findHost(after, old.address);
        if (!now) {
            changes << Change{ChangeKind::HostRemoved, old.address, QString(), 0, old.state, QString()};
            diffPorts(old.address, old.ports, {}, changes);
            continue;
        }
        if (old.state != now->state)
            changes << Change{ChangeKind::HostStateChanged, old.address, QString(), 0, old.state,
                              now->state};
        diffPorts(old.address, old.ports, now->ports, changes);
    }

    for (const Host &now : after) {
        if (findHost(before, now.address))
            continue;
        changes << Change{ChangeKind::HostAdded, now.address, QString(), 0, QString(), now.state};
        diffPorts(now.address, {}, now.ports, changes);
    }

    return changes;
}
