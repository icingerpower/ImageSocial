#include <QSettings>
#include <QDir>

#include "PageInfoList.h"

const int PageInfoList::IND_VALUE{0};
const int PageInfoList::IND_NAME{1};
const int PageInfoList::IND_ID{2};
const QString PageInfoList::SETTING_FILE_NAME{"settings.ini"};
const QString PageInfoList::KEY_VALUES{"pageInfos"};
const QString ID_PHOTO_LINK{"photo-link"};
const QString ID_REVIEW_LINK{"review-link"};
const QString NAME_PHOTO_LINK{"Photo link"};
const QString NAME_REVIEW_LINK{"Review link"};

//----------------------------------------
PageInfoList::PageInfoList(const QString &pagePath, QObject *parent)
    : QAbstractTableModel(parent)
{
    m_pagePath = pagePath;
    m_listOfVariantList << QVariantList{QString{}, QString{"Sourcing link"}, QString{"sourcing-link"}};
    m_listOfVariantList << QVariantList{QString{}, NAME_PHOTO_LINK, ID_PHOTO_LINK + "-1"};
    m_listOfVariantList << QVariantList{QString{}, NAME_REVIEW_LINK, ID_REVIEW_LINK + "-1"};
    _loadInSettings();
}
//----------------------------------------
void PageInfoList::addLinkReviews()
{
    QSet<QString> reviewIds;
    for (const auto &variantList : m_listOfVariantList)
    {
        QString id = variantList[IND_ID].toString();
        if (id.startsWith(ID_REVIEW_LINK))
        {
            reviewIds << id;
        }
    }
    int i = 2;
    QString id = ID_REVIEW_LINK + "-2";
    while (reviewIds.contains(id))
    {
        ++i;
        id = ID_REVIEW_LINK + "-" + QString::number(i);
    }
    beginInsertRows(QModelIndex{}, m_listOfVariantList.size(), m_listOfVariantList.size());
    m_listOfVariantList << QVariantList{QString{}, NAME_REVIEW_LINK + " " + QString::number(i), id};
    _saveInSettings();
    endInsertRows();
}
//----------------------------------------
void PageInfoList::addLinkPhotos()
{
    QSet<QString> photoIds;
    for (const auto &variantList : m_listOfVariantList)
    {
        QString id = variantList[IND_ID].toString();
        if (id.startsWith(ID_PHOTO_LINK))
        {
            photoIds << id;
        }
    }
    int i = 2;
    QString id = ID_PHOTO_LINK + "-2";
    while (photoIds.contains(id))
    {
        ++i;
        id = ID_PHOTO_LINK + "-" + QString::number(i);
    }
    beginInsertRows(QModelIndex{}, m_listOfVariantList.size(), m_listOfVariantList.size());
    m_listOfVariantList << QVariantList{QString{}, NAME_PHOTO_LINK + " " + QString::number(i), id};
    _saveInSettings();
    endInsertRows();

}
//----------------------------------------
void PageInfoList::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_listOfVariantList.remove(index.row());
    _saveInSettings();
    endRemoveRows();
}
//----------------------------------------
QString PageInfoList::getLink(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return m_listOfVariantList[index.row()][IND_VALUE].toString();
    }
    return QString{};
}
//----------------------------------------
QVariant PageInfoList::headerData(
    int section,
    Qt::Orientation orientation,
    int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Vertical)
        {
            return m_listOfVariantList[section][IND_NAME];
        }
        else if (orientation == Qt::Horizontal)
        {
            return "Link";
        }
    }
    return QVariant{};
}
//----------------------------------------
int PageInfoList::rowCount(const QModelIndex &) const
{
    return m_listOfVariantList.size();
}
//----------------------------------------
int PageInfoList::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------
QVariant PageInfoList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfVariantList[index.row()][IND_VALUE];
    }
    return QVariant{};
}
//----------------------------------------
bool PageInfoList::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid() && data(index) != value)
    {
        m_listOfVariantList[index.row()][index.column()] = value;
        _saveInSettings();
        emit dataChanged(index, index, QList<int>{role});
        return true;
    }
    return false;
}
//----------------------------------------
Qt::ItemFlags PageInfoList::flags(const QModelIndex &) const
{
    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
//----------------------------------------
void PageInfoList::_saveInSettings()
{
    QSettings settings{QDir{m_pagePath}.filePath(SETTING_FILE_NAME), QSettings::IniFormat};
    settings.setValue(KEY_VALUES, QVariant::fromValue(m_listOfVariantList));
}
//----------------------------------------
void PageInfoList::_loadInSettings()
{
    QSettings settings{QDir{m_pagePath}.filePath(SETTING_FILE_NAME), QSettings::IniFormat};
    if (settings.contains(KEY_VALUES))
    {
        auto listOfVariantList = settings.value(KEY_VALUES).value<QList<QVariantList>>();
        if (listOfVariantList.size() >= m_listOfVariantList.size())
        {
            m_listOfVariantList = listOfVariantList;
        }
    }
}
//----------------------------------------

