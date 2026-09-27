#pragma once

#include <QList>
#include <QString>

// The result of one scan, independent of where it came from: a live run, a
// saved file, or one side of a diff.

struct Port
{
    int portid = 0;
    QString protocol;
    QString state;
    QString service;
    QString version;
};

struct Host
{
    QString address;
    QString hostname;
    QString state;
    QList<Port> ports;
};
