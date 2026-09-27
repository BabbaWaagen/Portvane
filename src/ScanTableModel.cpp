#include "ScanTableModel.h"

#include <QColor>

// Translucent, so the text on top stays readable in light and dark themes.
static QVariant portStateColor(const QString &state)
{
    if (state == QLatin1String("open"))
        return QColor(0, 170, 0, 70);
    if (state == QLatin1String("closed"))
        return QColor(220, 0, 0, 70);
    if (state == QLatin1String("filtered"))
        return QColor(230, 160, 0, 70);
    return QVariant();
}

ScanTableModel::ScanTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ScanTableModel::setHosts(const QList<Host> &hosts)
{
    beginResetModel();
    m_hosts = hosts;
    m_rows.clear();
    for (qsizetype hostIndex = 0; hostIndex < m_hosts.size(); ++hostIndex) {
        const qsizetype portCount = m_hosts.at(hostIndex).ports.size();
        if (portCount == 0)
            m_rows << Row{hostIndex, -1};
        for (qsizetype portIndex = 0; portIndex < portCount; ++portIndex)
            m_rows << Row{hostIndex, portIndex};
    }
    endResetModel();
}

int ScanTableModel::rowCount(const QModelIndex &parent) const
{
    // A table has no children; Qt asks with a valid parent to find that out.
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int ScanTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant ScanTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Row &row = m_rows.at(index.row());
    const Host &host = m_hosts.at(row.hostIndex);
    const Port *port = row.portIndex >= 0 ? &host.ports.at(row.portIndex) : nullptr;

    if (role == Qt::BackgroundRole)
        return port ? portStateColor(port->state) : QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column()) {
    case AddressColumn:
        return host.address;
    case HostnameColumn:
        return host.hostname;
    case HostStateColumn:
        return host.state;
    }

    if (!port)
        return QVariant();

    switch (index.column()) {
    case PortColumn:
        return port->portid; // an int, so the column sorts 22 before 135
    case ProtocolColumn:
        return port->protocol;
    case PortStateColumn:
        return port->state;
    case ServiceColumn:
        return port->service;
    case VersionColumn:
        return port->version;
    }
    return QVariant();
}

QVariant ScanTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    switch (section) {
    case AddressColumn:
        return tr("Address");
    case HostnameColumn:
        return tr("Hostname");
    case HostStateColumn:
        return tr("Host state");
    case PortColumn:
        return tr("Port");
    case ProtocolColumn:
        return tr("Protocol");
    case PortStateColumn:
        return tr("State");
    case ServiceColumn:
        return tr("Service");
    case VersionColumn:
        return tr("Version");
    }
    return QVariant();
}
