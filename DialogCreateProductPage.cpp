#include <QFileDialog>
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
#include <QWebEnginePage>

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
static const QString SETTINGS_KEY_CHATGPT_PROMPT{"chatGptPrompt"};
static const QString SETTINGS_KEY_PAGE_TITLE{"pageTitle"};
static const QString SETTINGS_KEY_PAGE_PERMALINK{"pagePermalink"};
static const QString SETTINGS_KEY_PAGE_DESC{"pageDesc"};
static const QString SETTINGS_KEY_PIN_TITLE{"pinTitle"};
static const QString SETTINGS_KEY_PIN_DESC{"pinDesc"};
static const QString SETTINGS_KEY_META_DESC{"metaDesc"};
static const QString SETTINGS_KEY_FACE_SWAP_SOURCE_PATH{"faceSwapSourceImage"};
//----------------------------------------
DialogCreateProductPage::DialogCreateProductPage(
        const QString &pagePath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCreateProductPage)
{
    ui->setupUi(this);
    m_webViewDeepImage = nullptr;
    m_webViewChatGpt = nullptr;
    m_webViewPageCreation = nullptr;
    m_webViewFaceSwap = nullptr;
    m_webViewCj = nullptr;
    m_pagePath = pagePath;
    _initFileTrees();
    _initWebViewProfile();
    _initWebViewDeepImage();
    _initWebViewChatGpt();
    _initWebViewPageCreation();
    _initWebViewFaceSwap();
    _initWebViewCj();
    _initGraphicsView();
    auto model = new PageInfoList{pagePath, ui->tableViewPageInfos};
    ui->tableViewPageInfos->setModel(model);
    ui->listViewPinteresPlanned->setModel(PlannifyListModel::instance());
    _initSizing();
    _loadSettings();
    _connectSlots();
    m_aiWasRun = false;
}
//----------------------------------------
DialogCreateProductPage::~DialogCreateProductPage()
{
    m_webViewPageCreation->setParent(nullptr);
    delete ui;
}
//----------------------------------------
QString DialogCreateProductPage::getPermalink() const
{
    QString permalink = ui->lineEditPageTitle->text();
    permalink.replace("\"", "");
    permalink.replace("\'", "");
    permalink.replace(" ", "-");
    return permalink.toLower();
}
//----------------------------------------
QString DialogCreateProductPage::getPageLink() const
{
    return "https://pradize.com/product/" + getPermalink();
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewProfile()
{
    //static QWebEngineProfile webEngineProfile{"CustomProfile"};
    m_webEngineProfile = new QWebEngineProfile{"CustomProfile", this};
    auto settings = SettingsManager::instance()->getSettings();
    QString path = settings->fileName();
    path.remove(path.length()-4, 4);
    path += "Cache";
    m_webEngineProfile->setCachePath(path);
    m_webEngineProfile->setHttpCacheMaximumSize(100 * 1024 * 1024); // 100 MB
    m_webEngineProfile->setDownloadPath(m_pagePath.path());
    connect(m_webEngineProfile,
            &QWebEngineProfile::downloadRequested,
            this,
            [](QWebEngineDownloadRequest *downloadRequest){
        downloadRequest->accept();
    });
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewDeepImage()
{
    m_webViewDeepImage = new QWebEngineView(m_webEngineProfile, ui->page01DeepImage);
    ui->page01DeepImage->layout()->addWidget(m_webViewDeepImage);
    QUrl url("https://deep-image.ai/app/application/options/");
    m_webViewDeepImage->load(url);
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewChatGpt()
{
    m_webViewChatGpt = new QWebEngineView(m_webEngineProfile, ui->page02ChatGpt);
    ui->page02ChatGpt->layout()->addWidget(m_webViewChatGpt);
    QUrl url("https://chatgpt.com/g/g-c0kzqHnSq-fashion-keywords");
    m_webViewChatGpt->load(url);
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewPageCreation()
{
    m_webViewPageCreation = new QWebEngineView(m_webEngineProfile, ui->pageProductPage);
    ui->pageProductPage->layout()->addWidget(m_webViewPageCreation);
    QUrl urlAddPage("https://pradize.commercehq.com/admin/products/create");
    m_webViewPageCreation->load(urlAddPage);
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewFaceSwap()
{
    m_webViewFaceSwap = new QWebEngineView(m_webEngineProfile, ui->page04FaceSwap);
    ui->page04FaceSwap->layout()->addWidget(m_webViewFaceSwap);
    QUrl url("https://remaker.ai/face-swap-free");
    m_webViewFaceSwap->load(url);
}
//----------------------------------------
void DialogCreateProductPage::_initWebViewCj()
{
    m_webViewCj = new QWebEngineView(m_webEngineProfile, ui->pageWebCJ);
    ui->pageWebCJ->layout()->addWidget(m_webViewCj);
    QUrl url("https://cjdropshipping.com/mine/sourcing/list");
    m_webViewCj->load(url);
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
    ui->treeViewFilesSource->setHidden(true);

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
    int pinWidth = ui->graphicsView->height() / 2;
    m_rectVertical->setRect(QRect(pinWidth/4, 0, pinWidth, ui->graphicsView->height()));
    m_scene.addItem(m_rectVertical);
}
//----------------------------------------
void DialogCreateProductPage::_updateRectPin()
{
    int width = ui->spinBoxPinWidth->value();
    int height = ui->spinBoxPinHeight->value();
    int pixmapHeight = m_pixmapItem->boundingRect().height();
    int pinWidth = pixmapHeight * width / float(height);
    if (pinWidth % 2 != 0)
    {
        pinWidth += 1;
    }
    if (pixmapHeight % 2 != 0)
    {
        pixmapHeight -= 1;
    }
    m_rectVertical->setRect(QRect(pinWidth/4, 0, pinWidth, pixmapHeight));
}
//----------------------------------------
void DialogCreateProductPage::_initSizing()
{
    QStringList sizesShoe{
        "US-4 | UK/AU-2 | EU-35"
        , "US-5 | UK/AU-3 | EU-36"
        , "US-6 | UK/AU-4 | EU-37"
        , "US-7 | UK/AU-5 | EU-38"
        , "US-8 | UK/AU-6 | EU-39"
        , "US-8.5 | UK/AU-6.5 | EU-40"
        , "US-9 | UK/AU-7 | EU-41"
        , "US-10 | UK/AU-8 | EU-42"
        , "US-10.5 | UK/AU-8.5 | EU-43"
        , "US-11 | UK/AU-9 | EU-44"
        , "US-12 | UK/AU-10 | EU-45"
        , "US-13 | UK/AU-11 | EU-46"
    };
    ui->comboBoxShoeSizeFrom->addItems(sizesShoe);
    ui->comboBoxShoeSizeTo->addItems(sizesShoe);
    ui->comboBoxShoeSizeTo->setCurrentIndex(8);
    QStringList sizesClothe{
        "US-2 | UK/AU-6 | EU/DE-32 | FR/ES-34"
        , "US-4 | UK/AU-8 | EU/DE-34 | FR/ES-36"
        , "US-6 | UK/AU-10 | EU/DE-36 | FR/ES-38"
        , "US-8 | UK/AU-12 | EU/DE-38 | FR/ES-40"
        , "US-10 | UK/AU-14 | EU/DE-40 | FR/ES-42"
        , "US-12 | UK/AU-16 | EU/DE-42 | FR/ES-44"
        , "US-14 | UK/AU-18 | EU/DE-44 | FR/ES-46"
    };
    ui->comboBoxClotheSizeFrom->addItems(sizesClothe);
    ui->comboBoxClotheSizeTo->addItems(sizesClothe);
    ui->comboBoxClotheSizeTo->setCurrentIndex(3);
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
    auto settingsLocal = settingsPage();
    QString chatGptReply = settingsLocal->value(SETTINGS_KEY_CHATGPT_PROMPT, QString{}).toString();
    ui->textEditChatGpt->setPlainText(chatGptReply);
    QString pageTitle = settingsLocal->value(SETTINGS_KEY_PAGE_TITLE, QString{}).toString();
    ui->lineEditPageTitle->setText(pageTitle);
    QString permalink = settingsLocal->value(SETTINGS_KEY_PAGE_PERMALINK, QString{}).toString();
    ui->lineEditPermalink->setText(permalink);
    QString pageDesc = settingsLocal->value(SETTINGS_KEY_PAGE_DESC, QString{}).toString();
    ui->textEditPageDesc->setPlainText(pageDesc);
    QString pinTitle = settingsLocal->value(SETTINGS_KEY_PIN_TITLE, QString{}).toString();
    ui->lineEditPinTitle->setText(pinTitle);
    QString pinDesc = settingsLocal->value(SETTINGS_KEY_PIN_DESC, QString{}).toString();
    ui->textEditPinDesc->setPlainText(pinDesc);
    QString metaDesc = settingsLocal->value(SETTINGS_KEY_META_DESC, QString{}).toString();
    ui->lineEditPageMetaDesc->setText(metaDesc);
    QString faceSwapSource = settings->value(SETTINGS_KEY_FACE_SWAP_SOURCE_PATH, QString{}).toString();
    ui->lineEditFaceSwapSource->setText(faceSwapSource);
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
        auto pixmapPreview = pixmap.scaledToHeight(200);
        ui->labelPageImage->setPixmap(pixmapPreview);

        auto width = ui->graphicsView->width()-2;
        auto height = ui->graphicsView->height()-2;
        m_scene.setSceneRect(QRect(0, 0, width, height));
        pixmap = pixmap.scaledToHeight(height);
        m_pixmapItem->setPixmap(pixmap);
        m_pixmapItem->setPos(0, 0);
        _updateRectPin();
    } else if (selected.size() == 0
               && deselected.size() > 0) {
        m_pixmapItem->setPixmap(QPixmap());
    }
}
//----------------------------------------
void DialogCreateProductPage::_connectSlots()
{
    connect(ui->buttonOpenParentFolder,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::openParentPageFolder);
    connect(ui->buttonOpenPageFolder,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::openPageFolder);

    // Tab 01 deep image
    connect(ui->buttonCopyPathPagePath,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPageDirPath);
    connect(ui->buttonRemoveDeepImagePostNames,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::removeDeepImagesPostNames);

    // Tab 02 ChatGpt
    connect(ui->buttonCopyFirstImagePath_2,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyFirstImagePath);
    connect(ui->buttonCopyFirstImagePath,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyFirstImagePath);
    connect(ui->buttonCopyPrompt,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPrompt);
    connect(ui->buttonCopyPromptAmazon,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPromptAmazon);

    // Tab 03 ai
    connect(ui->buttonRunAis,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::runAi);
    connect(ui->buttonReplaceName,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::replaceInAiText);
    connect(ui->buttonRecordPageTitle,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::recordPageTitle);
    connect(ui->buttonRecordPageDesc,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::recordPageDescription);
    connect(ui->buttonRecordPinterestTitle,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::recordPinTitle);
    connect(ui->buttonRecordPinterestDescription,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::recordPinDescription);
    connect(ui->buttonRecordMetaDesc,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::recordMetaDesc);
    connect(ui->buttonCopyPermalink,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPageLinkFromInfo);
    connect(ui->buttonCopyPinTitle,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPinTitle);
    connect(ui->buttonCopyPinDescription,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPinDescription);
    connect(ui->lineEditPageTitle,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        settingsPage()->setValue(SETTINGS_KEY_PAGE_TITLE, text);
    });
    connect(ui->lineEditPermalink,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        settingsPage()->setValue(SETTINGS_KEY_PAGE_PERMALINK, text);
    });
    connect(ui->textEditChatGpt,
            &QTextEdit::textChanged,
            this,
            [this](){
        settingsPage()->setValue(SETTINGS_KEY_CHATGPT_PROMPT, ui->textEditChatGpt->toPlainText());
    });
    connect(ui->textEditPageDesc,
            &QTextEdit::textChanged,
            this,
            [this](){
        settingsPage()->setValue(SETTINGS_KEY_PAGE_DESC, ui->textEditPageDesc->toPlainText());
    });
    connect(ui->lineEditPinTitle,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        settingsPage()->setValue(SETTINGS_KEY_PIN_TITLE, text);
    });
    connect(ui->textEditPinDesc,
            &QTextEdit::textChanged,
            this,
            [this](){
        settingsPage()->setValue(SETTINGS_KEY_PIN_DESC, ui->textEditPinDesc->toPlainText());
    });
    connect(ui->lineEditPageMetaDesc,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        settingsPage()->setValue(SETTINGS_KEY_META_DESC, text);
    });
    connect(ui->lineEditFaceSwapSource,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        auto settings = SettingsManager::instance()->getSettings();
        settings->setValue(SETTINGS_KEY_FACE_SWAP_SOURCE_PATH, text);
    });

    // Tab 04 face swap
    connect(ui->buttonCopyPageImagePath,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copySelImagePath);
    connect(ui->buttonBRowseFaceSwapSource,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::browseFaceSmapImagePath);
    connect(ui->buttonCopyFaceSwapSource,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyFaceSmapImagePath);
    connect(ui->buttonOpenFaceSwapSource,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::openFaceSmapImageDir);

    // Tab 05 Link Informations
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
    connect(ui->buttonAddLinkPinterest,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::addLinkPinterest);
    connect(ui->buttonRemoveLink,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::removeLink);

    // Tab 06 edit page
    connect(ui->buttonCopySizeClothe,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copySizeClothe);
    connect(ui->buttonCopySizeShoe,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copySizeShoe);
    connect(ui->buttonFillPage,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::fillPage);
    connect(ui->buttonCopyPageDescription,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPageDescription);
    connect(ui->buttonCopyPageLink,
            &QPushButton::clicked,
            this,
            &DialogCreateProductPage::copyPageLink);

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
    connect(ui->spinBoxPinWidth,
            &QSpinBox::valueChanged,
            this,
            &DialogCreateProductPage::_updateRectPin);
    connect(ui->spinBoxPinHeight,
            &QSpinBox::valueChanged,
            this,
            &DialogCreateProductPage::_updateRectPin);
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
void DialogCreateProductPage::openParentPageFolder()
{
    QDir parentDir = m_pagePath;
    parentDir.cd("..");
    QUrl folderUrl = QUrl::fromLocalFile(parentDir.path());
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
        QString text = ui->textEditChatGpt->toPlainText();
        text.replace(dialog.getBeforeText(), dialog.getAfterText());
        ui->textEditChatGpt->setPlainText(text);
        //auto settings = SettingsManager::instance()->getSettings();
        //settings->setValue(SETTINGS_KEY_CHATGPT_PROMPT, text);
    }
}
//----------------------------------------
void DialogCreateProductPage::recordPageTitle()
{
    QString text = ui->textEditChatGpt->textCursor().selectedText();
    ui->lineEditPageTitle->setText(text);
    settingsPage()->setValue(SETTINGS_KEY_PAGE_TITLE, text);
    const QString &permalink = getPermalink();
    ui->lineEditPermalink->setText(permalink);
    settingsPage()->setValue(SETTINGS_KEY_PAGE_PERMALINK, permalink);
}
//----------------------------------------
void DialogCreateProductPage::recordPageDescription()
{
    QString text = ui->textEditChatGpt->textCursor().selectedText();
    ui->textEditPageDesc->setPlainText(text);
}
//----------------------------------------
void DialogCreateProductPage::recordPinTitle()
{
    QString text = ui->textEditChatGpt->textCursor().selectedText();
    ui->lineEditPinTitle->setText(text);
    settingsPage()->setValue(SETTINGS_KEY_PIN_TITLE, text);
}
//----------------------------------------
void DialogCreateProductPage::recordPinDescription()
{
    QString text = ui->textEditChatGpt->textCursor().selectedText();
    ui->textEditPinDesc->setPlainText(text);
    //auto settings = SettingsManager::instance()->getSettings();
    //settings->setValue(SETTINGS_KEY_PIN_DESC, text);
}
//----------------------------------------
void DialogCreateProductPage::recordMetaDesc()
{
    QString text = ui->textEditChatGpt->textCursor().selectedText();
    ui->lineEditPageMetaDesc->setText(text);
    settingsPage()->setValue(SETTINGS_KEY_META_DESC, text);
}
//----------------------------------------
void DialogCreateProductPage::copyPageLinkFromInfo()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText("https://pradize.com/product/" + ui->lineEditPermalink->text());
}
//----------------------------------------
void DialogCreateProductPage::copyPinTitle()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->lineEditPinTitle->text());
}
//----------------------------------------
void DialogCreateProductPage::copyPinDescription()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->textEditPinDesc->toPlainText());
}
//----------------------------------------
void DialogCreateProductPage::copyPageDirPath()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(m_pagePath.path());
}
//----------------------------------------
void DialogCreateProductPage::removeDeepImagesPostNames()
{
    const QStringList addedMarks{"-height="};
    for (const auto &addedMark : addedMarks)
    {
        const auto &imageFileInfos = m_pagePath.entryInfoList(
                    QStringList{"*" + addedMark + "*.jpg"}, QDir::Files, QDir::Name);
        for (const auto &imageFileInfo : imageFileInfos)
        {
            QStringList elements = imageFileInfo.absoluteFilePath().split(addedMark);
            elements.takeLast();
            const QString &newFilePath = elements.join(addedMark) + ".jpg";
            const QString &oldFilePath = imageFileInfo.absoluteFilePath();
            QFile::copy(oldFilePath, newFilePath);
            QFile::remove(oldFilePath);
        }
    }
}
//----------------------------------------
void DialogCreateProductPage::copyFirstImagePath()
{
    auto clipboard = QApplication::clipboard();
    const auto &imageFileInfos = m_pagePath.entryInfoList(
                QStringList{"*.jpg"}, QDir::Files, QDir::Name);
    if (imageFileInfos.size() > 0)
    {
        clipboard->setText(imageFileInfos[0].absoluteFilePath());
    }
    else
    {
        clipboard->setText(QString{});
    }
}
//----------------------------------------
void DialogCreateProductPage::copyPrompt()
{
    auto clipboard = QApplication::clipboard();
    QString prompt = R"(
Please provide a product description for this product that you can see on the image.
- The product description should have at least 200 words.
- The product description should use simple phrases that are easy to translate.
- Use word that raise confidence and perceived value.
- The first paragraph should connect with the reader and mention the occasion that the clothe can be used.
- In the description, please prevent most objections the buyer could have (except regarding shipping time as we don’t ship fast). Among the sale argument, we offer free return and free size exchange.
- We only sell the clothe so don’t make the buyer believe he will also receive the accessories
2) Once that you are done with description, please suggest 5 unique product names, with a woman name.
3) Then suggest a short google meta description that says that we ship world wide and that the first article is satisfied or refunded without return needed
4) Then suggest a 3 pinterest pin titles + 1 description that includes keywords people may use to search such product (as you have been trained, without hashtags, and adding as much relevant keywords as possible).
)";
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    const QString &extraInfos = model->getInfoExtra();
    if (!extraInfos.isEmpty())
    {
        QStringList promptElements = prompt.split(".");
        promptElements[0] += " (";
        promptElements[0] += extraInfos;
        promptElements[0] += ")";
        prompt = promptElements.join(".");
    }
    clipboard->setText(prompt);
}
//----------------------------------------
void DialogCreateProductPage::copyPromptAmazon()
{
    auto clipboard = QApplication::clipboard();
    QString prompt = R"(
For this product that you can see on the image, please provide different kind of titles and descriptions as instructed below.
- Use simple phrases that are easy to translate.
- Use word that raise confidence and perceived value.
- Always use as much keywords people may use to search such product (using your training document)

1) Suggest 5 product titles that start with a woman name and then optimized with keywords
2) Suggest a description of at least 200 words
- The first paragraph should connect with the reader and mention the occasion that the clothe can be used.
- In the description, please prevent most objections the buyer could have (except regarding shipping time as we don’t ship fast). Among the sale argument, we offer free return and free size exchange.
- The last phrase should be a call-to-action that invite to buy
3) Then suggest a short google meta description that says that we ship world wide and that the first article is satisfied or refunded without return needed
4) Then suggest a 3 pinterest pin titles + 1 description that includes keywords people may use to search such product (without hashtags, and adding as much relevant keywords as possible).
5) Then write an amazon product page description of at least 10 words that focus in giving product details and in preventing most objections the buyer could have.
6) Then write 5 amazon bullets points that focus in giving product details and in preventing most objections the buyer could have.
)";
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    const QString &extraInfos = model->getInfoExtra();
    if (!extraInfos.isEmpty())
    {
        QStringList promptElements = prompt.split(",");
        promptElements[0] += " (";
        promptElements[0] += extraInfos;
        promptElements[0] += ")";
        prompt = promptElements.join(",");
    }
    clipboard->setText(prompt);

}
//----------------------------------------
void DialogCreateProductPage::copySelImagePath()
{
    auto clipboard = QApplication::clipboard();
    const auto &selIndexes = ui->treeViewFilesPage->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        QString fileName = selIndexes.first().data().toString();
        const QString &filePath = m_pagePath.absoluteFilePath(fileName);
        clipboard->setText(filePath);
    }
    else
    {
        clipboard->setText(QString{});
    }
}
//----------------------------------------
void DialogCreateProductPage::browseFaceSmapImagePath()
{
    // TODO
    const auto &filePath = QFileDialog::getOpenFileName(
                this,
                "Settings file",
                QString{},
                QString{"Images (*.png *.xpm *.jpg*.jpeg *.PNG *.JPG *.gif)"});
    if (!filePath.isEmpty())
    {
        ui->lineEditFaceSwapSource->setText(filePath);
        auto settings = SettingsManager::instance()->getSettings();
        settings->setValue(SETTINGS_KEY_FACE_SWAP_SOURCE_PATH, filePath);
    }
}
//----------------------------------------
void DialogCreateProductPage::copyFaceSmapImagePath()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->lineEditFaceSwapSource->text());
}
//----------------------------------------
void DialogCreateProductPage::openFaceSmapImageDir()
{
    const auto &filePath = ui->lineEditFaceSwapSource->text();
    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        QUrl folderUrl = QUrl::fromLocalFile(fileInfo.dir().path());
        QDesktopServices::openUrl(folderUrl);
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
void DialogCreateProductPage::addLinkPinterest()
{
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    model->addLinkPin();
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
void DialogCreateProductPage::copySizeClothe()
{
    int start = ui->comboBoxClotheSizeFrom->currentIndex();
    int end = ui->comboBoxClotheSizeTo->currentIndex();
    auto clipboard = QApplication::clipboard();
    if (start < end)
    {
        QStringList sizes;
        for (int i=start; i<=end; ++i)
        {
            sizes << ui->comboBoxClotheSizeFrom->itemText(i);
        }
        clipboard->setText(sizes.join(","));
    }
    else
    {
        clipboard->setText(QString{});
    }
}
//----------------------------------------
void DialogCreateProductPage::copySizeShoe()
{
    int start = ui->comboBoxShoeSizeFrom->currentIndex();
    int end = ui->comboBoxShoeSizeTo->currentIndex();
    auto clipboard = QApplication::clipboard();
    if (start < end)
    {
        QStringList sizes;
        for (int i=start; i<=end; ++i)
        {
            sizes << ui->comboBoxShoeSizeFrom->itemText(i);
        }
        clipboard->setText(sizes.join(","));
    }
    else
    {
        clipboard->setText(QString{});
    }
}
//----------------------------------------
void DialogCreateProductPage::fillPage()
{
    QString jsMultiVariant = R"(document.querySelector("#chq-app > div.ng-scope > section > div > div > div > div.page-content > form > div:nth-child(1) > div > div.form-group.v-table.big-radio > div > label.btn.btn-primary.v-cell.toggle-multi > span").click();)";
    m_webViewPageCreation->page()->runJavaScript(jsMultiVariant);

    QString jsCodeTitle = R"(document.querySelector("#chq-app > div.ng-scope > section > div > div > div > div.page-content > form > div:nth-child(1) > div > div:nth-child(3) > input").value = "%1";)";
    jsCodeTitle.replace("%1", ui->lineEditPageTitle->text());
    m_webViewPageCreation->page()->runJavaScript(jsCodeTitle);

    QString jsCodeMetaTitle = R"(document.querySelector("#seo_title").value = "%1";)";
    jsCodeMetaTitle.replace("%1", ui->lineEditPageTitle->text());
    m_webViewPageCreation->page()->runJavaScript(jsCodeMetaTitle);

    QString jsCodeMetaDesc = R"(document.querySelector("#seo_meta").value = "%1";)";
    jsCodeMetaDesc.replace("%1", ui->lineEditPageMetaDesc->text());
    m_webViewPageCreation->page()->runJavaScript(jsCodeMetaDesc);

    QString jsCodePermalink = R"(document.querySelector("#seo_url").value = "%1";)";
    const QString &permalink = getPermalink();
    jsCodePermalink.replace("%1", permalink);
    m_webViewPageCreation->page()->runJavaScript(jsCodePermalink);
    ui->lineEditPageUrl->setText(getPageLink());
        /*
    m_webViewPageCreation->page()->runJavaScript(jsCodeTitle, 0, [jsCodeTitle](const QVariant &result){
        qDebug() << "Running javascript: " << jsCodeTitle;
        qDebug() << "Running result: " << result;
    });
    //*/
}
//----------------------------------------
void DialogCreateProductPage::copyPageDescription()
{
    const QString &text = ui->textEditPageDesc->toPlainText();
    auto clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//----------------------------------------
void DialogCreateProductPage::copyPageLink()
{
    auto clipboard = QApplication::clipboard();
    const QString &pageLink = getPageLink();
    clipboard->setText(pageLink);
    auto model = static_cast<PageInfoList *>(ui->tableViewPageInfos->model());
    model->setPageLink(pageLink);
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
        //gpt-4-turbo
        QString jsonPayload = R"({"model":"asst_TW5WLdGxRgrPQkufQrULDGBX",
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
        const auto &boundingRect = m_rectVertical->boundingRect();
        double ratio = boundingRect.width() / boundingRect.height();
        int xRect = boundingRect.left() + m_rectVertical->pos().x() - m_pixmapItem->boundingRect().left();
        double relX = 1. * xRect / widthGraphicsImage;
        int left = relX * height + 0.5;
        rect.setLeft(left);
        rect.setRight(left + height * ratio);
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
        QStringList curlArgs;
        curlArgs << "-X" << "POST"
                 << "-H" << "Content-Type: multipart/form-data"
                 << "-F" << "access_token=" + accessToken;

        // Ajouter les fichiers images à la commande curl
        for (const QString &filePath : pinFilePaths) {
            curlArgs << "-F" << "image=@" + filePath;
        }

        // Ajouter l'URL de l'API Pinterest à la commande curl
        curlArgs << "https://api.pinterest.com/v5/pins/";

        // Créer un processus pour exécuter la commande curl
        QProcess *curlProcess = new QProcess();
        curlProcess->setProgram("curl");
        curlProcess->setArguments(curlArgs);

        // Démarrer le processus curl
        curlProcess->start();

        // Attendre que le processus se termine
        if (!curlProcess->waitForFinished()) {
            qDebug() << "Erreur : Curl n'a pas pu se terminer correctement.";
            return;
        }

        // Lire la sortie du processus curl
        QByteArray output = curlProcess->readAllStandardOutput();
        qDebug() << "Réponse de curl :" << output;

        // Gérer la réponse de curl
        if (curlProcess->exitCode() != 0) {
            qDebug() << "Erreur lors de la planification de la publication sur Pinterest :" << curlProcess->errorString();
        } else {
            qDebug() << "Pinterest pinning done";
        }

        // Nettoyer
        curlProcess->deleteLater();

        /*
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
        //*/
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

QSharedPointer<QSettings> DialogCreateProductPage::settingsPage() const
{
    const QString &settingsPath = m_pagePath.absoluteFilePath(PageInfoList::SETTING_FILE_NAME);
    return QSharedPointer<QSettings>(new QSettings(settingsPath, QSettings::IniFormat));
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

        // Convertir la date et l'heure de publication en format UNIX timestamp
        qint64 publishTimestamp = dateTimePinning.toSecsSinceEpoch();

        // Construire la commande curl
        QStringList curlArgs;
        curlArgs << "-X" << "POST"
                 << "-H" << "Content-Type: multipart/form-data"
                 << "-F" << "access_token=" + accessToken;

        // Ajouter les fichiers images à la commande curl
        for (const QString &filePath : pinFilePaths) {
            curlArgs << "-F" << "image=@" + filePath;
        }

        // Ajouter la date et l'heure de publication à la commande curl
        curlArgs << "-F" << "publish_at=" + QString::number(publishTimestamp);

        // Ajouter l'URL de l'API Pinterest à la commande curl
        curlArgs << "https://api.pinterest.com/v5/pins/";

        // Créer un processus pour exécuter la commande curl
        QProcess *curlProcess = new QProcess();
        curlProcess->setProgram("curl");
        curlProcess->setArguments(curlArgs);

        // Démarrer le processus curl
        curlProcess->start();

        // Attendre que le processus se termine
        if (!curlProcess->waitForFinished()) {
            qDebug() << "Erreur : Curl n'a pas pu se terminer correctement.";
            return;
        }

        // Lire la sortie du processus curl
        QByteArray output = curlProcess->readAllStandardOutput();
        qDebug() << "Réponse de curl :" << output;

        // Gérer la réponse de curl
        if (curlProcess->exitCode() != 0) {
            qDebug() << "Erreur lors de la planification de la publication sur Pinterest :" << curlProcess->errorString();
        } else {
            qDebug() << "Publication planifiée sur Pinterest pour" << dateTimePinning.toString() << "!";
        }

        // Nettoyer
        curlProcess->deleteLater();

        /*
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
        //*/
    }
}
//----------------------------------------
