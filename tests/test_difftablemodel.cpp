#include "DiffTableModel.h"

#include <QColor>
#include <QTest>

class TestDiffTableModel : public QObject
{
    Q_OBJECT

private slots:
    void showsOneRowPerChange();
    void colorsByDirectionOfChange();
};

// Rows: 0 host removed, 1 port removed, 2 port state changed to open,
// 3 service changed from nothing to ssh, 4 host added.
static QList<Change> sampleChanges()
{
    return {
        Change{ChangeKind::HostRemoved, QStringLiteral("10.0.0.1"), QString(), 0,
               QStringLiteral("up"), QString()},
        Change{ChangeKind::PortRemoved, QStringLiteral("10.0.0.1"), QStringLiteral("tcp"), 80,
               QStringLiteral("open"), QString()},
        Change{ChangeKind::PortStateChanged, QStringLiteral("10.0.0.2"), QStringLiteral("tcp"), 22,
               QStringLiteral("closed"), QStringLiteral("open")},
        Change{ChangeKind::ServiceChanged, QStringLiteral("10.0.0.2"), QStringLiteral("tcp"), 22,
               QString(), QStringLiteral("ssh")},
        Change{ChangeKind::HostAdded, QStringLiteral("10.0.0.3"), QString(), 0, QString(),
               QStringLiteral("up")},
    };
}

void TestDiffTableModel::showsOneRowPerChange()
{
    DiffTableModel model;

    model.setChanges(sampleChanges());

    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(model.columnCount(), 6);

    auto cell = [&model](int row, int column) {
        return model.data(model.index(row, column));
    };
    QCOMPARE(cell(0, DiffTableModel::ChangeColumn).toString(), QStringLiteral("Host removed"));
    QVERIFY(!cell(0, DiffTableModel::PortColumn).isValid());
    QCOMPARE(cell(0, DiffTableModel::AfterColumn).toString(), QStringLiteral("not listed"));

    QCOMPARE(cell(1, DiffTableModel::AddressColumn).toString(), QStringLiteral("10.0.0.1"));
    QCOMPARE(cell(1, DiffTableModel::PortColumn).toInt(), 80);
    QCOMPARE(cell(1, DiffTableModel::ProtocolColumn).toString(), QStringLiteral("tcp"));
    QCOMPARE(cell(1, DiffTableModel::BeforeColumn).toString(), QStringLiteral("open"));
    QCOMPARE(cell(1, DiffTableModel::AfterColumn).toString(), QStringLiteral("not listed"));

    QCOMPARE(cell(2, DiffTableModel::BeforeColumn).toString(), QStringLiteral("closed"));
    QCOMPARE(cell(2, DiffTableModel::AfterColumn).toString(), QStringLiteral("open"));

    // An empty service is just empty; "not listed" is only for missing items.
    QCOMPARE(cell(3, DiffTableModel::BeforeColumn).toString(), QString());

    QCOMPARE(cell(4, DiffTableModel::BeforeColumn).toString(), QStringLiteral("not listed"));
    QCOMPARE(cell(4, DiffTableModel::AfterColumn).toString(), QStringLiteral("up"));
}

void TestDiffTableModel::colorsByDirectionOfChange()
{
    DiffTableModel model;
    model.setChanges(sampleChanges());

    auto background = [&model](int row) {
        return model.data(model.index(row, DiffTableModel::AddressColumn), Qt::BackgroundRole);
    };
    const QVariant removed = background(0);
    const QVariant opened = background(2);
    const QVariant serviceChanged = background(3);
    const QVariant added = background(4);

    QCOMPARE(added.metaType(), QMetaType::fromType<QColor>());
    QCOMPARE(removed, background(1));
    QCOMPARE(opened, added); // opening a port is as good as a new one
    QVERIFY(added != removed);
    QVERIFY(serviceChanged != added);
    QVERIFY(serviceChanged != removed);
}

QTEST_GUILESS_MAIN(TestDiffTableModel)
#include "test_difftablemodel.moc"
