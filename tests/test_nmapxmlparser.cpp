#include "NmapXmlParser.h"

#include <QDir>
#include <QFile>
#include <QTest>

// The fixtures are real nmap -oX output, captured once, so these tests never
// need nmap itself.
static QByteArray readFixture(const QString &name)
{
    QFile file(QDir(QString::fromUtf8(FIXTURES_DIR)).filePath(name));
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return file.readAll();
}

class TestNmapXmlParser : public QObject
{
    Q_OBJECT

private slots:
    void readsSeveralHosts();
    void readsFilteredPorts();
    void readsHostThatIsDown();
    void readsHostWithoutListedPorts();
    void readsServiceVersion();
    void rejectsTruncatedXml();
    void rejectsOtherXml();
    void rejectsEmptyInput();
};

void TestNmapXmlParser::readsSeveralHosts()
{
    const QByteArray xml = readFixture(QStringLiteral("two_hosts.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml);

    QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
    QCOMPARE(result.hosts.size(), 2);

    const Host &first = result.hosts.at(0);
    QCOMPARE(first.address, QStringLiteral("127.0.0.1"));
    QCOMPARE(first.hostname, QStringLiteral("localhost"));
    QCOMPARE(first.state, QStringLiteral("up"));
    QCOMPARE(first.ports.size(), 3);
    QCOMPARE(first.ports.at(0).portid, 1);
    QCOMPARE(first.ports.at(0).protocol, QStringLiteral("tcp"));
    QCOMPARE(first.ports.at(0).state, QStringLiteral("closed"));
    QCOMPARE(first.ports.at(0).service, QStringLiteral("tcpmux"));
    QCOMPARE(first.ports.at(0).version, QString());
    QCOMPARE(first.ports.at(1).portid, 135);
    QCOMPARE(first.ports.at(1).state, QStringLiteral("open"));

    // The second host has an empty <hostnames> element.
    const Host &second = result.hosts.at(1);
    QCOMPARE(second.address, QStringLiteral("127.0.0.2"));
    QCOMPARE(second.hostname, QString());
    QCOMPARE(second.ports.size(), 3);
}

void TestNmapXmlParser::readsFilteredPorts()
{
    const QByteArray xml = readFixture(QStringLiteral("filtered.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml);

    QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
    QCOMPARE(result.hosts.size(), 1);
    const Host &host = result.hosts.at(0);
    QCOMPARE(host.hostname, QString());
    QCOMPARE(host.ports.size(), 2);
    QCOMPARE(host.ports.at(0).portid, 80);
    QCOMPARE(host.ports.at(0).state, QStringLiteral("filtered"));
    QCOMPARE(host.ports.at(1).portid, 443);
    QCOMPARE(host.ports.at(1).state, QStringLiteral("filtered"));
}

// A down host has no <hostnames> and no <ports> element at all.
void TestNmapXmlParser::readsHostThatIsDown()
{
    const QByteArray xml = readFixture(QStringLiteral("host_down.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml);

    QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
    QCOMPARE(result.hosts.size(), 1);
    const Host &host = result.hosts.at(0);
    QCOMPARE(host.address, QStringLiteral("192.0.2.1"));
    QCOMPARE(host.hostname, QString());
    QCOMPARE(host.state, QStringLiteral("down"));
    QVERIFY(host.ports.isEmpty());
}

// All 40 ports are closed, so nmap only reports them as <extraports>.
void TestNmapXmlParser::readsHostWithoutListedPorts()
{
    const QByteArray xml = readFixture(QStringLiteral("no_open_ports.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml);

    QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
    QCOMPARE(result.hosts.size(), 1);
    QCOMPARE(result.hosts.at(0).state, QStringLiteral("up"));
    QVERIFY(result.hosts.at(0).ports.isEmpty());
}

// With -sV the <service> element has a product and a nested <cpe> element.
void TestNmapXmlParser::readsServiceVersion()
{
    const QByteArray xml = readFixture(QStringLiteral("service_version.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml);

    QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
    QCOMPARE(result.hosts.size(), 1);
    QCOMPARE(result.hosts.at(0).ports.size(), 1);
    const Port &port = result.hosts.at(0).ports.at(0);
    QCOMPARE(port.service, QStringLiteral("msrpc"));
    QCOMPARE(port.version, QStringLiteral("Microsoft Windows RPC"));
}

// What is left behind when nmap is killed mid-write.
void TestNmapXmlParser::rejectsTruncatedXml()
{
    const QByteArray xml = readFixture(QStringLiteral("two_hosts.xml"));
    QVERIFY(!xml.isEmpty());

    const ParseResult result = parseNmapXml(xml.left(xml.size() / 2));

    QVERIFY(!result.error.isEmpty());
    QVERIFY(result.hosts.isEmpty());
}

void TestNmapXmlParser::rejectsOtherXml()
{
    const ParseResult result = parseNmapXml("<html><body/></html>");

    QVERIFY(!result.error.isEmpty());
    QVERIFY(result.hosts.isEmpty());
}

void TestNmapXmlParser::rejectsEmptyInput()
{
    const ParseResult result = parseNmapXml(QByteArray());

    QVERIFY(!result.error.isEmpty());
    QVERIFY(result.hosts.isEmpty());
}

QTEST_GUILESS_MAIN(TestNmapXmlParser)
#include "test_nmapxmlparser.moc"
