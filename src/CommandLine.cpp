#include "CommandLine.h"

QString joinCommandLine(const QStringList &arguments)
{
    QStringList parts;
    for (const QString &argument : arguments) {
        if (argument.contains(QLatin1Char(' ')))
            parts << QLatin1Char('"') + argument + QLatin1Char('"');
        else
            parts << argument;
    }
    return parts.join(QLatin1Char(' '));
}

std::optional<QStringList> splitCommandLine(const QString &text)
{
    QStringList arguments;
    QString current;
    bool inArgument = false;
    QChar openQuote; // null while outside quotes

    for (const QChar c : text) {
        if (!openQuote.isNull()) {
            if (c == openQuote)
                openQuote = QChar();
            else
                current += c;
        } else if (c == QLatin1Char('\'') || c == QLatin1Char('"')) {
            openQuote = c;
            inArgument = true; // so "" still counts as an (empty) argument
        } else if (c.isSpace()) {
            if (inArgument)
                arguments << current;
            current.clear();
            inArgument = false;
        } else {
            current += c;
            inArgument = true;
        }
    }

    if (!openQuote.isNull())
        return std::nullopt;
    if (inArgument)
        arguments << current;
    return arguments;
}
