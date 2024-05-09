#include <QSettings>
#include <QDir>

#include "PageInfoList.h"

const int PageInfoList::IND_VALUE{0};
const int PageInfoList::IND_NAME{1};
const int PageInfoList::IND_ID{2};
const QString PageInfoList::ID_SOURCING_LINK{"sourcing-link"};
const QString PageInfoList::NAME_SOURCING_LINK{"Sourcing link"};
const QString PageInfoList::SETTING_FILE_NAME{"settings.ini"};
const QString PageInfoList::KEY_VALUES{"pageInfos"};
const QString PageInfoList::ID_PHOTO_LINK{"photo-link"};
const QString PageInfoList::ID_REVIEW_LINK{"review-link"};
const QString PageInfoList::NAME_PHOTO_LINK{"Photo link"};
const QString PageInfoList::NAME_REVIEW_LINK{"Review link"};
const QString PageInfoList::ID_CJ_SOURCING_ID{"cj-sourcing-id"};
const QString PageInfoList::ID_CJ_SKU{"cj-sku"};
const QString PageInfoList::ID_CJ_LINK{"cj-link"};
const QString PageInfoList::NAME_CJ_SOURCING_ID{"CJ Sourcing ID"};
const QString PageInfoList::NAME_CJ_SKU{"CJ SKU"};
const QString PageInfoList::NAME_CJ_LINK{"CJ LINK"};
const QString PageInfoList::ID_PIN_LINK{"pin-link"};
const QString PageInfoList::NAME_PIN_LINK{"Pin Link"};
const QString PageInfoList::ID_PAGE_LINK{"page-link"};
const QString PageInfoList::NAME_PAGE_LINK{"Page Link"};

//----------------------------------------
PageInfoList::PageInfoList(const QString &pagePath, QObject *parent)
    : QAbstractTableModel(parent)
{
    m_pagePath = pagePath;
    m_listOfVariantList << QVariantList{QString{}, NAME_SOURCING_LINK, ID_SOURCING_LINK};
    m_listOfVariantList << QVariantList{QString{}, NAME_PHOTO_LINK, ID_PHOTO_LINK + "-1"};
    m_listOfVariantList << QVariantList{QString{}, NAME_REVIEW_LINK, ID_REVIEW_LINK + "-1"};
    m_listOfVariantList << QVariantList{QString{}, NAME_CJ_SOURCING_ID, ID_CJ_SOURCING_ID}; //
    m_listOfVariantList << QVariantList{QString{}, NAME_CJ_SKU, ID_CJ_SKU}; //
    m_listOfVariantList << QVariantList{QString{}, NAME_CJ_LINK, ID_CJ_LINK}; //
    m_listOfVariantList << QVariantList{QString{}, NAME_PIN_LINK, ID_PIN_LINK + "-1"};
    m_listOfVariantList << QVariantList{QString{}, NAME_PAGE_LINK, ID_PAGE_LINK}; //
    _loadFromSettings();
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
void PageInfoList::addLinkPin()
{
    QSet<QString> photoIds;
    for (const auto &variantList : m_listOfVariantList)
    {
        QString id = variantList[IND_ID].toString();
        if (id.startsWith(ID_PIN_LINK))
        {
            photoIds << id;
        }
    }
    int i = 2;
    QString id = ID_PIN_LINK + "-2";
    while (photoIds.contains(id))
    {
        ++i;
        id = ID_PIN_LINK + "-" + QString::number(i);
    }
    beginInsertRows(QModelIndex{}, m_listOfVariantList.size(), m_listOfVariantList.size());
    m_listOfVariantList << QVariantList{QString{}, NAME_PIN_LINK + " " + QString::number(i), id};
    _saveInSettings();
    endInsertRows();
}
//----------------------------------------
void PageInfoList::setPageLink(const QString &pageLink)
{
    // TODO set page link + fix face swap image selection
    for (int i=0; i<m_listOfVariantList.size(); ++i)
    {
        if (m_listOfVariantList[i][IND_ID].toString() == ID_PAGE_LINK)
        {
            m_listOfVariantList[i][IND_VALUE] = pageLink;
            const auto &indexChanged = index(i, IND_VALUE);
            _saveInSettings();

            emit dataChanged(indexChanged, indexChanged);
        }
    }

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
QHash<QString, QString> PageInfoList::linksFilled() const
{
    QHash<QString, QString> links;
    for (const auto &variantList : m_listOfVariantList)
    {
        const QString &link = variantList[IND_VALUE].toString();
        if (!link.isEmpty())
        {
            const QString &name = variantList[IND_NAME].toString();
            links[name] = link;
        }
    }
    return links;
}

void PageInfoList::sortLinkNames(QStringList &linkNames)
{
    QStringList orders{
        NAME_CJ_SOURCING_ID
                , NAME_CJ_SKU
                , NAME_CJ_LINK
                , NAME_PAGE_LINK
                , NAME_PIN_LINK
                , NAME_SOURCING_LINK
                , NAME_REVIEW_LINK
                , NAME_PHOTO_LINK
    };
    QStringList linkNamesSorted;
    while (orders.size() > 0)
    {
        QString toOrder = orders.takeFirst();
        int nAdded = 0;
        QStringList linkNamesTemp;
        for (int i=linkNames.size() - 1; i >= 0; --i)
        {
            if (linkNames[i].startsWith(toOrder))
            {
                linkNamesTemp << linkNames.takeAt(i);
                ++nAdded;
            }
        }
        if (nAdded > 1)
        {
            std::sort(linkNamesTemp.begin(), linkNamesTemp.end());
        }
        linkNamesSorted << linkNamesTemp;
    }
    linkNames = linkNamesSorted;
}
//----------------------------------------
bool PageInfoList::hasPinLink() const
{
    for (const auto &variantList : m_listOfVariantList)
    {
        if (variantList[IND_ID].toString().startsWith(ID_PIN_LINK))
        {
            return !variantList[IND_VALUE].toString().isEmpty();
        }
    }
    return false;
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
void PageInfoList::_loadFromSettings()
{
    QSettings settings{QDir{m_pagePath}.filePath(SETTING_FILE_NAME), QSettings::IniFormat};
    if (settings.contains(KEY_VALUES))
    {
        auto listOfVariantList = settings.value(KEY_VALUES).value<QList<QVariantList>>();
        QHash<QString, QVariantList> idToValues;
        for (const auto &variantList : listOfVariantList)
        {
            static QSet<QString> failedIds{ID_CJ_SOURCING_ID + "-1"
                        , ID_CJ_SKU + "-1"
                        , ID_CJ_LINK + "-1"
                        , ID_PAGE_LINK + "-1"};
            if (!failedIds.contains(variantList[IND_ID].toString())){
                idToValues[variantList[IND_ID].toString()] = variantList;
            }
        }
        for (auto &variantList : m_listOfVariantList)
        {
            const QString &id = variantList[IND_ID].toString();
            if (idToValues.contains(id))
            {
                variantList = idToValues.take(id);
            }
        }
        for (auto it = idToValues.begin(); it!= idToValues.end(); ++it)
        {
            m_listOfVariantList << it.value();
        }
    }
}
//----------------------------------------

