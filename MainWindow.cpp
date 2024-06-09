#include <QFileDialog>
#include <QStringListModel>
#include <QListView>
#include <QProcess>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QGraphicsProxyWidget>
#include <QWebEngineProfile>
#include <QSettings>
#include <QDebug>

#include "../common/ffmpeg/FFmpegCommands.h"

#include "DialogCreateProductPage.h"
#include "DialogVideoEditor.h"
#include "ResizableRect.h"
#include "model/ImageDrawerAbstract.h"
#include "model/PageInfoList.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

//----------------------------------------
const QString MainWindow::KEY_IMAGE_DIR = "imageDir";
const QString MainWindow::KEY_RATIO_PINTEREST = "ratioPinterest";
const QString MainWindow::KEY_LAST_IMAGE_DIR = "lastImageDir";
const QString MainWindow::KEY_LAST_IMAGE_DIR_TO_CROP = "lastImageDirToCrop";
const QString KEY_LAST_IMAGE_DIR_PINTEREST_MASS_VIDEOS = "lastImageDirMassPinterestVideos";
const QString MainWindow::FILE_NAME_VIDEO = "VIDEO";
const QString MainWindow::DIR_NAME_TEMP_FILES = "tempFiles";
const QString MainWindow::DIR_NAME_TEMP_FFMPEG = "ffmpeg";
//----------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_urlLoaded = false;
    ui->buttonGeneratePinterestVideos->setHidden(true);
    ui->graphicsView->setScene(&m_scene);
    ui->graphicsViewImageToCrop->setScene(&m_sceneToCrop);

    ui->comboBoxBorderImage->addItems(
                ImageDrawerAbstract::allImageDrawerNames());

    ui->buttonCropBoth->setVisible(false);
    m_webView = new QWebEngineView();
    //m_webView->showMinimized();
    //m_webView->setParent(nullptr);
    m_proxyWidgetWeb = m_scene.addWidget(m_webView);
    m_scene.setSceneRect(QRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
    m_proxyWidgetWeb->resize(m_scene.width(), m_scene.height());
    m_proxyWidgetWeb->setPos(0, 0);

    m_pixmapItem = new QGraphicsPixmapItem;
    m_pixmapItem->setPos(0, 0);
    m_sceneToCrop.addItem(m_pixmapItem);

    m_rectSquare = new ResizableRect(Qt::green);
    m_rectSquare->setRect(QRect(0, 0, 380, 380));
    m_scene.addItem(m_rectSquare);
    m_rectSquareToCrop = new ResizableRect(Qt::green);
    m_rectSquareToCrop->setRect(QRect(0, 0, 380, 380));
    m_sceneToCrop.addItem(m_rectSquareToCrop);

    m_rectPinterest = new ResizableRect(Qt::red, QSize(300, 600));
    m_rectPinterest->setRect(QRect(0, 0, 300, 600));
    m_scene.addItem(m_rectPinterest);
    m_rectPinterestToCrop = new ResizableRect(Qt::red, QSize(300, 600));
    m_rectPinterestToCrop->setRect(QRect(0, 0, 300, 600));
    m_sceneToCrop.addItem(m_rectPinterestToCrop);


    QSettings settings;
    if (settings.contains(KEY_IMAGE_DIR)) {
        QString imageDir = settings.value(KEY_IMAGE_DIR).toString();
        ui->lineEditDirImages->setText(imageDir);
    }
    if (settings.contains(KEY_RATIO_PINTEREST)) {
        QSize sizeRatio = settings.value(KEY_RATIO_PINTEREST).toSize();
        ui->spinBoxPinterestWidth->setValue(sizeRatio.width());
        ui->spinBoxPinterestHeight->setValue(sizeRatio.height());
        _updatePinterestRatio();
    }
    if (settings.contains(KEY_LAST_IMAGE_DIR)) {
        auto lastDir = settings.value(KEY_LAST_IMAGE_DIR).toString();
        _setImageDirectory(lastDir);
    }
    if (settings.contains(KEY_LAST_IMAGE_DIR_TO_CROP)) {
        auto lastDir = settings.value(KEY_LAST_IMAGE_DIR_TO_CROP).toString();
        _setImageDirectoryToCrop(lastDir);
    }
    _updateSquareRatio();
    _connectSlots();
}
//----------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//----------------------------------------
void MainWindow::browseUrl()
{
    QString urlString = ui->lineEditUrl->text();
    if (urlString.size() > 4) {
        //QWebEngineProfile *profile = m_webView->page()->profile();
        //auto currentAgent = profile->httpUserAgent();
        //profile->setHttpUserAgent("AppleWebKit/537.36");
        ui->lineEditBaseName->clear();
        QUrl url(urlString);
        m_webView->load(url);
    }
}
//----------------------------------------
void MainWindow::exportPagesCsv()
{
    if (!ui->lineEditDirImages->text().isEmpty())
    {
        QStringList baseNames = _getBaseNames();
        std::sort(baseNames.begin(), baseNames.end());
        QDir dirImages = ui->lineEditDirImages->text();
        QList<QHash<QString, QString>> listOfLinks;
        QSet<QString> setColNames;
        for (const auto &baseName : baseNames)
        {
            QString pagePath = dirImages.filePath(baseName);
            PageInfoList infoList(pagePath);
            listOfLinks << infoList.linksFilled();
            listOfLinks.last()["Name"] = baseName;
            const auto &links = listOfLinks.last();
            for (auto it = links.begin(); it != links.end(); ++it)
            {
                setColNames << it.key();
            }
        }
        QStringList colNames{setColNames.begin(), setColNames.end()};
        PageInfoList::sortLinkNames(colNames);
        colNames.insert(0, "Image");
        colNames.insert(1, "Name");
        QString sep{"\t"};
        QStringList lines{colNames.join(sep)};
        for (const auto &links : listOfLinks)
        {
            QStringList lineElements;
            for (const auto &colName : colNames)
            {
                lineElements << links.value(colName, QString{});
            }
            lines << lineElements.join(sep);
        }
        QString csvFilePath = dirImages.filePath(dirImages.dirName() + ".csv");
        QFile file(csvFilePath);
        if (file.open(QFile::WriteOnly))
        {
            QTextStream stream(&file);
            stream << lines.join("\n");
            file.close();
        }
    }
}
//----------------------------------------
void MainWindow::displayPinToDo()
{
    if (!ui->lineEditDirImages->text().isEmpty())
    {
        QDir dirImages = ui->lineEditDirImages->text();
        QStringList pinsToDo = dirImages.entryList(
                    QStringList{"*.mp4", "*.avi"}, QDir::Files, QDir::Name);

        QStringList baseNames = _getBaseNames();
        std::sort(baseNames.begin(), baseNames.end());
        for (const auto &baseName : baseNames)
        {
            QString pagePath = dirImages.filePath(baseName);
            PageInfoList infoList(pagePath);
            if (!infoList.hasPinLink())
            {
                pinsToDo << baseName;
            }
        }
        if (pinsToDo.size() > 0)
        {
            QDialog dialog;
            dialog.setWindowTitle("Pins to do");
            QListView* listView = new QListView(&dialog);
            QStringListModel* model = new QStringListModel(pinsToDo, listView);
            listView->setModel(model);
            QVBoxLayout* layout = new QVBoxLayout(&dialog);
            layout->addWidget(listView);
            dialog.setLayout(layout);
            dialog.exec();
        }
    }
}
//----------------------------------------
void MainWindow::browseImageDirectory()
{
    QSettings settings;
    QString lastDirPath
            = settings.value(KEY_LAST_IMAGE_DIR, QString()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Image folder"),
                lastDirPath);
    if (!dirPath.isEmpty()) {
        settings.setValue(KEY_LAST_IMAGE_DIR, dirPath);
        _setImageDirectory(dirPath);
    }
}
//----------------------------------------
void MainWindow::browseImageDirectoryToCrop()
{
    QSettings settings;
    QString lastDirPath
            = settings.value(KEY_LAST_IMAGE_DIR_TO_CROP, QString()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Image folder"),
                lastDirPath);
    if (!dirPath.isEmpty()) {
        settings.setValue(KEY_LAST_IMAGE_DIR_TO_CROP, dirPath);
        _setImageDirectoryToCrop(dirPath);
    }
}

