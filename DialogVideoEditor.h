#ifndef DIALOGVIDEOEDITOR_H
#define DIALOGVIDEOEDITOR_H

#include <QDialog>
#include <QItemSelection>
#include <QGraphicsProxyWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>

namespace Ui {
class DialogVideoEditor;
}

class ResizableRect;

class DialogVideoEditor : public QDialog
{
    Q_OBJECT

public:
    static const QString KEY_RATIO_RECT;
    static const QString KEY_RATIO_RECT_VIDEO;
    static const QString DIR_NAME_TEMP_FFMPEG;
    explicit DialogVideoEditor(
            const QString &dirFiles,
            const QString &outVideoFilePath,
            const QSize &sizeRect,
            QWidget *parent = nullptr);
    ~DialogVideoEditor();

public slots:
    void playPause();
    void startInterval();
    void endInterval();
    void removeInterval();
    void exportVideo();

protected slots:
    void resizeEvent(QResizeEvent *event) override;
    void setVisible(bool visible) override;
    void onFileSelectionChanged(
            const QItemSelection &selected,
            const QItemSelection &deselected);
private slots:
    void _resize();
    void _setPositionSliderLabel(qint64 position);
    void _setDuration(qint64 duration);
    void _updateRectSel(const QPointF &pos, const QRectF &rect);
    void _saveRatio();
    void _setHasRatio(bool has);

private:
    Ui::DialogVideoEditor *ui;
    QString m_dirFiles;
    QString m_outVideoFilePath;
    void _connectSlots();
    QGraphicsScene m_scene;
    //QGraphicsProxyWidget *m_proxyWidgetPlayer;
    QMediaPlayer *m_mediaPlayer;
    //QVideoWidget *m_videoWidget;
    QGraphicsVideoItem *m_videoSceneItem;
    ResizableRect *m_rectSel;
    bool _lastDurationEmpty();
    QHash<QString, QList<QStringList>> m_fileNameToIntervals;
    QHash<QString, QRectF> m_fileNameToRects;
    QHash<QString, QPointF> m_fileNameToPos;
    void _saveCurrentIntervals();
};

#endif // DIALOGVIDEOEDITOR_H
