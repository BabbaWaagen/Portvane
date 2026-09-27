#include "Privileges.h"

#include <QTest>

// Only the decisions are tested here. What detectPrivileges() returns depends
// on how the test happens to be run, so it is not checked.
class TestPrivileges : public QObject
{
    Q_OBJECT

private slots:
    void allowedOnlyWithBoth();
    void explainsWhatIsMissing();
};

void TestPrivileges::allowedOnlyWithBoth()
{
    QVERIFY(privilegedOptionsAllowed(Privileges{true, true}));
    QVERIFY(!privilegedOptionsAllowed(Privileges{true, false}));
    QVERIFY(!privilegedOptionsAllowed(Privileges{false, true}));
    QVERIFY(!privilegedOptionsAllowed(Privileges{false, false}));
}

void TestPrivileges::explainsWhatIsMissing()
{
    QVERIFY(missingPrivilegesText(Privileges{true, true}).isEmpty());

    const QString noRights = missingPrivilegesText(Privileges{false, true});
    QVERIFY(!noRights.isEmpty());
    QVERIFY(!noRights.contains(QStringLiteral("Npcap")));

    const QString noNpcap = missingPrivilegesText(Privileges{true, false});
    QVERIFY(noNpcap.contains(QStringLiteral("Npcap")));

    const QString neither = missingPrivilegesText(Privileges{false, false});
    QVERIFY(neither.contains(QStringLiteral("Npcap")));
    QVERIFY(neither.size() > noNpcap.size());
}

QTEST_GUILESS_MAIN(TestPrivileges)
#include "test_privileges.moc"
