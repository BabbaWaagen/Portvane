#include "NmapXmlParser.h"
#include "ScanTableModel.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QTest>

static QList<Host> hostsFromFixture(const QString &name)
{
    QFile file(QDir(QString::fromUtf8(FIXTURES_DIR)).filePath(name));
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return parseNmapXml(file.readAll()).hosts;
}

class TestScanTableModel : public QObject
{
    Q_OBJECT

private slots:
    void showsOneRowPerPort();
    void colorsRowsByPortState();
    void showsHostWithoutPorts();
};

void TestScanTableModel::showsOneRowPerPort()
{
    const QList<Host> hosts = hostsFromFixture(QStringLiteral("two_hosts.xml"));
    QCOMPARE(hosts.size(), 2);
    ScanTableModel model;

    model.setHosts(hosts);

    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(model.columnCount(), 8);

    auto cell = [&model](int row, int column) {
        return model.data(model.index(row, column));
    };
    QCOMPARE(cell(0, ScanTableModel::AddressColumn).toString(), QStringLiteral("127.0.0.1"));
    QCOMPARE(cell(0, ScanTableModel::HostnameColumn).toString(), QStringLiteral("localhost"));
    QCOMPARE(cell(0, ScanTableModel::HostStateColumn).toString(), QStringLiteral("up"));
    QCOMPARE(cell(0, ScanTableModel::PortColumn).toInt(), 1);
    QCOMPARE(cell(0, ScanTableModel::ProtocolColumn).toString(), QStringLiteral("tcp"));
    QCOMPARE(cell(0, ScanTableModel::PortStateColumn).toString(), QStringLiteral("closed"));
    QCOMPARE(cell(0, ScanTableModel::ServiceColumn).toString(), QStringLiteral("tcpmux"));
    QCOMPARE(cell(1, ScanTableModel::PortColumn).toInt(), 135);
    QCOMPARE(cell(1, ScanTableModel::PortStateColumn).toString(), QStringLiteral("open"));
    QCOMPARE(cell(3, ScanTableModel::AddressColumn).toString(), QStringLiteral("127.0.0.2"));
    QCOMPARE(cell(3, ScanTableModel::HostnameColumn).toString(), QString());
}

void TestScanTableModel::colorsRowsByPortState()
{
    ScanTableModel model;
    model.setHosts(hostsFromFixture(QStringLiteral("two_hosts.xml"))
                   + hostsFromFixture(QStringLiteral("filtered.xml")));

    auto background = [&model](int row) {
        return model.data(model.index(row, ScanTableModel::AddressColumn), Qt::BackgroundRole);
    };
    const QVariant closed = background(0);
    const QVariant open = background(1);
    const QVariant filtered = background(6);

    QCOMPARE(model.data(model.index(6, ScanTableModel::PortStateColumn)).toString(),
             QStringLiteral("filtered"));
    QCOMPARE(closed.metaType(), QMetaType::fromType<QColor>());
    QCOMPARE(open.metaType(), QMetaType::fromType<QColor>());
    QCOMPARE(filtered.metaType(), QMetaType::fromType<QColor>());
    QVERIFY(closed != open);
    QVERIFY(closed != filtered);
    QVERIFY(open != filtered);
}

void TestScanTableModel::showsHostWithoutPorts()
{
    ScanTableModel model;

    model.setHosts(hostsFromFixture(QStringLiteral("host_down.xml")));

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, ScanTableModel::AddressColumn)).toString(),
             QStringLiteral("192.0.2.1"));
    QCOMPARE(model.data(model.index(0, ScanTableModel::HostStateColumn)).toString(),
             QStringLiteral("down"));
    QVERIFY(!model.data(model.index(0, ScanTableModel::PortColumn)).isValid());
    QVERIFY(!model.data(model.index(0, ScanTableModel::AddressColumn), Qt::BackgroundRole).isValid());
}

QTEST_GUILESS_MAIN(TestScanTableModel)
#include "test_scantablemodel.moc"
