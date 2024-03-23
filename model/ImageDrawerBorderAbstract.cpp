#include <QPainter>

#include "ImageDrawerBorderAbstract.h"

//----------------------------------------
ImageDrawerBorderAbstract::ImageDrawerBorderAbstract()
    : ImageDrawerAbstract()
{

}
//----------------------------------------
ImageDrawerBorderAbstract::~ImageDrawerBorderAbstract()
{
}
//----------------------------------------
void ImageDrawerBorderAbstract::draw(QImage &image) const
{
    int imHeight = image.height();
    int imWidth = image.width();
    QPainter painter(&image);
    int widthBorder = qMin(image.width(), image.height()) / 10;
    if (widthBorder % 2 != 0){
        ++widthBorder;
    }
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    for (int y = 0; y < imHeight; ++y) {
        for (int x = 0; x < imWidth; ++x) {
            if (x >= widthBorder && x < imWidth - widthBorder && y >= widthBorder && y < imHeight - widthBorder) {
                continue;
            }
            int distLeft = x;
            int distRight = imWidth-x;
            int distTop = y;
            int distBottom = imHeight - y;
            int distVert = qMin(distTop, distBottom);
            int distHoriz = qMin(distLeft, distRight);
            int distMin = qMin(distVert, distHoriz);
            double alpha = qMax(0., (widthBorder - distMin * 1.) / widthBorder - 0.3);
            QColor pixelColor = color();
            pixelColor.setAlphaF(alpha);
            painter.setPen(QPen(pixelColor, 1));
            painter.drawPoint(x, y);
        }
    }
    painter.end();
}
//----------------------------------------
QString ImageDrawerBorderAbstract::nameFile() const
{
    return colorName().toUpper();
}
//----------------------------------------
QString ImageDrawerBorderAbstract::name() const
{
    return QString("border ") + colorName();
}
//----------------------------------------
