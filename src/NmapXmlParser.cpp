#include "NmapXmlParser.h"

#include <QCoreApplication>
#include <QXmlStreamReader>

// Every reader below leaves the reader on the end tag of the element it was
// called for, so the caller's readNextStartElement() loop can simply go on.

// Copies the value into a QString right away: the QStringView that
// QXmlStreamAttributes::value() returns is only valid until the reader moves.
static QString attribute(const QXmlStreamReader &reader, QLatin1String name)
{
    return reader.attributes().value(name).toString();
}

static Port readPort(QXmlStreamReader &reader)
{
    Port port;
    port.portid = attribute(reader, QLatin1String("portid")).toInt();
    port.protocol = attribute(reader, QLatin1String("protocol"));

    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("state")) {
            port.state = attribute(reader, QLatin1String("state"));
        } else if (reader.name() == QLatin1String("service")) {
            port.service = attribute(reader, QLatin1String("name"));
            // Without -sV nmap only guesses the service name, and both are empty.
            const QString product = attribute(reader, QLatin1String("product"));
            const QString version = attribute(reader, QLatin1String("version"));
            port.version = (product + QLatin1Char(' ') + version).trimmed();
        }
        // All children of <port> carry their data in attributes, so the rest
        // of each one (e.g. <cpe> inside <service>) can be skipped.
        reader.skipCurrentElement();
    }
    return port;
}

static QList<Port> readPorts(QXmlStreamReader &reader)
{
    QList<Port> ports;
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("port"))
            ports << readPort(reader);
        else
            reader.skipCurrentElement(); // <extraports> is only a summary count
    }
    return ports;
}

static QString readFirstHostname(QXmlStreamReader &reader)
{
    QString hostname;
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("hostname") && hostname.isEmpty())
            hostname = attribute(reader, QLatin1String("name"));
        reader.skipCurrentElement();
    }
    return hostname;
}

static Host readHost(QXmlStreamReader &reader)
{
    Host host;
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("status")) {
            host.state = attribute(reader, QLatin1String("state"));
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("address")) {
            // A host on the local network also lists its MAC address. The IP
            // address is the one the user scanned, so that one is kept.
            if (host.address.isEmpty()
                && attribute(reader, QLatin1String("addrtype")) != QLatin1String("mac"))
                host.address = attribute(reader, QLatin1String("addr"));
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("hostnames")) {
            host.hostname = readFirstHostname(reader);
        } else if (reader.name() == QLatin1String("ports")) {
            host.ports = readPorts(reader);
        } else {
            reader.skipCurrentElement();
        }
    }
    return host;
}

ParseResult parseNmapXml(const QByteArray &xml)
{
    ParseResult result;
    QXmlStreamReader reader(xml);

    if (!reader.readNextStartElement() || reader.name() != QLatin1String("nmaprun")) {
        result.error = reader.hasError()
            ? reader.errorString()
            : QCoreApplication::translate("NmapXmlParser", "The file is not nmap XML output.");
        return result;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("host"))
            result.hosts << readHost(reader);
        else
            reader.skipCurrentElement();
    }

    // Also catches a file that just stops, e.g. because nmap was killed.
    if (reader.hasError()) {
        result.hosts.clear();
        result.error = QCoreApplication::translate("NmapXmlParser",
                                                   "Invalid nmap XML at line %1: %2")
                           .arg(reader.lineNumber())
                           .arg(reader.errorString());
    }
    return result;
}
