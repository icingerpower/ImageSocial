#include <QFileSystemModel>
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include <QTime>
#include <QMediaMetaData>

#include "../common/ffmpeg/FFmpegCommands.h"

#include "ResizableRect.h"

#include "DialogVideoEditor.h"
#include "ui_DialogVideoEditor.h"

//----------------------------------------
const QString DialogVideoEditor::KEY_RATIO_RECT = "ratioRectVideo";
const QString DialogVideoEditor::KEY_RATIO_RECT_VIDEO = "hasRatioRectVideo";
const QString DialogVideoEditor::DIR_NAME_TEMP_FFMPEG = "ffmpeg";
//----------------------------------------
DialogVideoEditor::DialogVideoEditor(
        const QString &dirFiles,
        const QString &outVideoFilePath,
        const QSize &sizeRect,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVideoEditor)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(&m_scene);
    m_dirFiles = dirFiles;
    m_outVideoFilePath = outVideoFilePath;
    ui->graphicsView->setScene(&m_scene);
    QFileSystemModel *fileSystemModel
            = new QFileSystemModel(ui->treeViewFiles);
    ui->treeViewFiles->setModel(fileSystemModel);
    ui->treeViewFiles->header()->resizeSection(0, 300);
    fileSystemModel->setRootPath(dirFiles);
    fileSystemModel->setFilter(
                QDir::Files | QDir::NoDotAndDotDot);
    auto rootIndex = fileSystemModel->index(
                    fileSystemModel->rootPath());
    ui->treeViewFiles
            ->setRootIndex(rootIndex);
    m_mediaPlayer = new QMediaPlayer;
    QStringList fileNames = QDir(dirFiles).entryList(QDir::Files, QDir::Name);
    //m_videoWidget = new QVideoWidget();
    //m_mediaPlayer->setVideoOutput(m_videoWidget);

    QSettings settings;
    bool hasRatioRect = settings.value(KEY_RATIO_RECT_VIDEO, true).toBool();
    if (hasRatioRect) {
        ui->checkBoxKeepRatio->setChecked(true);
    }
    if (!sizeRect.isNull()) {
        ui->spinBoxRatioX->setValue(sizeRect.width());
        ui->spinBoxRatioY->setValue(sizeRect.height());
    } else if (settings.contains(KEY_RATIO_RECT)) {
        QSize size = settings.value(KEY_RATIO_RECT).toSize();
        ui->spinBoxRatioX->setValue(size.width());
        ui->spinBoxRatioY->setValue(size.height());
    }

    m_videoSceneItem = new QGraphicsVideoItem;
    m_mediaPlayer->setVideoOutput(m_videoSceneItem);
    m_scene.addItem(m_videoSceneItem);
    m_videoSceneItem->setSize(QSize(m_scene.width(), m_scene.height()));
    m_videoSceneItem->setPos(0, 0);

    m_scene.setSceneRect(QRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
    //m_proxyWidgetPlayer = m_scene.addWidget(m_videoWidget);
    //m_proxyWidgetPlayer->resize(m_scene.width(), m_scene.height());
    //m_proxyWidgetPlayer->setPos(0, 0);


    QRectF rectSel(0,
                      0,
                      ui->spinBoxRatioX->value(),
                      ui->spinBoxRatioY->value());
    for (auto it = fileNames.begin();
         it != fileNames.end(); ++it) {
        m_fileNameToIntervals[*it] = QList<QStringList>();
        m_fileNameToRects[*it] = rectSel;
        m_fileNameToPos[*it] = QPointF(0, 0);
    }

    if (ui->checkBoxKeepRatio->isChecked()) {
        QSize sizeRatio(ui->spinBoxRatioX->value(),
                        ui->spinBoxRatioY->value());
        m_rectSel = new ResizableRect(Qt::green, sizeRatio);
    } else {
        m_rectSel = new ResizableRect(Qt::green);
    }
    m_rectSel->setRect(rectSel);
    m_rectSel->setCallbackOnResized(
                [this](const QPointF &pos, const QRectF &rect){
        _updateRectSel(pos, rect);
    });
    m_scene.addItem(m_rectSel);

    _connectSlots();
}
//----------------------------------------
DialogVideoEditor::~DialogVideoEditor()
{
    delete ui;
    delete m_mediaPlayer;
    delete m_videoSceneItem;
    delete m_rectSel;
}
//----------------------------------------
void DialogVideoEditor::playPause()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause();
        ui->buttonPlayPause->setText("Play");
    } else {
        m_mediaPlayer->play();
        ui->buttonPlayPause->setText("Pause");
    }
}
//----------------------------------------
void DialogVideoEditor::startInterval()
{
    auto selIndexes = ui->treeViewFiles->selectionModel()->selectedIndexes();
    int nRows = ui->tableWidgetIntervals->rowCount();
    if (selIndexes.size() > 0
            && (nRows == 0 || !_lastDurationEmpty())) {
        qint64 pos = m_mediaPlayer->position();
        QTime time(0, 0, 0);
        time = time.addMSecs(pos);
        ui->tableWidgetIntervals->setRowCount(nRows + 1);
        auto itemLeft = new QTableWidgetItem(time.toString("hh:mm:ss zzz"));
        ui->tableWidgetIntervals->setItem(nRows, 0, itemLeft);
        auto itemRight = new QTableWidgetItem(QString());
        ui->tableWidgetIntervals->setItem(nRows, 1, itemRight);
        _saveCurrentIntervals();
    }
}
//----------------------------------------
void DialogVideoEditor::endInterval()
{
    auto selIndexes = ui->treeViewFiles->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0 && _lastDurationEmpty()) {
        int nRows = ui->tableWidgetIntervals->rowCount();
        auto itemRight = ui->tableWidgetIntervals->item(nRows-1, 1);
        qint64 pos = m_mediaPlayer->position();
        QTime time(0, 0, 0);
        time = time.addMSecs(pos);
        itemRight->setText(time.toString("hh:mm:ss zzz"));
        _saveCurrentIntervals();
    }
}
//----------------------------------------
void DialogVideoEditor::_saveCurrentIntervals()
{
    auto selIndexes = ui->treeViewFiles->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        QList<QStringList> values;
        int nRows = ui->tableWidgetIntervals->rowCount();
        for (int i=0; i<nRows; ++i) {
            QStringList line;
            line << ui->tableWidgetIntervals->item(i, 0)->text();
            line << ui->tableWidgetIntervals->item(i, 1)->text();
            values << line;
        }
        QString fileName = selIndexes.first().data().toString();
        m_fileNameToIntervals[fileName] = values;
    }
}
//----------------------------------------
void DialogVideoEditor::removeInterval()
{
    auto selItems = ui->tableWidgetIntervals->selectedItems();
    if (selItems.size() > 0) {
        int row = selItems.first()->row();
        ui->tableWidgetIntervals->removeRow(row);
        _saveCurrentIntervals();
    }
}
//----------------------------------------
void DialogVideoEditor::exportVideo()
{
    auto fileInfos = QDir(m_dirFiles).entryInfoList(
                QDir::Files, QDir::Name);
    QDir ffmpegDir = QDir(m_dirFiles).filePath(DIR_NAME_TEMP_FFMPEG);
    ffmpegDir.mkpath(".");
    QHash<QString, QStringList> videoFileToImages;
    QStringList videoFileNamesToConcat;
    QString videoFileName;
    QMediaPlayer player;
    int nImagesToVideo = 0;
    QTime timeZero(0, 0, 0);
    FFmpegCommands ffmpegCommands;
    for (auto it = fileInfos.begin();
         it != fileInfos.end(); ++it) {
        QString fileName = it->fileName();
        auto rect = m_fileNameToRects[fileName];
        auto pos = m_fileNameToPos[fileName];
        auto intervals = m_fileNameToIntervals[fileName];
        bool isImage = fileName.endsWith(".jpg");
        if (isImage) {
            if (videoFileName.isEmpty()) {
                videoFileName = it->baseName() + ".mp4";
                ++nImagesToVideo;
                videoFileToImages[videoFileName] = QStringList();
            }
            videoFileToImages[videoFileName] << it->filePath();
        } else {
            if (!videoFileName.isEmpty()) {
                QString videoFilePath = QDir(ffmpegDir).filePath(videoFileName);
                ffmpegCommands.createSlideshowVideo(
                            videoFileToImages[videoFileName],
                            ffmpegDir.path(),
                            videoFilePath);
                videoFileNamesToConcat << videoFileName;
                videoFileName.clear();
            }
            /*
            if (intervals.size() == 0) {
                player.setSource(QUrl::fromLocalFile(it->filePath()));
                player.setPosition(0);
                player.pause();
                qint64 duration = 0.;
                do {
                    duration = player.duration();
                    QCoreApplication::processEvents();
                } while (duration < 0.001);
                auto time = timeZero.addMSecs(duration);
                QStringList interval = {"00:00:00 000", time.toString("hh:mm:ss zzz")};
                intervals << interval;
            }
            //*/
            if (intervals.size() == 0) {
                QString outVideoFilePath
                        = ffmpegDir.filePath(it->fileName());
                if (pos.isNull() || rect.isNull()) {
                    QFile::copy(it->filePath(), outVideoFilePath);
                } else {
                    ffmpegCommands.extractVideoClip(
                                it->filePath(),
                                outVideoFilePath,
                                pos.toPoint(),
                                rect.toRect(),
                                -1,
                                -1);
                }
                videoFileNamesToConcat << it->fileName();
            } else {
                QString baseFileNameVideoInter = it->baseName();
                baseFileNameVideoInter += "-";
                QString timeFormat("hh:mm:ss zzz");
                for (int i=0; i<intervals.size(); ++i) {
                    QString fileNameVideoInter = baseFileNameVideoInter;
                    fileNameVideoInter += QString::number(i);
                    fileNameVideoInter += ".mp4";
                    videoFileNamesToConcat << fileNameVideoInter;
                    QString outVideoFilePath
                            = ffmpegDir.filePath(fileNameVideoInter);
                    QTime timeNull(0, 0, 0);
                    QTime timeBegin = QTime::fromString(
                                intervals[i][0], timeFormat);
                    QTime timeEnd = QTime::fromString(
                                intervals[i][1], timeFormat);
                    double beginSec = timeNull.msecsTo(timeBegin) / 1000.;
                    double endSec = timeNull.msecsTo(timeEnd) / 1000.;
                    ffmpegCommands.extractVideoClip(
                                it->filePath(),
                                outVideoFilePath,
                                pos.toPoint(),
                                rect.toRect(),
                                beginSec,
                                endSec);
                }
            }
        }
    }
    ffmpegCommands.joinVideoClips(
                videoFileNamesToConcat,
                ffmpegDir.path(),
                m_outVideoFilePath);
    for (auto itFileToDelete = videoFileNamesToConcat.begin();
         itFileToDelete != videoFileNamesToConcat.end(); ++itFileToDelete) {
        QString filePath = ffmpegDir.filePath(*itFileToDelete);
        QFile::remove(filePath);
    }
    ffmpegDir.rmpath(".");
}
//----------------------------------------
void DialogVideoEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    _resize();
}
//----------------------------------------
void DialogVideoEditor::setVisible(bool visible)
{
    QDialog::setVisible(visible);
    if (visible)
    {
        _resize();
    }
}
//----------------------------------------
void DialogVideoEditor::onFileSelectionChanged(
        const QItemSelection &selected,
        const QItemSelection &deselected)
{
    if (selected.size() > 0) {
        QString fileName = selected.indexes().first().data().toString();
        QString filePath = QDir(m_dirFiles).filePath(fileName);
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        m_mediaPlayer->setPosition(0);
        m_mediaPlayer->pause();
        ui->labelTimeCurrent->setText("00:00:00");

        QVariant variantSize;
        do {
            variantSize = m_mediaPlayer->metaData().value(
                        QMediaMetaData::Resolution);
            QCoreApplication::processEvents();
        } while (!variantSize.isValid());
        QSize sizeMax = variantSize.toSize();
        if (m_fileNameToRects[fileName].width() > sizeMax.width()
                || m_fileNameToRects[fileName].height() > sizeMax.height()) {
            QSize sizeCopy(ui->spinBoxRatioX->value(),
                           ui->spinBoxRatioY->value());
            sizeCopy.scale(sizeMax.width(),
                           sizeMax.height(),
                           Qt::KeepAspectRatio);
            m_fileNameToRects[fileName]
                    = QRectF(0, 0, sizeCopy.width(), sizeCopy.height());
        }
        m_rectSel->setRect(m_fileNameToRects[fileName]);
        m_rectSel->setPos(m_fileNameToPos[fileName]);

        int nIntervals = m_fileNameToIntervals[fileName].size();
        ui->tableWidgetIntervals->setRowCount(0);
        ui->tableWidgetIntervals->setRowCount(nIntervals);
        for (int i=0; i<nIntervals; ++i){
            ui->tableWidgetIntervals->setItem(
                        i, 0, new QTableWidgetItem(
                            m_fileNameToIntervals[fileName][i][0]));
            ui->tableWidgetIntervals->setItem(
                        i, 1, new QTableWidgetItem(
                            m_fileNameToIntervals[fileName][i][1]));
        }
        QTimer::singleShot(
                    200,
                    this,
                    &DialogVideoEditor::_resize);
        _resize();
        //m_mediaPlayer->play();
        //int width = m_videoWidget->width();
        //int height = m_videoWidget->height();
        //int TEMP=10;++TEMP;
    } else if (deselected.size() > 0) {
        m_mediaPlayer->setSource(QUrl());
    }
}
//----------------------------------------
void DialogVideoEditor::_setDuration(qint64 duration)
{
    QTime time(0, 0, 0);
    time = time.addMSecs(duration);
    ui->sliderMedia->setMaximum(duration);
    ui->labelTimeMax->setText(time.toString("hh:mm:ss"));
}
//----------------------------------------
void DialogVideoEditor::_updateRectSel(
        const QPointF &pos, const QRectF &rect)
{
    auto selIndexes = ui->treeViewFiles->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        QString fileName = selIndexes.first().data().toString();
        m_fileNameToRects[fileName] = rect;
        m_fileNameToPos[fileName] = pos;
    }
}
//----------------------------------------
void DialogVideoEditor::_saveRatio()
{
    QSize sizeRatio(ui->spinBoxRatioX->value(),
                    ui->spinBoxRatioY->value());
    m_rectSel->setSizeRatio(sizeRatio);
    QSettings settings;
    settings.setValue(KEY_RATIO_RECT, sizeRatio);
}
//----------------------------------------
void DialogVideoEditor::_setHasRatio(bool has)
{
    if (has) {
        QSize size(ui->spinBoxRatioX->value(),
                   ui->spinBoxRatioY->value());
        m_rectSel->setSizeRatio(size);
    } else {
        m_rectSel->setSizeRatio(QSize());
    }
}
//----------------------------------------
void DialogVideoEditor::_resize()
{
    if (isVisible()){
        m_scene.setSceneRect(QRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
        //int width = m_scene.width();
        //int height = m_scene.height();
        QSize size(QSize(m_scene.width(), m_scene.height()));
        QVariant variantSize;
        if (!m_mediaPlayer->source().isEmpty()) {
            do {
                variantSize = m_mediaPlayer->metaData().value(
                            QMediaMetaData::Resolution);
                QCoreApplication::processEvents();
            } while (!variantSize.isValid());
        }
        if (variantSize.isValid()) {
            size = variantSize.toSize();
        } else {
            int TEMP=10;++TEMP;
        }
        m_videoSceneItem->setSize(size);
        m_videoSceneItem->setPos(0, 0);
        //m_proxyWidgetPlayer->resize(m_scene.width(), m_scene.height());
        //m_proxyWidgetPlayer->setPos(0, 0);
    }
}
//----------------------------------------
void DialogVideoEditor::_setPositionSliderLabel(qint64 position)
{
    ui->sliderMedia->setValue(position);
    QTime time(0, 0, 0);
    time = time.addMSecs(position);
    ui->labelTimeCurrent->setText(time.toString("hh:mm:ss"));
}
//----------------------------------------
void DialogVideoEditor::_connectSlots()
{
    connect(ui->buttonStartInverval,
            &QPushButton::clicked,
            this,
            &DialogVideoEditor::startInterval);
    connect(ui->buttonEndInterval,
            &QPushButton::clicked,
            this,
            &DialogVideoEditor::endInterval);
    connect(ui->buttonRemoveInterval,
            &QPushButton::clicked,
            this,
            &DialogVideoEditor::removeInterval);
    connect(ui->buttonExportVideo,
            &QPushButton::clicked,
            this,
            &DialogVideoEditor::exportVideo);
    connect(ui->treeViewFiles->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DialogVideoEditor::onFileSelectionChanged);
    connect(ui->buttonPlayPause,
            &QPushButton::clicked,
            this,
            &DialogVideoEditor::playPause);
    connect(m_mediaPlayer,
            &QMediaPlayer::positionChanged,
            this,
            &DialogVideoEditor::_setPositionSliderLabel);
    connect(m_mediaPlayer,
            &QMediaPlayer::durationChanged,
            this,
            &DialogVideoEditor::_setDuration);
    connect(ui->checkBoxKeepRatio,
            &QCheckBox::clicked,
            this,
            &DialogVideoEditor::_setHasRatio);
    connect(ui->sliderMedia,
            &QSlider::sliderMoved,
            m_mediaPlayer,
            &QMediaPlayer::setPosition);
    connect(ui->spinBoxRatioX,
            &QSpinBox::valueChanged,
            this,
            &DialogVideoEditor::_saveRatio);
    connect(ui->spinBoxRatioY,
            &QSpinBox::valueChanged,
            this,
            &DialogVideoEditor::_saveRatio);
}
//----------------------------------------
bool DialogVideoEditor::_lastDurationEmpty()
{
    int nRows = ui->tableWidgetIntervals->rowCount();
    if (nRows == 0) {
        return false;
    }
    auto lastItemRight = ui->tableWidgetIntervals->item(nRows-1, 1);
    return lastItemRight->text().isEmpty();
}
//----------------------------------------
