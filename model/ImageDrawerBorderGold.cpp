#include "ImageDrawerBorderGold.h"

//----------------------------------------
ImageDrawerBorderGold::ImageDrawerBorderGold()
    : ImageDrawerBorderAbstract()
{

}
//----------------------------------------
QString ImageDrawerBorderGold::colorName() const
{
    return "gold";
}
//----------------------------------------
QColor ImageDrawerBorderGold::color() const
{
    return QColor(255, 215, 0);
}
//----------------------------------------
