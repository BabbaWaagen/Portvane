#include "NmapXmlParser.h"
#include "ScanDiff.h"

#include <QDir>
#include <QFile>
#include <QTest>

static Port port(int portid, const QString &state, const QString &protocol = QStringLiteral("tcp"))
{
    Port result;
    result.portid = portid;
    result.protocol = protocol;
    result.state = state;
    return result;
}

static Host host(const QString &address, const QString &state, const QList<Port> &ports = {})
{
    return Host{address, QString(), state, ports};
}

static QString kindName(ChangeKind kind)
{
    switch (kind) {
    case ChangeKind::HostAdded:
        return QStringLiteral("HostAdded");
    case ChangeKind::HostRemoved:
        return QStringLiteral("HostRemoved");
    case ChangeKind::HostStateChanged:
        return QStringLiteral("HostStateChanged");
    case ChangeKind::PortAdded:
        return QStringLiteral("PortAdded");
    case ChangeKind::PortRemoved:
        return QStringLiteral("PortRemoved");
    case ChangeKind::PortStateChanged:
        return QStringLiteral("PortStateChanged");
    case ChangeKind::ServiceChanged:
        return QStringLiteral("ServiceChanged");
    }
    return QString();
}

// One line per change, so a failing QCOMPARE shows exactly what differs.
static QStringList describe(const QList<Change> &changes)
{
    QStringList lines;
    for (const Change &change : changes) {
        QString where = change.address;
        if (!change.protocol.isEmpty())
            where += QStringLiteral(" %1/%2").arg(change.portid).arg(change.protocol);
        lines << QStringLiteral("%1 %2: '%3' -> '%4'")
                     .arg(kindName(change.kind), where, change.before, change.after);
    }
    return lines;
}

static QList<Host> hostsFromFixture(const QString &name)
{
    QFile file(QDir(QString::fromUtf8(FIXTURES_DIR)).filePath(name));
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return parseNmapXml(file.readAll()).hosts;
}

class TestScanDiff : public QObject
{
    Q_OBJECT

private slots:
    void identicalScansHaveNoChanges();
    void findsAddedAndRemovedHosts();
    void findsHostStateChange();
    void findsPortStateChange();
    void portListedInOnlyOneScan();
    void findsServiceChange();
    void protocolIsPartOfThePort();
    void comparesRealScans();
};

void TestScanDiff::identicalScansHaveNoChanges()
{
    const QList<Host> hosts = hostsFromFixture(QStringLiteral("two_hosts.xml"));
    QCOMPARE(hosts.size(), 2);

    QVERIFY(diffScans(hosts, hosts).isEmpty());
}

void TestScanDiff::findsAddedAndRemovedHosts()
{
    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                     {port(22, QStringLiteral("open"))})};
    const QList<Host> after = {host(QStringLiteral("10.0.0.2"), QStringLiteral("up"),
                                    {port(80, QStringLiteral("open"))})};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("HostRemoved 10.0.0.1: 'up' -> ''"),
                          QStringLiteral("PortRemoved 10.0.0.1 22/tcp: 'open' -> ''"),
                          QStringLiteral("HostAdded 10.0.0.2: '' -> 'up'"),
                          QStringLiteral("PortAdded 10.0.0.2 80/tcp: '' -> 'open'")}));
}

void TestScanDiff::findsHostStateChange()
{
    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"))};
    const QList<Host> after = {host(QStringLiteral("10.0.0.1"), QStringLiteral("down"))};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("HostStateChanged 10.0.0.1: 'up' -> 'down'")}));
}

void TestScanDiff::findsPortStateChange()
{
    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                     {port(22, QStringLiteral("closed"))})};
    const QList<Host> after = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                    {port(22, QStringLiteral("open"))})};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("PortStateChanged 10.0.0.1 22/tcp: 'closed' -> 'open'")}));
}

// nmap folds closed ports into <extraports>, so a port can simply vanish from
// the list. That is reported as removed from the list, not as closed.
void TestScanDiff::portListedInOnlyOneScan()
{
    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                     {port(22, QStringLiteral("open")),
                                      port(80, QStringLiteral("open"))})};
    const QList<Host> after = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                    {port(22, QStringLiteral("open"))})};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("PortRemoved 10.0.0.1 80/tcp: 'open' -> ''")}));
}

void TestScanDiff::findsServiceChange()
{
    Port oldSsh = port(22, QStringLiteral("open"));
    oldSsh.service = QStringLiteral("ssh");
    oldSsh.version = QStringLiteral("OpenSSH 8.9");
    Port newSsh = oldSsh;
    newSsh.version = QStringLiteral("OpenSSH 9.6");

    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"), {oldSsh})};
    const QList<Host> after = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"), {newSsh})};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral(
                 "ServiceChanged 10.0.0.1 22/tcp: 'ssh OpenSSH 8.9' -> 'ssh OpenSSH 9.6'")}));
}

void TestScanDiff::protocolIsPartOfThePort()
{
    const QList<Host> before = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                     {port(53, QStringLiteral("open"), QStringLiteral("tcp"))})};
    const QList<Host> after = {host(QStringLiteral("10.0.0.1"), QStringLiteral("up"),
                                    {port(53, QStringLiteral("open"), QStringLiteral("udp"))})};

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("PortRemoved 10.0.0.1 53/tcp: 'open' -> ''"),
                          QStringLiteral("PortAdded 10.0.0.1 53/udp: '' -> 'open'")}));
}

// two_hosts.xml scanned ports 1, 135 and 445 on two hosts; service_version.xml
// only port 135 on localhost, with -sV.
void TestScanDiff::comparesRealScans()
{
    const QList<Host> before = hostsFromFixture(QStringLiteral("two_hosts.xml"));
    const QList<Host> after = hostsFromFixture(QStringLiteral("service_version.xml"));
    QCOMPARE(before.size(), 2);
    QCOMPARE(after.size(), 1);

    QCOMPARE(describe(diffScans(before, after)),
             QStringList({QStringLiteral("PortRemoved 127.0.0.1 1/tcp: 'closed' -> ''"),
                          QStringLiteral("ServiceChanged 127.0.0.1 135/tcp: 'msrpc' -> "
                                         "'msrpc Microsoft Windows RPC'"),
                          QStringLiteral("PortRemoved 127.0.0.1 445/tcp: 'open' -> ''"),
                          QStringLiteral("HostRemoved 127.0.0.2: 'up' -> ''"),
                          QStringLiteral("PortRemoved 127.0.0.2 1/tcp: 'closed' -> ''"),
                          QStringLiteral("PortRemoved 127.0.0.2 135/tcp: 'open' -> ''"),
                          QStringLiteral("PortRemoved 127.0.0.2 445/tcp: 'open' -> ''")}));
}

QTEST_GUILESS_MAIN(TestScanDiff)
#include "test_scandiff.moc"
