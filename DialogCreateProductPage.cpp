#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QClipboard>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonObject>
#include <QJsonDocument>

#include "../common/config/SettingsManager.h"

#include "model/PageInfoList.h"
#include "model/PlannifyListModel.h"
#include "ResizableRect.h"
#include "DialogReplace.h"

#include "DialogCreateProductPage.h"
#include "ui_DialogCreateProductPage.h"

static const QString SETTINGS_API_PINTEREST_ID{"apiPinterestId"};
static const QString SETTINGS_API_PINTEREST_SECRET{"apiPinterestSecret"};
static const QString SETTINGS_API_PINTEREST_ACCESS_TOKEN{"apiPinterestToken"};
static const QString SETTINGS_API_KEY_DEEP_IMAGE{"apiKeyDeepImage"};
static const QString SETTINGS_API_KEY_CHAT_GPT{"apiKeyChatGpt"};
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
    auto model = new PageInfoList{pagePath, ui->tableViewPageInfos};
    ui->tableViewPageInfos->setModel(model);
    ui->listViewPinteresPlanned->setModel(PlannifyListModel::instance());
    _loadSettings();
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
    ui->pageProductPage->layout()->addWidget(m_webView);
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
void DialogCreateProductPage::_loadSettings()
{
    auto settings = SettingsManager::instance()->getSettings();
    QString apiPinterestId = settings->value(SETTINGS_API_PINTEREST_ID, QString{}).toString();
    ui->lineEditPinterestApiId->setText(apiPinterestId);
    QString apiPinterestSecret = settings->value(SETTINGS_API_PINTEREST_SECRET, QString{}).toString();
    ui->lineEditPinterestApiSecret->setText(apiPinterestSecret);
    QString apiPinterestAccessToken = settings->value(SETTINGS_API_PINTEREST_ACCESS_TOKEN, QString{}).toString();
    ui->lineEditPinterestAccessToken->setText(apiPinterestAccessToken);
    QString apiKeyDeepImage = settings->value(SETTINGS_API_KEY_DEEP_IMAGE, QString{}).toString();
    ui->lineEditDeepImageApitKey->setText(apiKeyDeepImage);
    QString apiKeyChatGpt = settings->value(SETTINGS_API_KEY_CHAT_GPT, QString{}).toString();
    ui->lineEditChatGptApiKey->setText(apiKeyChatGpt);
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
    connect(ui->buttonCopyLink,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyLink);
    connect(ui->buttonAddLinkImage,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::addLinkPhotos);
    connect(ui->buttonAddLinkReviews,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::addLinkReviews);
    connect(ui->buttonRemoveLink,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::removeLink);
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
    connect(ui->buttonPinterestPublish,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::publishPinterest);
    connect(ui->buttonPinterestPlan,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::planifyPinterest);
    connect(ui->lineEditDeepImageApitKey,
            &QLineEdit::textEdited,
            this,
            [](const QString &apiKey){
                auto settings = SettingsManager::instance()->getSettings();
                settings->setValue(SETTINGS_API_KEY_DEEP_IMAGE, apiKey);
            });
    connect(ui->lineEditPinterestApiId,
            &QLineEdit::textEdited,
            this,
            [](const QString &value){
                auto settings = SettingsManager::instance()->getSettings();
                settings->setValue(SETTINGS_API_PINTEREST_ID, value);
            });
    connect(ui->lineEditPinterestApiSecret,
            &QLineEdit::textEdited,
            this,
            [](const QString &value){
                auto settings = SettingsManager::instance()->getSettings();
                settings->setValue(SETTINGS_API_PINTEREST_SECRET, value);
            });
    connect(ui->lineEditPinterestAccessToken,
            &QLineEdit::textEdited,
            this,
            [](const QString &value){
                auto settings = SettingsManager::instance()->getSettings();
                settings->setValue(SETTINGS_API_PINTEREST_ACCESS_TOKEN, value);
            });
    connect(ui->lineEditChatGptApiKey,
            &QLineEdit::textEdited,
            this,
            [](const QString &apiKey){
                auto settings = SettingsManager::instance()->getSettings();
                settings->setValue(SETTINGS_API_KEY_CHAT_GPT, apiKey);
            });
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
    //_runAiDeepImage();
}
//----------------------------------------
void DialogCreateProductPage::replaceInAiText()
{
    DialogReplace dialog;
    dialog.exec();
    if (dialog.wasAccepted())
    {
        //TODO
    }
}
//----------------------------------------
void DialogCreateProductPage::copyLink()
{
    auto selIndexes = ui->tableViewPageInfos->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
        auto clipboard = QApplication::clipboard();
        clipboard->setText(model->getLink(selIndexes.first()));
    }
}
//----------------------------------------
void DialogCreateProductPage::addLinkPhotos()
{
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    model->addLinkPhotos();
}
//----------------------------------------
void DialogCreateProductPage::addLinkReviews()
{
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    model->addLinkReviews();
}
//----------------------------------------
void DialogCreateProductPage::removeLink()
{
    auto selIndexes = ui->tableViewPageInfos->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
        model->remove(selIndexes.first());
    }
}
//----------------------------------------
void DialogCreateProductPage::_runAiDeepImage()
{
    const auto &imageFilePaths = m_pagePath.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    const QString &apiKey = ui->lineEditDeepImageApitKey->text();
    //static const QUrl apiUrl("https://api.deep-image.ai/");
    static const QString apiUrl("https://api.deep-image.ai/");
    for (const auto &imageFileInfo : qAsConst(imageFilePaths))
    {
        const auto &imageFilePath = imageFileInfo.absoluteFilePath();
        //QString curlCommand = QString("curl -X POST -F \"api_key=" + apiKey + "\" -F \"image=" + imageFileInfo.fileName() + "\" " + apiUrl);
        QString programCurl{"curl"};

        QStringList arguments;
        arguments << "--request";
        arguments << "POST";
        arguments << "--url";
        arguments << "https://deep-image.ai/rest_api/process_result";
        arguments << "--header";
        arguments << "x-api-key:" + apiKey;
        arguments << "--header";
        arguments << "content-type:application/json";
        arguments << "--data";
        arguments << "{\"height\": \"1920\", \"url\": \"" + imageFileInfo.fileName() + "\"}";
                              /*
        QStringList arguments{"-X"
                              , "api_key=" + apiKey
                              , "-F"
                              , "image=" + imageFileInfo.fileName()
                              , apiUrl};
                                                //*/

        qDebug() << "DEEP AI curl" << arguments.join(" ");
        QProcess process;
        process.setWorkingDirectory(m_pagePath.path());
        process.start(programCurl, arguments);
        process.waitForFinished();

        QByteArray response = process.readAllStandardOutput();
        qDebug() << "DEEP AI reply:" << response;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        QJsonObject jsonObj = jsonDoc.object();
        QString resultImageUrl = jsonObj["url"].toString();

        // Download the result image
        //QString downloadCommand = QString("curl %1 -o %2").arg(resultImageUrl).arg(imageFilePath);
        QStringList arguments2{resultImageUrl, "-o", imageFilePath};

        process.start(programCurl, arguments2);
        process.waitForFinished();

        if (process.exitCode() != 0)
        {
            qWarning() << "Error downloading image from Deep Image API for file" << imageFilePath;
        }
        /*
        QFile* file = new QFile(imageFilePath);
        if (file->open(QIODevice::ReadOnly))
        {

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

            QHttpPart imagePart;
            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
            imagePart.setHeader(
                        QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"image\"; filename=\"" + file->fileName() + "\""));
            imagePart.setBodyDevice(file);
            file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

            QHttpPart apiKeyPart;
            apiKeyPart.setHeader(
                        QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"api_key\""));
            apiKeyPart.setBody(apiKey.toUtf8());

            multiPart->append(apiKeyPart);
            multiPart->append(imagePart);

            QNetworkRequest request(apiUrl);
            QNetworkReply *reply = m_networkAccessManager.post(request, multiPart);
            multiPart->setParent(reply); // Delete the multiPart with the reply

            // Handle reply asynchronously
            connect(reply, &QNetworkReply::finished, this, [this, reply, imageFilePath]()
            {
                if (reply->error() == QNetworkReply::NoError)
                {
                    QByteArray response = reply->readAll();
                    // Parse JSON and download the result image URL
                    QJsonDocument doc = QJsonDocument::fromJson(response);
                    QJsonObject obj = doc.object();
                    QString resultImageUrl = obj["url"].toString();

                    // Download and replace the image file
                    QNetworkReply *downloadReply = m_networkAccessManager.get(
                                QNetworkRequest(QUrl(resultImageUrl)));
                    connect(downloadReply, &QNetworkReply::finished, this, [downloadReply, imageFilePath]()
                    {
                        if (downloadReply->error() == QNetworkReply::NoError)
                        {
                            QFile outFile(imageFilePath);
                            if (outFile.open(QIODevice::WriteOnly))
                            {
                                outFile.write(downloadReply->readAll());
                                outFile.close();
                            }
                        }
                        else
                        {
                            qWarning() << "Deep image API error on retrieving image " << imageFilePath << downloadReply->errorString();;
                        }
                        downloadReply->deleteLater();
                    });
                }
                else
                {
                     qWarning() << "Deep image API error on sending image " << imageFilePath << reply->errorString();;
                }
                reply->deleteLater();
            });
        }
        //*/
    }
}
//----------------------------------------
/*
void DialogCreateProductPage::_replyDeepAi(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Read the processed image data
        QByteArray imageData = reply->readAll();
        QString filePath = reply->property("filePath").toString();

        // Replace the original file with the processed image
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(imageData);
            file.close();
            qDebug() << "Processed image saved to:" << filePath;
        }
        else
        {
            qWarning() << "Failed to write processed image to file:" << filePath;
        }
    } else {
        qWarning() << "Error processing image:" << reply->errorString();
    }

    reply->deleteLater();
}
//*/
//----------------------------------------
void DialogCreateProductPage::_runAiChatGpt()
{
    const auto &imageFilePaths = m_pagePath.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    if (imageFilePaths.size() > 0)
    {
        const auto &firstImageFileName = imageFilePaths[0].fileName();
        QUrl url("https://api.openai.com/v1/chat/completions");
        QString programCurl{"curl"};
        const QString &apiKey = ui->lineEditChatGptApiKey->text();

        QStringList arguments;
        arguments << "https://api.openai.com/v1/chat/completions";
        arguments << "-H";
        arguments << "Content-Type:application/json";
        arguments << "-H";
        arguments << "Authorization: Bearer " + apiKey;
        arguments << "-d";
        /*
        arguments << "'{\"model\":\"gpt-3.5-turbo\","
                     "\"messages\":["
      "{"
        "\"role\": \"system\","
        "\"content\": \"You are a pinterest assistant, skilled in writting pinterest pin description.\""
      "},"
      "{"
        "\"role\": \"user\","
        "\"content\": \"Compose a pinterest description of a black elegant midi dress with sleeves.\""
      "}"
                     "]}'";
                     //*/
        QString jsonPayload = R"({"model":"gpt-4-turbo",
                                 "messages":[
                                    {"role": "system",
                                     "content": "You are a Pinterest assistant, skilled in writing Pinterest pin descriptions including always the most searched keywords."
                                    },
                                    {"role": "user",
                                    "content": [
                                    {
                                        "type": "text",
                                        "text": "For the attached product, provide me a pinterest pin title + description that includes keywords people may use to search such product (without hashtags, and adding as much relevant keywords as possible). Don't make it longer that allowed by pinterest."
                                    },
                                    {
                                        "type": "image_url",
                                        "image_url": {
                                            "url": "%1"
                                        }
                                    }
                                    ]
                                    }
                                 ]})";
        jsonPayload.replace("%1", firstImageFileName);

        arguments << jsonPayload;
        qDebug() << "ChatGpt curl" << arguments.join(" ");
        QProcess process;
        process.setWorkingDirectory(m_pagePath.path());
        process.start(programCurl, arguments);
        process.waitForFinished();

        QByteArray response = process.readAllStandardOutput();
        qDebug() << "ChatGpt reply:" << response;

        /*
        QNetworkRequest request(url);

        const QString &apiKey = ui->lineEditChatGptApiKey->text();
        request.setRawHeader("Content-Type", QString{"application/json"}.toUtf8());
        request.setRawHeader("Authorization", QString{"Bearer " + apiKey}.toUtf8());
        QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart jsonPart;
        jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json; charset=UTF-8"));
        QJsonObject json;
        json.insert("model", "gpt-4-turbo");  // Assuming this is the identifier for your custom model
        //json.insert("model", "asst_TW5WLdGxRgrPQkufQrULDGBX");  // Assuming this is the identifier for your custom model
        json.insert("prompt", "For the attached product, provide me a pinterest pin title + description that includes keywords people may use to search such product (without hashtags, and adding as much relevant keywords as possible). Don't make it longer that allowed by pinterest.");
        json.insert("max_tokens", 8000);
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
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
        {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response_data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(response_data);
                QJsonObject obj = doc.object();
                qDebug() << "ChatGpt4 Response:" << obj;
                ui->textEditChatGpt->setText("OK"); // TODO
            } else {
                qDebug() << "ChatGpt Error:" << reply->errorString();
            }
            reply->deleteLater();
        });
        //*/
    }
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
void DialogCreateProductPage::publishPinterest()
{
    if (_checkPinterestInfoFiled())
    {
        const QStringList &pinFilePaths = _getFilesToPin();
        //const QString &clientId = ui->lineEditPinterestApiId->text();
        //const QString &clientSecret = ui->lineEditPinterestApiSecret->text();
        const QString &accessToken = ui->lineEditPinterestAccessToken->text();

        // Créer un objet QHttpMultiPart pour les données multipart/form-data
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // Ajouter le token d'accès à la requête
        QHttpPart tokenPart;
        tokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"access_token\""));
        tokenPart.setBody(accessToken.toUtf8());
        multiPart->append(tokenPart);

        // Ajouter les fichiers images
        for (const QString &filePath : pinFilePaths) {
            QFile *file = new QFile(filePath);
            if (!file->open(QIODevice::ReadOnly)) {
                qDebug() << "Impossible d'ouvrir le fichier" << filePath;
                delete file;
                continue;
            }

            QHttpPart imagePart;
            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/*"));
            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\"; filename=\"" + file->fileName() + "\""));
            imagePart.setBodyDevice(file);
            multiPart->append(imagePart);
        }

        // Créer une requête POST pour l'API Pinterest
        QUrl url("https://api.pinterest.com/v5/pins/");
        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.post(request, multiPart);
        multiPart->setParent(reply); // le reply détruira multiPart lorsque ce sera terminé

        // Gérer la réponse de Pinterest
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Erreur lors de la publication sur Pinterest :" << reply->errorString();
            } else {
                qDebug() << "Publication réussie sur Pinterest !";
            }
            reply->deleteLater();
        });
    }
}
//----------------------------------------
QStringList DialogCreateProductPage::_getFilesToPin() const
{
    const auto &imageFileInfos = m_pagePath.entryInfoList(
                QStringList{"*PINTEREST*"}, QDir::Files, QDir::Name);
    QStringList pinFilePaths;
    for (const auto &fileInfo : qAsConst(imageFileInfos))
    {
        pinFilePaths << fileInfo.absoluteFilePath();
    }
    while (pinFilePaths.size() > 5)
    {
        pinFilePaths.takeLast();
    }
    return pinFilePaths;
}
//----------------------------------------
bool DialogCreateProductPage::_checkPinterestInfoFiled()
{
    if (ui->lineEditPinterestTitle->text().size() < 5)
    {
        QMessageBox::warning(
                    this,
                    "No Pinterest title",
                    "You need to enter a Pinterest title");
        return false;
    }
    else if (ui->textEditPinterestDescirption->toPlainText().size() < 10)
    {
        QMessageBox::warning(
                    this,
                    "No Pinterest Description",
                    "You need to enter the pinterest description");
        return false;
    }
    else if (ui->lineEditPageUrl->text().size() < 5)
    {
        QMessageBox::warning(
                    this,
                    "No Page Url",
                    "You need to enter the page URL");
        return false;
    }
    else if (_getFilesToPin().size() < 2)
    {
        QMessageBox::warning(
                    this,
                    "No pinning images",
                    "You need to have at least 2 images for Pinterest pinning");
        return false;
    }
    return true;

}

//----------------------------------------
void DialogCreateProductPage::planifyPinterest()
{
    if (_checkPinterestInfoFiled())
    {
        const QStringList &pinFilePaths = _getFilesToPin();
        int nPinsPerDay = ui->spinBoxPinningPerDay->value();
        const QString &permalink = ui->lineEditPageUrl->text();
        const QTime &timePinning = ui->timeEditPinning->time();
        const QDateTime &dateTimePinning
                = PlannifyListModel::instance()->planify(
                    permalink, nPinsPerDay, timePinning);
        const QString &accessToken = ui->lineEditPinterestAccessToken->text();

        // Créer un objet QHttpMultiPart pour les données multipart/form-data
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // Ajouter le token d'accès à la requête
        QHttpPart tokenPart;
        tokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"access_token\""));
        tokenPart.setBody(accessToken.toUtf8());
        multiPart->append(tokenPart);

        // Ajouter les fichiers images
        for (const QString &filePath : pinFilePaths) {
            QFile *file = new QFile(filePath);
            if (!file->open(QIODevice::ReadOnly)) {
                qDebug() << "Impossible d'ouvrir le fichier" << filePath;
                delete file;
                continue;
            }

            QHttpPart imagePart;
            imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/*"));
            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\"; filename=\"" + file->fileName() + "\""));
            imagePart.setBodyDevice(file);
            multiPart->append(imagePart);
        }

        // Convertir la date et l'heure de publication en format UNIX timestamp
        qint64 publishTimestamp = dateTimePinning.toSecsSinceEpoch();

        // Ajouter la date et l'heure de publication à la requête
        QHttpPart publishAtPart;
        publishAtPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"publish_at\""));
        publishAtPart.setBody(QString::number(publishTimestamp).toUtf8());
        multiPart->append(publishAtPart);

        // Créer une requête POST pour l'API Pinterest
        QUrl url("https://api.pinterest.com/v5/pins/");
        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.post(request, multiPart);
        multiPart->setParent(reply); // le reply détruira multiPart lorsque ce sera terminé

        // Gérer la réponse de Pinterest
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Erreur lors de la planification de la publication sur Pinterest :" << reply->errorString();
            } else {
                qDebug() << "Publication planifiée sur Pinterest pour" << dateTimePinning.toString() << "!";
            }
            reply->deleteLater();
        });
    }
}
//----------------------------------------
