#include "CommandLine.h"

#include <QTest>

class TestCommandLine : public QObject
{
    Q_OBJECT

private slots:
    void splitsAtWhitespace();
    void keepsQuotedTextTogether();
    void rejectsUnclosedQuote();
    void emptyTextGivesNoArguments();
    void splitUndoesJoin();
};

void TestCommandLine::splitsAtWhitespace()
{
    const std::optional<QStringList> result =
        splitCommandLine(QStringLiteral("  -sT   -p 22\t10.0.0.1 "));

    QVERIFY(result.has_value());
    QCOMPARE(*result, QStringList({QStringLiteral("-sT"), QStringLiteral("-p"),
                                   QStringLiteral("22"), QStringLiteral("10.0.0.1")}));
}

void TestCommandLine::keepsQuotedTextTogether()
{
    const std::optional<QStringList> result =
        splitCommandLine(QStringLiteral("--script-args 'user=a b' \"C:\\My Dir\\x\" ''"));

    QVERIFY(result.has_value());
    QCOMPARE(*result, QStringList({QStringLiteral("--script-args"), QStringLiteral("user=a b"),
                                   QStringLiteral("C:\\My Dir\\x"), QString()}));
}

void TestCommandLine::rejectsUnclosedQuote()
{
    QVERIFY(!splitCommandLine(QStringLiteral("-sT 'scanme.nmap.org")).has_value());
}

void TestCommandLine::emptyTextGivesNoArguments()
{
    const std::optional<QStringList> result = splitCommandLine(QStringLiteral("   "));

    QVERIFY(result.has_value());
    QVERIFY(result->isEmpty());
}

// The shown command is what the user edits, so splitting it without changes
// must give back exactly the arguments it was made from.
void TestCommandLine::splitUndoesJoin()
{
    const QStringList arguments = {QStringLiteral("-sT"), QStringLiteral("--exclude"),
                                   QStringLiteral("a b"), QStringLiteral("10.0.0.1")};

    const std::optional<QStringList> result = splitCommandLine(joinCommandLine(arguments));

    QVERIFY(result.has_value());
    QCOMPARE(*result, arguments);
}

QTEST_GUILESS_MAIN(TestCommandLine)
#include "test_commandline.moc"
