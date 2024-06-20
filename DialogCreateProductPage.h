#ifndef DIALOGCREATEPRODUCTPAGE_H
#define DIALOGCREATEPRODUCTPAGE_H

#include <QGraphicsScene>
#include <QSettings>
#include <QGraphicsPixmapItem>
#include <QDialog>
#include <QDir>
#include <QWebEngineView>
#include <QNetworkAccessManager>
#include <QItemSelection>
#include <QWebEngineProfile>

class ResizableRect;

namespace Ui {
class DialogCreateProductPage;
}

class DialogCreateProductPage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreateProductPage(
            const QString &pagePath, QWidget *parent = nullptr);
    ~DialogCreateProductPage();
    QString getPermalink() const;
    QString getPageLink() const;

public slots:
    void openPageFolder();
    void openParentPageFolder();

    // Tab 01 deep image
    void copyPageDirPath();
    void removeDeepImagesPostNames();

    // Tab 02 ChatGpt
    void copyFirstImagePath();
    void copyPrompt();
    void copyPromptAmazon();

    // Tab 03 AI
    void runAi();
    void replaceInAiText();
    void recordPageTitle();
    void recordPageDescription();
    void recordPinTitle();
    void recordPinDescription();
    void recordMetaDesc();
    void copyPageLinkFromInfo();
    void copyPinTitle();
    void copyPinDescription();

    // Tab 04 face swap
    void copySelImagePath();
    void browseFaceSmapImagePath();
    void copyFaceSmapImagePath();
    void openFaceSmapImageDir();

    // Tab 05 informations
    void copyLink();
    void addLinkPhotos();
    void addLinkReviews();
    void addLinkPinterest();
    void addLinkCj();
    void removeLink();

    // Tab 06 edit page
    void copySizeClothe();
    void copySizeShoe();
    void fillPage();
    void copyPageDescription();
    void copyPageLink();

    // Tab crop ads
    void cropAll();
    void cropPinterest();
    void cropGoogleImageAds();
    void cropPinterestAddingWhite();

    // Tab pinterest
    void publishPinterest();
    void planifyPinterest();

private slots:
    //void _replyDeepAi(QNetworkReply *reply);
    //void _replyChaptGpt4(QNetworkReply *reply);
    //void _replyPinterest(QNetworkReply *reply);
    void onImageSelectionChanged(
            const QItemSelection &selected,
            const QItemSelection &deselected);
    void _updateRectPin();

private:
    Ui::DialogCreateProductPage *ui;
    QDir m_pagePath;
    QWebEngineView *m_webViewDeepImage;
    QWebEngineView *m_webViewChatGpt;
    QWebEngineView *m_webViewPageCreation;
    QWebEngineView *m_webViewFaceSwap;
    QWebEngineView *m_webViewCj;
    void _initWebViewProfile();
    void _initWebViewDeepImage();
    void _initWebViewChatGpt();
    void _initWebViewPageCreation();
    void _initWebViewFaceSwap();
    void _initWebViewCj();
    void _initFileTrees();
    void _initGraphicsView();
    void _initSizing();
    void _loadSettings();
    void _connectSlots();
    void _runAiChatGpt();
    void _runAiDeepImage();
    QNetworkAccessManager m_networkAccessManager;
    bool m_aiWasRun;
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsScene m_scene;
    ResizableRect *m_rectVertical;
    bool _checkPinterestInfoFiled();
    QStringList _getFilesToPin() const;
    QWebEngineProfile *m_webEngineProfile;
    QSharedPointer<QSettings> settingsPage() const;
    //ResizableRect *m_rectPinterest;
};

#endif // DIALOGCREATEPRODUCTPAGE_H
