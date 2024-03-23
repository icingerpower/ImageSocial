#include "ImageDrawerBorderGold.h"
#include "ImageDrawerBorderBlack.h"
#include "ImageDrawerBorderRed.h"

#include "ImageDrawerAbstract.h"

//----------------------------------------
QList<const ImageDrawerAbstract *> ImageDrawerAbstract::allImageDrawers()
{
    static ImageDrawerBorderGold gold;
    static ImageDrawerBorderBlack black;
    static ImageDrawerBorderRed red;
    static QList<const ImageDrawerAbstract *> drawers
            = {
        &gold
        , &black
        , &red
    };
    return drawers;
}
//----------------------------------------
QStringList ImageDrawerAbstract::allImageDrawerNames()
{
    QStringList names;
    auto _drawers = allImageDrawers();
    for (auto it=_drawers.begin();
         it != _drawers.end(); ++it) {
        names << (*it)->name();
    }
    return names;
}
//----------------------------------------
const ImageDrawerAbstract *ImageDrawerAbstract::getDrawer(
        const QString &name)
{
    auto _drawers = allImageDrawers();
    for (auto it=_drawers.begin();
         it != _drawers.end(); ++it) {
        if ((*it)->name() == name) {
            return *it;
        }
    }
    return nullptr;
}
//----------------------------------------
ImageDrawerAbstract::ImageDrawerAbstract()
{
}
//----------------------------------------
ImageDrawerAbstract::~ImageDrawerAbstract()
{
}
//----------------------------------------
