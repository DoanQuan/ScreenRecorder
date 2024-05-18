#include "recordscreen.h"
#include <iostream>
#include<QDebug>
#include <QApplication>
#include<QGuiApplication>
#include<QScreen>

RecordScreen::RecordScreen()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

//    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_PaintOnScreen);

    setAttribute(Qt::WA_TransparentForMouseEvents);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
    resize(width, height);
    m_screenSelector = new QRect(QPoint({width/4, height/4}), QPoint({width/4 + width/2, height/4 + height/2}));

}

void RecordScreen::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    // Draw selector
    //    painter.setPen(Qt::NoPen);
    //    painter.setBrush(palette().window());
//    painter.fillRect(m_screenSelector,QBrush(QColor(128, 128, 255, 128)));

    // Highlight border of selector
    // Highlight top side
    QRect topSide;
    topSide.setTopLeft(m_screenSelector->topLeft());
    topSide.setWidth(m_screenSelector->width() + SELECTOR_BORDER_SIZE);
    topSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(topSide, QBrush(QColor("green")));
    qCritical() << "top side: " << topSide.topLeft().rx() << " " << topSide.topLeft().ry();

    // Highlight bottom side
    QRect bottomSide;
//    bottomSide.setTopLeft({m_screenSelector.bottomLeft().rx(),
//                           m_screenSelector.bottomLeft().ry()});
    bottomSide.setTopLeft(m_screenSelector->bottomLeft());
    bottomSide.setWidth(m_screenSelector->width() + SELECTOR_BORDER_SIZE);
    bottomSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(bottomSide, QBrush(QColor("green")));
    qCritical() << "bottomSide: " << bottomSide.topLeft().rx() << " " << bottomSide.topLeft().ry();

    // Highlight left side
    QRect leftSide;
    leftSide.setTopLeft(m_screenSelector->topLeft());

    leftSide.setWidth(SELECTOR_BORDER_SIZE);
    leftSide.setHeight(m_screenSelector->height());
    painter.fillRect(leftSide, QBrush(QColor("green")));
    qCritical() << "leftSide: " << leftSide.topLeft().rx() << " " << leftSide.topLeft().ry();


    // Highlight right side
    QRect rightSide;
    rightSide.setTopLeft(m_screenSelector->topRight());
    rightSide.setWidth(SELECTOR_BORDER_SIZE);
    rightSide.setHeight(m_screenSelector->height());
    painter.fillRect(rightSide, QBrush(QColor("green")));
    qCritical() << "rightSide: " << rightSide.topLeft().rx() << " " << rightSide.topLeft().ry();
    qCritical() << "rightSide: " << this->size().width() << " " << this->size().height();
}
