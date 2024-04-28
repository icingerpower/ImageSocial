#ifndef DIALOGCREATEPRODUCTPAGE_H
#define DIALOGCREATEPRODUCTPAGE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDialog>
#include <QDir>
#include <QWebEngineView>
#include <QNetworkAccessManager>
#include <QItemSelection>

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

public slots:
    void openPageFolder();
    void runAi();
    void replaceInAiText();

    // Tab crop ads
    void cropAll();
    void cropPinterest();
    void cropGoogleImageAds();

private slots:
    void _replyDeepAi(QNetworkReply *reply);
    void _replyChaptGpt4(QNetworkReply *reply);
    void onImageSelectionChanged(
            const QItemSelection &selected,
            const QItemSelection &deselected);

private:
    Ui::DialogCreateProductPage *ui;
    QDir m_pagePath;
    QWebEngineView *m_webView;
    void _initWebView();
    void _initFileTrees();
    void _initGraphicsView();
    void _connectSlots();
    void _runAiChatGpt();
    void _runAiDeepImage();
    QNetworkAccessManager m_networkAccessManager;
    bool m_aiWasRun;
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsScene m_scene;
    ResizableRect *m_rectVertical;
    //ResizableRect *m_rectPinterest;
};

#endif // DIALOGCREATEPRODUCTPAGE_H
