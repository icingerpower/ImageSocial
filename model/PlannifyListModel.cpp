#include "../common/config/SettingsManager.h"

#include "PlannifyListModel.h"

const QString KEY_PLANNED_PINS{"plannedPins"};

PlannifyListModel::PlannifyListModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

PlannifyListModel *PlannifyListModel::instance()
{
    static PlannifyListModel instance;
    return &instance;
}

QDateTime PlannifyListModel::getNextDateTime(
    int maxPinPerDay, const QTime &time) const
{
    QMap<QDateTime, int> plannedByDate;
    for (const auto &variantList : m_listOfVariantList)
    {
        const auto &dateTime = variantList[0].toDateTime();
        if (!plannedByDate.contains(dateTime))
        {
            plannedByDate[dateTime] = 0;
        }
        plannedByDate[dateTime] += 1;
    }
    if (plannedByDate.size() == 0)
    {
        QDateTime tomorrow = QDateTime::currentDateTime().addDays(1);
        tomorrow.setTime(time);
        return  tomorrow;
    }
    else if (plannedByDate.last() < maxPinPerDay)
    {
        return plannedByDate.lastKey();
    }
    else
    {
        QDateTime next = plannedByDate.lastKey().addDays(1);
        next.setTime(time);
        return next;
    }
}

void PlannifyListModel::planify(
    const QDateTime &dateTime,
    const QString &baseName)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_listOfVariantList.insert(
        0, QVariantList{dateTime, baseName});
    _saveInSettings();
    endInsertRows();
}

QDateTime PlannifyListModel::planify(
    const QString &baseName, int maxPinPerDay, const QTime &time)
{
    const auto &nextDate = getNextDateTime(maxPinPerDay, time);
    planify(nextDate, baseName);
    return nextDate;
}

QVariant PlannifyListModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            static QStringList header{"Date", "base name"};
            return header[section];
        }
        else
        {
            return QString::number(m_listOfVariantList.size() - section);
        }
    }
    return QVariant{};
}

int PlannifyListModel::rowCount(const QModelIndex &) const
{
    return m_listOfVariantList.size();
}

int PlannifyListModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant PlannifyListModel::data(
    const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant{};
}

void PlannifyListModel::_saveInSettings()
{
    auto settings = SettingsManager::instance()->getSettings();
    if (m_listOfVariantList.size() > 0)
    {
        settings->setValue(KEY_PLANNED_PINS, QVariant::fromValue(m_listOfVariantList));
    }
    else if (settings->contains(KEY_PLANNED_PINS))
    {
        settings->remove(KEY_PLANNED_PINS);
    }
}

void PlannifyListModel::_loadFromSettings()
{
    auto settings = SettingsManager::instance()->getSettings();
    if (settings->contains(KEY_PLANNED_PINS))
    {
        m_listOfVariantList = settings->value(
                    KEY_PLANNED_PINS).value<QList<QVariantList>>();
    }
}
