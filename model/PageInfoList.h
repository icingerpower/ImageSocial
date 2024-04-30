#ifndef PAGEINFOLIST_H
#define PAGEINFOLIST_H

#include <QAbstractTableModel>

class PageInfoList : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PageInfoList(const QString &pagePath,
                          QObject *parent = nullptr);
    void addLinkReviews();
    void addLinkPhotos();
    void remove(const QModelIndex &index);
    QString getLink(const QModelIndex &index) const;

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
    void _loadInSettings();
    static const QString SETTING_FILE_NAME;
    static const QString KEY_VALUES;
    static const int IND_VALUE;
    static const int IND_NAME;
    static const int IND_ID;
};

#endif // PAGEINFOLIST_H
