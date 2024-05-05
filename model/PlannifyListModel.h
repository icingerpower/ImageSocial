#ifndef PLANNIFYLISTMODEL_H
#define PLANNIFYLISTMODEL_H

#include <QAbstractItemModel>
#include <QDateTime>

class PlannifyListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static PlannifyListModel *instance();
    QDateTime getNextDateTime(int maxPinPerDay, const QTime &time) const;
    void planify(const QDateTime &dateTime, const QString &baseName);
    QDateTime planify(const QString &baseName, int maxPinPerDay, const QTime &time);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    explicit PlannifyListModel(QObject *parent = nullptr);
    QList<QVariantList> m_listOfVariantList;
    void _saveInSettings();
    void _loadFromSettings();
};

#endif // PLANNIFYLISTMODEL_H
