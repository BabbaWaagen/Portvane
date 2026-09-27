#include "OptionsPanel.h"

#include <QAbstractButton>
#include <QTest>

// Finds a radio button or check box by the flag in its label, e.g. "-sS".
static QAbstractButton *button(const OptionsPanel &panel, const QString &flag)
{
    for (QAbstractButton *candidate : panel.findChildren<QAbstractButton *>()) {
        if (candidate->text().contains(QLatin1Char('(') + flag + QLatin1Char(')')))
            return candidate;
    }
    return nullptr;
}

class TestOptionsPanel : public QObject
{
    Q_OBJECT

private slots:
    void greysOutPrivilegedOptions();
    void enablesEverythingWithPrivileges();
};

void TestOptionsPanel::greysOutPrivilegedOptions()
{
    OptionsPanel panel;

    panel.setPrivileges(Privileges{false, false});

    for (const QString &flag : {QStringLiteral("-sS"), QStringLiteral("-sU"), QStringLiteral("-O")}) {
        QAbstractButton *option = button(panel, flag);
        QVERIFY2(option, qPrintable(flag));
        QVERIFY2(!option->isEnabled(), qPrintable(flag));
        QVERIFY2(!option->isHidden(), qPrintable(flag)); // greyed out, not hidden
        QVERIFY2(!option->toolTip().isEmpty(), qPrintable(flag));
    }
    for (const QString &flag : {QStringLiteral("-sT"), QStringLiteral("-sn"), QStringLiteral("-sV"),
                               QStringLiteral("-Pn")}) {
        QAbstractButton *option = button(panel, flag);
        QVERIFY2(option, qPrintable(flag));
        QVERIFY2(option->isEnabled(), qPrintable(flag));
    }
}

void TestOptionsPanel::enablesEverythingWithPrivileges()
{
    OptionsPanel panel;

    panel.setPrivileges(Privileges{true, true});

    for (const QString &flag : {QStringLiteral("-sT"), QStringLiteral("-sS"), QStringLiteral("-sU"),
                               QStringLiteral("-sn"), QStringLiteral("-sV"), QStringLiteral("-O"),
                               QStringLiteral("-Pn")}) {
        QAbstractButton *option = button(panel, flag);
        QVERIFY2(option, qPrintable(flag));
        QVERIFY2(option->isEnabled(), qPrintable(flag));
    }
}

QTEST_MAIN(TestOptionsPanel)
#include "test_optionspanel.moc"
