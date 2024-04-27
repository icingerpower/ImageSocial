#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMainWindow>
#include <QWebEngineView>
#include <QItemSelection>

class ResizableRect;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    static const QString KEY_IMAGE_DIR;
    static const QString KEY_RATIO_PINTEREST;
    static const QString KEY_LAST_IMAGE_DIR;
    static const QString KEY_LAST_IMAGE_DIR_TO_CROP;
    static const QString FILE_NAME_VIDEO;
    static const QString DIR_NAME_TEMP_FILES;
    static const QString DIR_NAME_TEMP_FFMPEG;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void browseUrl();
    void browseImageDirectory();
    void browseImageDirectoryToCrop();
    void generatePinterestVideos();
    void cropBoth();
    void fileToRight();
    void fileToLeft();
    void fileUp();
    void fileDown();
    void cropSquare();
    void cropSquareFillingBlank();
    void cropPinterest();
    void copyForDeepImageResize();
    void createSlideShowVideo();
    void saveWithBorder();
    void editVideo();

protected slots:
    void resizeEvent(QResizeEvent *event) override;
    void setVisible(bool visible) override;
    void onLoadFinished(bool ok);
    void onImageSelectionChanged(
            const QItemSelection &selected,
            const QItemSelection &deselected);
    void onImageSelectionChangedToCrop(
            const QItemSelection &selected,
            const QItemSelection &deselected);
    void onImageSelectionChangedTemp(
            const QItemSelection &selected,
            const QItemSelection &deselected);

private slots:
    void _resize();
    void _updatePinterestRatio();
    void _updateSquareRatio();

private:
    Ui::MainWindow *ui;
    QWebEngineView *m_webView;
    QGraphicsProxyWidget *m_proxyWidgetWeb;
    QGraphicsScene m_scene;
    QGraphicsScene m_sceneToCrop;
    void _connectSlots();
    void _setRectVisible(bool visible);
    bool _confirmPatternName();
    bool _confirmPatternNameAndWebPage();
    void _setImageDirectory(const QString &dirPath);
    void _setImageDirectoryToCrop(const QString &dirPath);
    int _lastNumberFileName(
            const QString &postFix,
            const QString &ext = QString()) const;
    QString _nextFilePath(
            const QString &postFix,
            const QString &ext = QString()) const;
    void _fixBaseNameIfNeeded(
            QString &baseName,
            const QString &postFix) const;
    bool m_urlLoaded;
    ResizableRect *m_rectSquare;
    ResizableRect *m_rectPinterest;
    QGraphicsPixmapItem *m_pixmapItem;
    ResizableRect *m_rectSquareToCrop;
    ResizableRect *m_rectPinterestToCrop;
};
#endif // MAINWINDOW_H
