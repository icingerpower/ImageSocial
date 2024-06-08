#ifndef PAGEINFOLIST_H
#define PAGEINFOLIST_H

#include <QAbstractTableModel>

class PageInfoList : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const QString SETTING_FILE_NAME;
    explicit PageInfoList(const QString &pagePath,
                          QObject *parent = nullptr);
    void addLinkReviews();
    void addLinkPhotos();
    void addLinkPin();
    void addLinkCj();
    void setPageLink(const QString &pageLink);
    void remove(const QModelIndex &index);
    QString getLink(const QModelIndex &index) const;
    QHash<QString, QString> linksFilled() const;
    static void sortLinkNames(QStringList &linkNames);
    bool hasPinLink() const;
    QString getInfoExtra() const;

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<QVariantList> m_listOfVariantList;
    QString m_pagePath;
    void _saveInSettings();
    void _loadFromSettings();
    static const QString KEY_VALUES;
    static const int IND_VALUE;
    static const int IND_NAME;
    static const int IND_ID;
    static const QString ID_SOURCING_LINK;
    static const QString NAME_SOURCING_LINK;
    static const QString ID_PHOTO_LINK;
    static const QString ID_REVIEW_LINK;
    static const QString NAME_PHOTO_LINK;
    static const QString NAME_REVIEW_LINK;
    static const QString ID_CJ_SOURCING_ID;
    static const QString ID_CJ_SKU;
    static const QString ID_CJ_LINK;
    static const QString NAME_CJ_SOURCING_ID;
    static const QString NAME_CJ_SKU;
    static const QString NAME_CJ_LINK;
    static const QString ID_PIN_LINK;
    static const QString NAME_PIN_LINK;
    static const QString ID_PAGE_LINK;
    static const QString NAME_PAGE_LINK;
    static const QString ID_EXTRA_INFOS;
    static const QString NAME_EXTRA_INFOS;
};

#endif // PAGEINFOLIST_H
