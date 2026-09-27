#pragma once

#include <QString>
#include <QStringList>

#include <optional>

// Joins arguments into one line for display. Arguments with spaces are put in
// double quotes, so splitCommandLine() gets the same list back.
QString joinCommandLine(const QStringList &arguments);

// Splits a hand-edited command into arguments: at whitespace, keeping text in
// '...' or "..." together. This is deliberately not a shell: no escapes, no
// variables, no quotes inside quotes. Returns std::nullopt if a quote is left
// open, rather than guessing where it should end.
std::optional<QStringList> splitCommandLine(const QString &text);
