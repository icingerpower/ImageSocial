#include <QMessageBox>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonObject>
#include <QJsonDocument>

#include "ResizableRect.h"

#include "DialogCreateProductPage.h"
#include "ui_DialogCreateProductPage.h"

//----------------------------------------
DialogCreateProductPage::DialogCreateProductPage(
        const QString &pagePath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCreateProductPage)
{
    ui->setupUi(this);
    m_pagePath = pagePath;
    _initFileTrees();
    _initWebView();
    _initGraphicsView();
    _connectSlots();
    m_aiWasRun = false;
}
//----------------------------------------
DialogCreateProductPage::~DialogCreateProductPage()
{
    m_webView->setParent(nullptr);
    delete ui;
}
//----------------------------------------
void DialogCreateProductPage::_initWebView()
{
    static QWebEngineView webEngineView;
    webEngineView.setParent(nullptr);
    m_webView = &webEngineView;
    auto layout = new QVBoxLayout(ui->pageProductPage);
    layout->addWidget(m_webView);
    ui->pageProductPage->setLayout(layout);
    QUrl urlAddPage("https://pradize.commercehq.com/admin/products/create");
    m_webView->load(urlAddPage);
}
//----------------------------------------
void DialogCreateProductPage::_initFileTrees()
{
    QDir dirSource = m_pagePath;
    dirSource.cd("..");

    QFileSystemModel *fileSystemModelSource = new QFileSystemModel(
                ui->treeViewFilesSource);
    ui->treeViewFilesSource->setModel(fileSystemModelSource);
    fileSystemModelSource->setRootPath(dirSource.path());
    ui->treeViewFilesSource
            ->setRootIndex(
                fileSystemModelSource->index(
                    fileSystemModelSource->rootPath()));

    QFileSystemModel *fileSystemModelPage = new QFileSystemModel(
                ui->treeViewFilesPage);
    ui->treeViewFilesPage->setModel(
                fileSystemModelPage);
    fileSystemModelPage->setRootPath(m_pagePath.path());
    ui->treeViewFilesPage
            ->setRootIndex(
                fileSystemModelPage->index(
                    fileSystemModelPage->rootPath()));
}
//----------------------------------------
void DialogCreateProductPage::_initGraphicsView()
{
    ui->graphicsView->setScene(&m_scene);
    m_scene.setSceneRect(QRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
    m_pixmapItem = new QGraphicsPixmapItem;
    m_pixmapItem->setPos(0, 0);
    m_scene.addItem(m_pixmapItem);
    m_rectVertical = new ResizableRect(Qt::green);
    int pinWidth = ui->graphicsView->width() / 2;
    m_rectVertical->setRect(QRect(pinWidth/4, 0, pinWidth, ui->graphicsView->height()));
    m_scene.addItem(m_rectVertical);
}
//----------------------------------------
void DialogCreateProductPage::onImageSelectionChanged(
        const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0)
    {
        QString imageFileName = selected.indexes().first().data().toString();
        QString imageFilePath = m_pagePath.filePath(imageFileName);
        QPixmap pixmap(imageFilePath);
        auto width = ui->graphicsView->width()-2;
        auto height = ui->graphicsView->height()-2;
        m_scene.setSceneRect(QRect(0, 0, width, height));
        pixmap = pixmap.scaledToHeight(height);
        m_pixmapItem->setPixmap(pixmap);
        m_pixmapItem->setPos(0, 0);
        int pinWidth = height / 2;
        m_rectVertical->setRect(QRect(height/4, 0, pinWidth, height));
    } else if (selected.size() == 0
               && deselected.size() > 0) {
        m_pixmapItem->setPixmap(QPixmap());
    }
}
//----------------------------------------
void DialogCreateProductPage::_connectSlots()
{
    connect(ui->buttonOpenPageFolder,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::openPageFolder);
    connect(ui->buttonRunAis,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::runAi);
    connect(ui->buttonReplaceName,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::replaceInAiText);
    connect(ui->treeViewFilesPage->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DialogCreateProductPage::onImageSelectionChanged);
    connect(ui->buttonCropAll,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::cropAll);
    connect(ui->buttonCropPinterest,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::cropPinterest);
    connect(ui->buttonCropGoogleAds,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::cropGoogleImageAds);
}
//----------------------------------------
void DialogCreateProductPage::openPageFolder()
{
    QUrl folderUrl = QUrl::fromLocalFile(m_pagePath.path());
    QDesktopServices::openUrl(folderUrl);
}
//----------------------------------------
void DialogCreateProductPage::runAi()
{
    if (m_aiWasRun)
    {
        auto reply = QMessageBox::question(
                    this,
                    "Run AI again?",
                    "Are you sure to want to run AI again?");
        if (reply != QMessageBox::Yes)
        {
            return;
        }
    }
    m_aiWasRun = true;
    _runAiChatGpt();
    _runAiDeepImage();
}
//----------------------------------------
void DialogCreateProductPage::replaceInAiText()
{
}
//----------------------------------------
void DialogCreateProductPage::_runAiDeepImage()
{
    const auto &imageFilePaths = m_pagePath.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    for (const auto &imageFilePath : imageFilePaths)
    {
        const auto &filePath = imageFilePath.absoluteFilePath();
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly))
        {
            QUrl apiUrl("https://deep-image.ai/api/increase/deep-image-x3");
            QNetworkRequest request(apiUrl);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

            // Send the image file to the API for processing
            QNetworkReply *reply = m_networkAccessManager.post(request, file.readAll());
            reply->setProperty("filePath", filePath);
        }
    }
}
//----------------------------------------
void DialogCreateProductPage::_replyDeepAi(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Read the processed image data
        QByteArray imageData = reply->readAll();
        QString filePath = reply->property("filePath").toString();

        // Replace the original file with the processed image
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(imageData);
            file.close();
            qDebug() << "Processed image saved to:" << filePath;
        } else {
            qWarning() << "Failed to write processed image to file:" << filePath;
        }
    } else {
        qWarning() << "Error processing image:" << reply->errorString();
    }

    reply->deleteLater();
}
//----------------------------------------
void DialogCreateProductPage::_runAiChatGpt()
{
    const auto &imageFilePaths = m_pagePath.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    if (imageFilePaths.size() > 0)
    {
        const auto &firstImageFilePath = imageFilePaths[0];
        QUrl url("https://api.openai.com/v1/chat/completions");
        QNetworkRequest request(url);

        request.setRawHeader("Authorization", "Bearer YOUR_API_KEY_HERE");
        QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart jsonPart;
        jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json; charset=UTF-8"));
        QJsonObject json;
        json.insert("model", "Fashion Keywords");  // Assuming this is the identifier for your custom model
        json.insert("prompt", "Please describe the attached image.");
        json.insert("max_tokens", 30);
        jsonPart.setBody(QJsonDocument(json).toJson());
        multiPart->append(jsonPart);

        QHttpPart imagePart;
        imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
        imagePart.setHeader(
                    QNetworkRequest::ContentDispositionHeader,
                    QVariant("form-data; name=\"file\"; filename=\"" + firstImageFilePath.fileName() + "\""));
        QFile* file = new QFile(firstImageFilePath.absoluteFilePath());
        file->open(QIODevice::ReadOnly);
        imagePart.setBodyDevice(file);
        file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
        multiPart->append(imagePart);

        QNetworkReply* reply = m_networkAccessManager.post(request, multiPart);
        multiPart->setParent(reply);
    }
}
//----------------------------------------
void DialogCreateProductPage::_replyChaptGpt4(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response_data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response_data);
        QJsonObject obj = doc.object();
        qDebug() << "ChatGpt4 Response:" << obj;
        ui->textEditChatGpt->setText("OK");
        // Handle the response object here
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
}
//----------------------------------------
void DialogCreateProductPage::cropAll()
{
    const auto &fileInfos = m_pagePath.entryInfoList(
                {"*.jpg"}, QDir::Files, QDir::Name);
    for (const auto &fileInfo : fileInfos)
    {
        const auto &baseName = fileInfo.baseName();
        if (!baseName.contains("GOOGLE") && !baseName.contains("PINTEREST"))
        {
            QStringList elements = baseName.split("-");
            elements.insert(elements.size()-1, "PINTEREST");
            const QString &newFilePath = m_pagePath.filePath(elements.join("-")) + ".jpg";
            QImage image(fileInfo.filePath());
            QRect rect(image.rect());
            int height = rect.height();
            rect.setLeft(height/4);
            rect.setRight(height*3/4);
            image = image.copy(rect);
            image.save(newFilePath);
        }
    }
}
//----------------------------------------
void DialogCreateProductPage::cropPinterest()
{
    const auto &selIndexes = ui->treeViewFilesPage->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        QString imageFileName = selIndexes.first().data().toString();
        QString imageFilePath = m_pagePath.filePath(imageFileName);
        QStringList elements = imageFileName.split("-");
        elements.insert(elements.size()-1, "PINTEREST");
        const QString &newFilePath = m_pagePath.filePath(elements.join("-"));
        QImage image(imageFilePath);
        QRect rect(image.rect());
        int height = rect.height();


        auto widthGraphicsImage = m_pixmapItem->boundingRect().width();
        int xRect = m_rectVertical->boundingRect().left() + m_rectVertical->pos().x() - m_pixmapItem->boundingRect().left();
        double relX = 1. * xRect / widthGraphicsImage;
        int left = relX * height + 0.5;
        rect.setLeft(left);
        rect.setRight(left + height/2);
        image = image.copy(rect);
        image.save(newFilePath);
    }
}
//----------------------------------------
void DialogCreateProductPage::cropGoogleImageAds()
{
}
//----------------------------------------
