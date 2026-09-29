#include "DiffTableModel.h"

#include <QColor>

// Same translucent colours as the results table, so they read the same way:
// green is something new or opened, red something gone or closed.
static const QColor Better(0, 170, 0, 70);
static const QColor Worse(220, 0, 0, 70);
static const QColor Different(230, 160, 0, 70);

static bool isOpenOrUp(const QString &state)
{
    return state == QLatin1String("open") || state == QLatin1String("up");
}

static QVariant changeColor(const Change &change)
{
    switch (change.kind) {
    case ChangeKind::HostAdded:
    case ChangeKind::PortAdded:
        return Better;
    case ChangeKind::HostRemoved:
    case ChangeKind::PortRemoved:
        return Worse;
    case ChangeKind::HostStateChanged:
    case ChangeKind::PortStateChanged:
        if (isOpenOrUp(change.after))
            return Better;
        if (isOpenOrUp(change.before))
            return Worse;
        return Different;
    case ChangeKind::ServiceChanged:
        return Different;
    }
    return QVariant();
}

static QString changeText(ChangeKind kind)
{
    switch (kind) {
    case ChangeKind::HostAdded:
        return DiffTableModel::tr("Host added");
    case ChangeKind::HostRemoved:
        return DiffTableModel::tr("Host removed");
    case ChangeKind::HostStateChanged:
        return DiffTableModel::tr("Host state");
    case ChangeKind::PortAdded:
        return DiffTableModel::tr("Port added");
    case ChangeKind::PortRemoved:
        return DiffTableModel::tr("Port removed");
    case ChangeKind::PortStateChanged:
        return DiffTableModel::tr("Port state");
    case ChangeKind::ServiceChanged:
        return DiffTableModel::tr("Service");
    }
    return QString();
}

// Saying "not listed" rather than "closed" is deliberate, see diffScans().
static QString notListed()
{
    return DiffTableModel::tr("not listed");
}

DiffTableModel::DiffTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void DiffTableModel::setChanges(const QList<Change> &changes)
{
    beginResetModel();
    m_changes = changes;
    endResetModel();
}

int DiffTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_changes.size());
}

int DiffTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant DiffTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Change &change = m_changes.at(index.row());
    if (role == Qt::BackgroundRole)
        return changeColor(change);
    if (role != Qt::DisplayRole)
        return QVariant();

    // Port 0 is a valid port, so the protocol tells the two kinds apart.
    const bool isPortChange = !change.protocol.isEmpty();
    const bool isAdded = change.kind == ChangeKind::HostAdded || change.kind == ChangeKind::PortAdded;
    const bool isRemoved =
        change.kind == ChangeKind::HostRemoved || change.kind == ChangeKind::PortRemoved;
    switch (index.column()) {
    case ChangeColumn:
        return changeText(change.kind);
    case AddressColumn:
        return change.address;
    case PortColumn:
        return isPortChange ? QVariant(change.portid) : QVariant();
    case ProtocolColumn:
        return change.protocol;
    case BeforeColumn:
        return isAdded ? notListed() : change.before;
    case AfterColumn:
        return isRemoved ? notListed() : change.after;
    }
    return QVariant();
}

QVariant DiffTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    switch (section) {
    case ChangeColumn:
        return tr("Change");
    case AddressColumn:
        return tr("Address");
    case PortColumn:
        return tr("Port");
    case ProtocolColumn:
        return tr("Protocol");
    case BeforeColumn:
        return tr("Before");
    case AfterColumn:
        return tr("After");
    }
    return QVariant();
}