void MainWindow::generatePinterestVideos()
{
    QSettings settings;
    QString lastDirPath
            = settings.value(KEY_LAST_IMAGE_DIR_PINTEREST_MASS_VIDEOS, QString()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Image folder with subfolders"),
                lastDirPath);
    if (!dirPath.isEmpty()) {
        FFmpegCommands ffmpegCommands;
        settings.setValue(KEY_LAST_IMAGE_DIR_PINTEREST_MASS_VIDEOS, dirPath);
        QDir dirParent{dirPath};
        const auto &subDirs = dirParent.entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        QStringList ratios{"1-2", "2-3"};
        for (const auto &subDir : subDirs)
        {
            for (const auto &ratio : ratios)
            {
                const QDir &absDir = subDir.absoluteFilePath();
                const auto &pinImages = absDir.entryInfoList(
                    QStringList{"*PIN-" + ratio + "*.jpg"}, QDir::Files, QDir::Name);
                if (pinImages.size() > 4)
                {
                    QFileInfo fileInfo{pinImages.first()};
                    const QString &baseName = fileInfo.baseName().left(fileInfo.size()-3);
                    const QString &outFileName = baseName + ".mp4";
                    const QString &outFilePath = absDir.absoluteFilePath(outFileName);
                    if (!QFile::exists(outFilePath))
                    {
                        const QString &dirTempFFmpeg = absDir.absoluteFilePath("ffmegTempRatio" + ratio);
                        QStringList imageFilePaths;
                        for (const auto &fileInfo : pinImages)
                        {
                            imageFilePaths << fileInfo.absoluteFilePath();
                        }
                        ffmpegCommands.createSlideshowVideo(
                            imageFilePaths, dirTempFFmpeg, outFilePath);
                    }
                }
            }
        }
    }
}
//----------------------------------------
void MainWindow::_setImageDirectory(const QString &dirPath)
{
    ui->lineEditDirImages->setText(dirPath);
    auto model = ui->treeViewImages->model();
    auto modelTempFiles = ui->treeViewTempFiles->model();
    QFileSystemModel *fileSystemModel = nullptr;
    QFileSystemModel *fileSystemModelTemp = nullptr;
    if (model == nullptr) {
        fileSystemModel = new QFileSystemModel(ui->treeViewImages);
        ui->treeViewImages->setModel(fileSystemModel);
        ui->treeViewImages->header()->resizeSection(0, 300);
        fileSystemModelTemp = new QFileSystemModel(ui->treeViewTempFiles);
        ui->treeViewTempFiles->setModel(fileSystemModelTemp);
        ui->treeViewTempFiles->header()->resizeSection(0, 200);
    } else {
        fileSystemModel = static_cast<QFileSystemModel *>(model);
        fileSystemModelTemp = static_cast<QFileSystemModel *>(modelTempFiles);
    }
    _fillComboPageDirs();

    fileSystemModel->setRootPath(dirPath);
    ui->treeViewImages
            ->setRootIndex(
                fileSystemModel->index(
                    fileSystemModel->rootPath()));
    connect(ui->treeViewImages->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::onImageSelectionChanged);

    QDir dirTemp = dirPath;
    QString dirPathTemp = dirTemp.filePath(DIR_NAME_TEMP_FILES);
    dirTemp = dirPathTemp;
    dirTemp.mkpath(".");
    fileSystemModelTemp->setRootPath(dirPathTemp);
    ui->treeViewTempFiles
            ->setRootIndex(
                fileSystemModelTemp->index(
                    fileSystemModelTemp->rootPath()));
    connect(ui->treeViewTempFiles->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::onImageSelectionChangedTemp);
}
//----------------------------------------
void MainWindow::_setImageDirectoryToCrop(const QString &dirPath)
{
    ui->lineEditDirImagesToCrop->setText(dirPath);
    auto model = ui->treeViewImagesToCrop->model();
    QFileSystemModel *fileSystemModel = nullptr;
    if (model == nullptr) {
        fileSystemModel = new QFileSystemModel(ui->treeViewImagesToCrop);
        ui->treeViewImagesToCrop->setModel(fileSystemModel);
        ui->treeViewImagesToCrop->header()->resizeSection(0, 300);
    } else {
        fileSystemModel = static_cast<QFileSystemModel *>(model);
    }

    fileSystemModel->setRootPath(dirPath);
    ui->treeViewImagesToCrop
            ->setRootIndex(
                fileSystemModel->index(
                    fileSystemModel->rootPath()));
    connect(ui->treeViewImagesToCrop->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::onImageSelectionChangedToCrop);
}
//----------------------------------------
QString MainWindow::_nextFilePath(
        const QString &postFix, const QString &ext) const
{
    QDir dirImage = ui->lineEditDirImages->text();
    int lastNumber = _lastNumberFileName(postFix, ext);
    QString fileName = ui->lineEditBaseName->text();
    _fixBaseNameIfNeeded(fileName, postFix);
    fileName += QString::number(lastNumber + 1);
    if (ext.isEmpty()) {
        fileName += ".jpg";
    } else {
        fileName += ".";
        fileName += ext;
    }
    return dirImage.filePath(fileName);
}
//----------------------------------------
void MainWindow::_fixBaseNameIfNeeded(
        QString &baseName, const QString &postFix) const
{
    if (!baseName.endsWith("-")) {
        baseName += "-";
    }
    if (!postFix.isEmpty()) {
        baseName += postFix;
        baseName += "-";
    }
}
//----------------------------------------
int MainWindow::_lastNumberFileName(
        const QString &postFix, const QString &ext) const
{
    QDir dirImage = ui->lineEditDirImages->text();
    QString filterName = ui->lineEditBaseName->text();
    _fixBaseNameIfNeeded(filterName, postFix);
    filterName += "*";
    if (!ext.isEmpty()) {
        filterName += ".";
        filterName += ext;
    }
    QStringList filter({filterName});
    QStringList fileNames = dirImage.entryList(
                filter, QDir::Files, QDir::Name);
    QList<int> numbers;
    for (auto it = fileNames.begin(); it != fileNames.end(); ++it) {
        int lastIndexDash = it->lastIndexOf("-");
        int lastIndexDot = it->lastIndexOf(".");
        QString numberStr = it->mid(lastIndexDash + 1, lastIndexDot - lastIndexDash - 1);
        bool converted = false;
        int numberInt = numberStr.toInt(&converted);
        if (converted) {
            numbers << numberInt;
        }
    }
    if (numbers.size() > 0) {
        std::sort(numbers.begin(), numbers.end());
        return numbers.last();
    }
    return 0;
}
//----------------------------------------
void MainWindow::cropBoth()
{
    if (_confirmPatternNameAndWebPage()) {
        cropSquare();
        cropPinterest();
    }
}
//----------------------------------------
void MainWindow::fileToRight()
{
    auto itemSel = ui->treeViewImages
            ->selectionModel()->selectedIndexes();
    QDir dirFrom = ui->lineEditDirImages->text();
    QDir dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
    QList<int> numbers;
    QStringList fileNamesTo = dirTo.entryList(QDir::Files);
    numbers << 0;
    for (auto itTo = fileNamesTo.begin();
         itTo != fileNamesTo.end(); ++itTo) {
        QString number = itTo->split("-")[0];
        numbers << number.toInt();
    }
    std::sort(numbers.begin(), numbers.end());
    int nextNumber = numbers.last() + 1;
    for (auto it=itemSel.begin();
         it!=itemSel.end(); ++it) {
        if (it->column() == 0) {
            QString fileName = it->data().toString();
            QString filePathFrom = dirFrom.filePath(fileName);
            QString fileNameTo = QString::number(nextNumber).rightJustified(2, '0');
            fileNameTo += "-";
            fileNameTo += fileName;
            QString filePathTo = dirTo.filePath(fileNameTo);
            QFile::copy(filePathFrom, filePathTo);
            ++nextNumber;
        }
    }
}
//----------------------------------------
void MainWindow::fileToLeft()
{
    auto itemSel = ui->treeViewTempFiles
            ->selectionModel()->selectedIndexes();
    QDir dirFrom = ui->lineEditDirImages->text();
    QDir dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
    for (auto it=itemSel.begin();
         it!=itemSel.end(); ++it) {
        if (it->column() == 0) {
            QString fileName = it->data().toString();
            QString filePathTo = dirTo.filePath(fileName);
            QFile::remove(filePathTo);
        }
    }
}
//----------------------------------------
void MainWindow::fileUp()
{
    auto itemSel = ui->treeViewTempFiles
            ->selectionModel()->selectedIndexes();
    if (itemSel.size() > 0){
        QDir dirFrom = ui->lineEditDirImages->text();
        QDir dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
        auto indexSel = itemSel.first();
        if (indexSel.row() > 0) {
            auto indexUp = indexSel.siblingAtRow(indexSel.row() - 1);
            QString fileNameUp = indexUp.data().toString();
            QString filePathUp = dirTo.filePath(fileNameUp);
            QStringList elementsUp = fileNameUp.split("-");
            QString numberUp = elementsUp.takeFirst();

            QString fileNameSel = indexSel.data().toString();
            QString filePathSel = dirTo.filePath(fileNameSel);
            QStringList elementsSel = fileNameSel.split("-");
            QString numberSel = elementsSel.takeFirst();
            elementsSel.insert(0, numberUp);
            QString newFileNameSel = elementsSel.join("-");
            QString newFilePathSel = dirTo.filePath(newFileNameSel);

            elementsUp.insert(0, numberSel);
            QString newFileNameUp = elementsUp.join("-");
            QString newFilePathUp = dirTo.filePath(newFileNameUp);

            QFile::copy(filePathSel, newFilePathSel);
            QFile::remove(filePathSel);

            QFile::copy(filePathUp, newFilePathUp);
            QFile::remove(filePathUp);
        }
    }
}
//----------------------------------------
void MainWindow::fileDown()
{
    auto itemSel = ui->treeViewTempFiles
            ->selectionModel()->selectedIndexes();
    if (itemSel.size() > 0){
        QDir dirFrom = ui->lineEditDirImages->text();
        QDir dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
        int nTempFiles = dirTo.entryList(QDir::Files).count();
        auto indexSel = itemSel.first();
        if (indexSel.row() < nTempFiles - 1) {
            auto indexDown = indexSel.siblingAtRow(indexSel.row() + 1);
            QString fileNameDown = indexDown.data().toString();
            QString filePathDown = dirTo.filePath(fileNameDown);
            QStringList elementsDown = fileNameDown.split("-");
            QString numberDown = elementsDown.takeFirst();

            QString fileNameSel = indexSel.data().toString();
            QString filePathSel = dirTo.filePath(fileNameSel);
            QStringList elementsSel = fileNameSel.split("-");
            QString numberSel = elementsSel.takeFirst();
            elementsSel.insert(0, numberDown);
            QString newFileNameSel = elementsSel.join("-");
            QString newFilePathSel = dirTo.filePath(newFileNameSel);

            elementsDown.insert(0, numberSel);
            QString newFileNameDown = elementsDown.join("-");
            QString newFilePathDown = dirTo.filePath(newFileNameDown);

            QFile::copy(filePathSel, newFilePathSel);
            QFile::remove(filePathSel);

            QFile::copy(filePathDown, newFilePathDown);
            QFile::remove(filePathDown);
        }
    }
}
//----------------------------------------
void MainWindow::cropSquare()
{
    if (_confirmPatternNameAndWebPage()) {
        _setRectVisible(false);
        QPixmap pixmap;
        if (ui->tabWeb->isVisible()) {
            QRectF rectScenteF = m_rectSquare->sceneBoundingRect();
            pixmap = m_webView->grab(rectScenteF.toRect());
        } else {
            QRectF rectScenteF = m_rectSquareToCrop->sceneBoundingRect();
            pixmap = ui->graphicsViewImageToCrop->grab(
                        rectScenteF.toRect());
        }
        QString filePath = _nextFilePath(QString());
        pixmap.save(filePath);
        _addCurrentComboPageDir();
        _setRectVisible(true);
    }
}
//----------------------------------------
void MainWindow::cropSquareFillingBlank()
{
    if (_confirmPatternNameAndWebPage()) {
        _setRectVisible(false);
        QPixmap pixmap;
        if (ui->tabWeb->isVisible()) {
            QRectF rectScenteF = m_rectPinterest->sceneBoundingRect();
            pixmap = m_webView->grab(rectScenteF.toRect());
        } else {
            QRectF rectScenteF = m_rectSquareToCrop->sceneBoundingRect();
            pixmap = ui->graphicsViewImageToCrop->grab(
                        rectScenteF.toRect());
        }
        _setRectVisible(true);
        QString filePath = _nextFilePath(QString());
        if (pixmap.width() < pixmap.height()) {
            QPixmap resizedPixmap(pixmap.height(), pixmap.height());
            resizedPixmap.fill(Qt::white);

            QPainter painter(&resizedPixmap);
            int x = (resizedPixmap.width() - pixmap.width()) / 2;
            int y = (resizedPixmap.height() - pixmap.height()) / 2;
            painter.drawPixmap(x, y, pixmap);
            resizedPixmap.save(filePath);
        } else {
            pixmap.save(filePath);
        }
    }
}
//----------------------------------------
void MainWindow::cropPinterest()
{
    if (_confirmPatternNameAndWebPage()) {
        _setRectVisible(false);
        QPixmap pixmap;
        if (ui->tabWeb->isVisible()) {
            QRectF rectScenteF = m_rectPinterest->sceneBoundingRect();
            pixmap = m_webView->grab(rectScenteF.toRect());
        } else {
            QRectF rectScenteF = m_rectPinterestToCrop->sceneBoundingRect();
            pixmap = ui->graphicsViewImageToCrop->grab(
                        rectScenteF.toRect());
        }
        QString filePath = _nextFilePath("PINTEREST");
        pixmap.save(filePath);
        _setRectVisible(true);
    }
}
//----------------------------------------
void MainWindow::copyForDeepImageResize()
{
    QDir dirImages = ui->lineEditDirImages->text();
    QDir dirImagesResize = dirImages.filePath("forResize");
    dirImagesResize.mkpath(".");
    auto fileInfos = dirImages.entryInfoList(QDir::Files);
    int secsMaxModified = 4*60*60;
    int sizeMinForResize = ui->spinBoxMinSizeResize->value();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    for (auto itFileInfo = fileInfos.begin();
         itFileInfo != fileInfos.end(); ++itFileInfo) {
        QDateTime lastModified = itFileInfo->lastModified();
        if (lastModified.secsTo(lastModified) < secsMaxModified) {
            QString filePath = itFileInfo->absoluteFilePath();
            QPixmap pixmap(filePath);
            int maxSize = qMax(pixmap.width(), pixmap.height());
            if (maxSize < sizeMinForResize) {
                QString filePathTo = dirImagesResize.filePath(
                            itFileInfo->fileName());
                QFile::copy(filePath, filePathTo);
            }
        }
    }
}
//----------------------------------------
void MainWindow::createSlideShowVideo()
{
    if (_confirmPatternName()) {
        QDir dirFrom = ui->lineEditDirImages->text();
        QDir dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
        QDir dirToFfmpeg = dirTo.filePath(DIR_NAME_TEMP_FFMPEG);
        dirToFfmpeg.mkpath(".");
        auto tempFileNames = dirTo.entryInfoList(QDir::Files, QDir::Name);
        QList<QImage> images;
        int minWidth = 99999;
        int minHeight = 999999;
        for (auto it=tempFileNames.begin();
             it != tempFileNames.end(); ++it) {
            images << QImage(it->absoluteFilePath());
            minWidth = qMin(images.last().width(), minWidth);
            minHeight = qMin(images.last().height(), minHeight);
        }
        if (minHeight % 2 > 0) {
            minHeight += 1;
        }
        int i = 1;
        QStringList ffmpegFilePathsToDelete;
        for (auto itIm = images.begin();
             itIm != images.end(); ++itIm) {
            if (itIm->width() > minWidth || minHeight != itIm->height()) {
                *itIm = itIm->scaled(minWidth, minHeight, Qt::IgnoreAspectRatio);
            }
            QString ffmpegFileName("image");
            ffmpegFileName += QString::number(i).rightJustified(2, '0');
            ffmpegFileName += ".jpg";
            QString ffmpegFilePath = dirToFfmpeg.filePath(ffmpegFileName);
            itIm->save(ffmpegFilePath);
            ffmpegFilePathsToDelete << ffmpegFilePath;
            ++i;
        }
        QString videoFilePath = _nextFilePath(
                    FILE_NAME_VIDEO, "mp4");

        QStringList args;
        args << "-y"; // Overwrite output file
        args << "-framerate";
        args << "1/1";
        args << "-i";
        args << "image%2d.jpg"; // Input file pattern
        args << "-r";
        args << "25"; // 1 frame per second
        args << videoFilePath; // Output file

        // Start ffmpeg process
        QProcess ffmpeg;
        QString tempPath = dirToFfmpeg.path();
        ffmpeg.setWorkingDirectory(dirToFfmpeg.path());
        ffmpeg.setProgram("ffmpeg");
        ffmpeg.setArguments(args);
        ffmpeg.start();
        ffmpeg.waitForFinished();
        for (auto it=ffmpegFilePathsToDelete.begin();
             it != ffmpegFilePathsToDelete.end(); ++it){
            QFile::remove(*it);
        }
        dirToFfmpeg.rmpath(".");
    }
}
//----------------------------------------
void MainWindow::saveWithBorder()
{
    auto selIndexes = ui->treeViewImages->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        QDir dirImage = ui->lineEditDirImages->text();
        for (auto it=selIndexes.begin();
             it!=selIndexes.end(); ++it) {
            if (it->column() == 0) {
                QString fileName = dirImage.filePath(it->data().toString());
                QString filePath = dirImage.filePath(fileName);
                QImage image(filePath);
                QString imageDrawerName = ui->comboBoxBorderImage->currentText();
                auto imageDrawer = ImageDrawerAbstract::getDrawer(imageDrawerName);
                imageDrawer->draw(image);

                QStringList elements = fileName.split("-");
                elements.insert(elements.size()-1, imageDrawer->nameFile());
                QString fileNameNew = elements.join("-");
                QString filePathNew = dirImage.filePath(fileNameNew);
                image.save(filePathNew);
            }
        }
    }
}
//----------------------------------------
void MainWindow::editVideo()
{
    if (_confirmPatternName()) {
        QDir dirFrom = ui->lineEditDirImages->text();
        QString dirTo = dirFrom.filePath(DIR_NAME_TEMP_FILES);
        QString videoFilePath = _nextFilePath(
                    FILE_NAME_VIDEO, "mp4");
        QSize size(ui->spinBoxPinterestWidth->value(),
                   ui->spinBoxPinterestHeight->value());
        if (size.height() > 800) {
            size /= 2;
        }
        DialogVideoEditor dialog(dirTo, videoFilePath, size);
        dialog.exec();
    }
}
//----------------------------------------
void MainWindow::createProductPage()
{
    QString baseName = ui->lineEditBaseName->text();
    if (baseName.isEmpty())
    {
        baseName = ui->comboBoxPageDirs->currentText();
    }
    if (baseName.isEmpty())
    {
        QMessageBox::information(
                    this, "No base name", "You need to enter a base name");
        return;
    }
    QDir dirImages = ui->lineEditDirImages->text();
    QDir dirPage = dirImages.filePath(baseName);
    if (!dirPage.exists())
    {
        dirPage.mkpath(".");
        const auto &fileInfos = dirImages.entryInfoList(
                    QStringList{baseName + "*.jpg"}, QDir::Files, QDir::Name);
        for (const auto &fileInfo : fileInfos)
        {
            const QString &filePathTo = dirPage.filePath(fileInfo.fileName());
            QFile::copy(fileInfo.absoluteFilePath(), filePathTo);
        }
    }
    DialogCreateProductPage dialog(dirPage.path());
    dialog.exec();
}
//----------------------------------------
void MainWindow::_setRectVisible(bool visible)
{
    m_rectSquare->setVisible(visible);
    m_rectPinterest->setVisible(visible);
    m_rectSquareToCrop->setVisible(visible);
    m_rectPinterestToCrop->setVisible(visible);
}
//----------------------------------------
bool MainWindow::_confirmPatternName()
{
    if (ui->lineEditBaseName->text().size() < 3) {
        QMessageBox::information(
                    this,
                    tr("No base name"),
                    tr("You need to enter a file base name"));
    } else if (ui->lineEditDirImages->text().size() < 3) {
        QMessageBox::information(
                    this,
                    tr("No image folter"),
                    tr("You need to select an image folder to save image"));
    } else {
        return true;
    }
    return false;
}
//----------------------------------------
bool MainWindow::_confirmPatternNameAndWebPage()
{
    if (!m_urlLoaded && ui->tabWeb->isVisible()) {
        QMessageBox::information(
                    this,
                    tr("No webpage loaded"),
                    tr("You need to load a webpage"));
        return false;
    } else if (ui->tabFolderImages->isVisible()
               && m_pixmapItem->boundingRect().width() == 0) {
        QMessageBox::information(
                    this,
                    tr("No image selected"),
                    tr("You need to select an image"));
        return false;
    }
    return _confirmPatternName();
}
//----------------------------------------
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    _resize();
}
//----------------------------------------
void MainWindow::_resize()
{
    if (isVisible()){
        m_scene.setSceneRect(
                    QRect(0, 0, ui->graphicsView->width(),
                          ui->graphicsView->height()));
        m_proxyWidgetWeb->resize(m_scene.width(), m_scene.height());
        m_proxyWidgetWeb->setPos(0, 0);
        m_sceneToCrop.setSceneRect(
                    QRect(0, 0, ui->graphicsViewImageToCrop->width(),
                          ui->graphicsViewImageToCrop->height()));
        qDebug() << "resized:" << m_scene.width() << "-" << m_scene.height();
    }
}
//----------------------------------------
void MainWindow::_updatePinterestRatio()
{
    QSize sizeRatio(ui->spinBoxPinterestWidth->value(),
                    ui->spinBoxPinterestHeight->value());
    m_rectPinterest->setSizeRatio(sizeRatio);
    m_rectPinterestToCrop->setSizeRatio(sizeRatio);
    QSettings settings;
    settings.setValue(KEY_RATIO_PINTEREST, sizeRatio);
}
//----------------------------------------
void MainWindow::_updateSquareRatio()
{
    if (ui->checkBoxSquareImage->isChecked()) {
        m_rectSquare->setSizeRatio(QSize(100, 100));
        m_rectSquareToCrop->setSizeRatio(QSize(100, 100));
    } else {
        m_rectSquare->setSizeRatio(QSize());
        m_rectSquareToCrop->setSizeRatio(QSize());
    }
}
//----------------------------------------
void MainWindow::onLoadFinished(bool ok)
{
    if (!ok) {
        qWarning() << "URL load failed";
    } else {
        m_urlLoaded = true;
    }
}
//----------------------------------------
void MainWindow::setVisible(bool visible)
{
    QMainWindow::setVisible(visible);
    if (visible) {
        _resize();
    }
}
//----------------------------------------
void MainWindow::onImageSelectionChanged(
        const QItemSelection &selected,
        const QItemSelection &deselected)
{
    if (selected.size() > 0) {
        QDir imageDir = ui->lineEditDirImages->text();
        QString imageFileName = selected.indexes().first().data().toString();
        QString imageFilePath = imageDir.filePath(imageFileName);
        QPixmap pixmap(imageFilePath);
        if (pixmap.width() > 800)
        {
            pixmap = pixmap.scaledToWidth(800);
        }
        ui->labelImageSel->setPixmap(pixmap);
    } else if (selected.size() == 0
               && deselected.size() > 0) {
        ui->labelImageSel->setPixmap(QPixmap());
    }
}
//----------------------------------------
void MainWindow::onImageSelectionChangedToCrop(
        const QItemSelection &selected,
        const QItemSelection &deselected)
{
     if (selected.size() > 0)
     {
        QDir imageDir = ui->lineEditDirImagesToCrop->text();
        QString imageFileName = selected.indexes().first().data().toString();
        QString imageFilePath = imageDir.filePath(imageFileName);
        QPixmap pixmap(imageFilePath);
        m_pixmapItem->setPixmap(pixmap);
        //ui->labelImageSel->setPixmap(pixmap);
    } else if (selected.size() == 0
               && deselected.size() > 0) {
        m_pixmapItem->setPixmap(QPixmap());
    }

}
//----------------------------------------
void MainWindow::onImageSelectionChangedTemp(
        const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0) {
        QDir imageDir = ui->lineEditDirImages->text();
        QDir imageDirTemp = imageDir.filePath(DIR_NAME_TEMP_FILES);
        QString imageFileName = selected.indexes().first().data().toString();
        QString imageFilePath = imageDirTemp.filePath(imageFileName);
        QPixmap pixmap(imageFilePath);
        if (pixmap.width() > 800)
        {
            pixmap = pixmap.scaledToWidth(800);
        }
        ui->labelImageSelTemp->setPixmap(pixmap);
    } else if (selected.size() == 0
               && deselected.size() > 0) {
        ui->labelImageSelTemp->setPixmap(QPixmap());
    }
}
//----------------------------------------
void MainWindow::_connectSlots()
{
    connect(ui->buttonToLeft,
            &QCheckBox::clicked,
            this,
            &MainWindow::fileToLeft);
    connect(ui->buttonToRight,
            &QCheckBox::clicked,
            this,
            &MainWindow::fileToRight);
    connect(ui->buttonUp,
            &QCheckBox::clicked,
            this,
            &MainWindow::fileUp);
    connect(ui->buttonDown,
            &QCheckBox::clicked,
            this,
            &MainWindow::fileDown);
    connect(ui->buttonCopyDeepImageResize,
            &QCheckBox::clicked,
            this,
            &MainWindow::copyForDeepImageResize);
    connect(ui->buttonCreateSlideshowVideo,
            &QCheckBox::clicked,
            this,
            &MainWindow::createSlideShowVideo);
    connect(ui->buttonSaveWithBorder,
            &QCheckBox::clicked,
            this,
            &MainWindow::saveWithBorder);
    connect(ui->buttonEditVideo,
            &QCheckBox::clicked,
            this,
            &MainWindow::editVideo);
    connect(ui->buttonCreateProductPage,
            &QCheckBox::clicked,
            this,
            &MainWindow::createProductPage);
    connect(m_webView,
            &QWebEngineView::loadFinished,
            this,
            &MainWindow::onLoadFinished);
    connect(ui->spinBoxZoom,
            &QDoubleSpinBox::valueChanged,
            m_webView,
            &QWebEngineView::setZoomFactor);
    connect(ui->splitter,
            &QSplitter::splitterMoved,
            this,
            &MainWindow::_resize);
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &MainWindow::browseUrl);
    connect(ui->buttonExportCsvPages,
            &QPushButton::clicked,
            this,
            &MainWindow::exportPagesCsv);
    connect(ui->buttonDisplayPinsToDo,
            &QPushButton::clicked,
            this,
            &MainWindow::displayPinToDo);

    connect(ui->buttonBrowseDirImageToCrop,
            &QPushButton::clicked,
            this,
            &MainWindow::browseImageDirectoryToCrop);
    connect(ui->buttonGeneratePinterestVideos,
            &QPushButton::clicked,
            this,
            &MainWindow::generatePinterestVideos);
    connect(ui->buttonBrowseDirImage,
            &QPushButton::clicked,
            this,
            &MainWindow::browseImageDirectory);
    connect(ui->buttonCropBoth,
            &QPushButton::clicked,
            this,
            &MainWindow::cropBoth);
    connect(ui->buttonCropFillingWhite,
            &QPushButton::clicked,
            this,
            &MainWindow::cropSquareFillingBlank);
    connect(ui->buttonCropSquare,
            &QPushButton::clicked,
            this,
            &MainWindow::cropSquare);
    connect(ui->buttonCropPinterest,
            &QPushButton::clicked,
            this,
            &MainWindow::cropPinterest);
    connect(ui->spinBoxPinterestWidth,
            &QSpinBox::valueChanged,
            this,
            &MainWindow::_updatePinterestRatio);
    connect(ui->spinBoxPinterestHeight,
            &QSpinBox::valueChanged,
            this,
            &MainWindow::_updatePinterestRatio);
    connect(ui->checkBoxSquareImage,
            &QCheckBox::clicked,
            this,
            &MainWindow::_updateSquareRatio);
    connect(ui->checkBoxSquareImage,
            &QCheckBox::clicked,
            this,
            [this](bool clicked) {
        m_rectSquare->setVisible(clicked);
    });
    connect(ui->groupBoxPinterest,
            &QGroupBox::clicked,
            this,
            [this](bool clicked) {
        m_rectPinterest->setVisible(clicked);
    });
}
//----------------------------------------
void MainWindow::_fillComboPageDirs()
{
    QStringList baseNames = _getBaseNames();
    baseNames.insert(0, "");
    ui->comboBoxPageDirs->clear();
    ui->comboBoxPageDirs->addItems(baseNames);
}
//----------------------------------------
QStringList MainWindow::_getBaseNames() const
{
    const QString &imageDirPath = ui->lineEditDirImages->text();
    if (imageDirPath.isEmpty())
    {
        return QStringList{};
    }
    QDir imageDir = imageDirPath;
    const auto &fileInfos = imageDir.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    QSet<QString> baseNamesSet;
    for (const auto &fileInfo : fileInfos)
    {
        const QString &fileName = fileInfo.fileName();
        int index = fileName.lastIndexOf("-");
        baseNamesSet << fileName.left(index);
    }
    QStringList baseNames{baseNamesSet.begin(), baseNamesSet.end()};
    std::sort(baseNames.begin(), baseNames.end(), std::greater<QString>());
    return baseNames;
}
//----------------------------------------
void MainWindow::_addCurrentComboPageDir()
{
    int nItems = ui->comboBoxPageDirs->count();
    QSet<QString> items;
    for (int i=0; i<nItems; ++i)
    {
        items << ui->comboBoxPageDirs->itemText(i);
    }
    const QString &baseName = ui->lineEditBaseName->text();
    if (!items.contains(baseName))
    {
        ui->comboBoxPageDirs->insertItem(1, baseName);
    }
}
//----------------------------------------
