#include "ScanOptions.h"

#include <QTest>

class TestScanOptions : public QObject
{
    Q_OBJECT

private slots:
    void defaultsGiveConnectScan();
    void combinesAllOptions();
    void udpScanWithPortRange();
    void pingOnlyLeavesOutPortOptions();
    void neverAddsOutputOptions();
};

void TestScanOptions::defaultsGiveConnectScan()
{
    ScanOptions options;
    options.targets = {QStringLiteral("scanme.nmap.org")};

    QCOMPARE(buildNmapArguments(options),
             QStringList({QStringLiteral("-sT"), QStringLiteral("-T3"),
                          QStringLiteral("scanme.nmap.org")}));
}

void TestScanOptions::combinesAllOptions()
{
    ScanOptions options;
    options.scanType = ScanType::Syn;
    options.ports = QStringLiteral("22, 80,443");
    options.timing = 4;
    options.serviceVersion = true;
    options.osDetection = true;
    options.skipPing = true;
    options.targets = {QStringLiteral("10.0.0.1"), QStringLiteral("192.168.1.0/24")};

    QCOMPARE(buildNmapArguments(options),
             QStringList({QStringLiteral("-sS"), QStringLiteral("-p"), QStringLiteral("22,80,443"),
                          QStringLiteral("-sV"), QStringLiteral("-O"), QStringLiteral("-T4"),
                          QStringLiteral("-Pn"), QStringLiteral("10.0.0.1"),
                          QStringLiteral("192.168.1.0/24")}));
}

void TestScanOptions::udpScanWithPortRange()
{
    ScanOptions options;
    options.scanType = ScanType::Udp;
    options.ports = QStringLiteral("1-1024");
    options.timing = 0;
    options.targets = {QStringLiteral("localhost")};

    QCOMPARE(buildNmapArguments(options),
             QStringList({QStringLiteral("-sU"), QStringLiteral("-p"), QStringLiteral("1-1024"),
                          QStringLiteral("-T0"), QStringLiteral("localhost")}));
}

void TestScanOptions::pingOnlyLeavesOutPortOptions()
{
    ScanOptions options;
    options.scanType = ScanType::PingOnly;
    options.ports = QStringLiteral("80");
    options.serviceVersion = true;
    options.osDetection = true;
    options.skipPing = true;
    options.targets = {QStringLiteral("192.168.1.0/24")};

    QCOMPARE(buildNmapArguments(options),
             QStringList({QStringLiteral("-sn"), QStringLiteral("-T3"), QStringLiteral("-Pn"),
                          QStringLiteral("192.168.1.0/24")}));
}

// The runner adds -oX itself. A second output option from the builder would
// fight with it, so no combination may produce one.
void TestScanOptions::neverAddsOutputOptions()
{
    const QList<ScanType> scanTypes = {ScanType::Connect, ScanType::Syn, ScanType::Udp,
                                       ScanType::PingOnly};
    for (const ScanType scanType : scanTypes) {
        for (int flags = 0; flags < 8; ++flags) {
            ScanOptions options;
            options.scanType = scanType;
            options.ports = QStringLiteral("1-100");
            options.serviceVersion = (flags & 1) != 0;
            options.osDetection = (flags & 2) != 0;
            options.skipPing = (flags & 4) != 0;
            options.targets = {QStringLiteral("localhost")};

            for (const QString &argument : buildNmapArguments(options))
                QVERIFY2(!argument.startsWith(QStringLiteral("-o")), qPrintable(argument));
        }
    }
}

QTEST_GUILESS_MAIN(TestScanOptions)
#include "test_scanoptions.moc"
