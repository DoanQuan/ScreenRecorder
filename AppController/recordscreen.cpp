#include "recordscreen.h"
#include <iostream>
#include<QDebug>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>


ResizePoint::ResizePoint(RecordScreen *parent, int x, int y, ReSizeDirection dirt) : m_recordScreen(parent) {
    setMouseTracking(true);
    off_x = x;
    off_y = y;
    m_dirt = dirt;
    rect.setTopLeft(QPoint({off_x, off_y}));
    rect.setSize(QSize({RESIZE_POINT_SIZE, RESIZE_POINT_SIZE}));
    /*
     * Check with ResizePoint is resizing and draw selector based on that point.
     * How to determine which ResizePoint is being resized? Is ResizePoint the best idea?
     * Resize Record window
     */

    /*
     * A new enum to define location of every ResizePoint
     * Create resizeable points:
     * * Create resize cursor
     *
     * When a point is dragged, notify AreaSelector to paint new window
     * AreaSelecto redefines selectArea based on new ResizePoint.
     */
}

// void ResizePoint::paintEvent(QPaintEvent *event) {
//     // Draw a rectangle
//     QPainter painter(this);
//     painter.fillRect(rect, QBrush(QColor(RESIZE_POINT_COLOR)));
// }

void ResizePoint::mouseMoveEvent(QMouseEvent *event) {
    if(isRectUnderMouse(event->pos())) {
        setCursor(Qt::SplitVCursor);
        std::cout << "set cursor" << std::endl;
    } else {
        unsetCursor();
        std::cout << "underCursor" << std::endl;
    }
}

bool ResizePoint::isRectUnderMouse(QPoint mountPoint) {
    return ((mountPoint.x() >= off_x && mountPoint.x() <= off_x + RESIZE_POINT_SIZE)
            && (mountPoint.y() >= off_y && mountPoint.y() <= off_y + RESIZE_POINT_SIZE));
}


RecordScreen::RecordScreen()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(true);

    // Form the record screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
    resize(width, height);
    m_selectArea = new QRect(QPoint({width/4, height/4}), QSize({ width/2, height/2}));


    // rpTopLeft->show();
}

void RecordScreen::drawScreenSelector() {
    QPainter painter(this);
    QRect topSide;
    topSide.setTopLeft(m_selectArea->topLeft());
    topSide.setWidth(m_selectArea->width());
    topSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(topSide, QBrush(QColor(SELECTOR_DEFAULT_COLOR)));

    QRect bottomSide;
    bottomSide.setTopLeft(m_selectArea->bottomLeft());
    bottomSide.setWidth(m_selectArea->width());
    bottomSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(bottomSide, QBrush(QColor(SELECTOR_DEFAULT_COLOR)));

    QRect leftSide;
    leftSide.setTopLeft(m_selectArea->topLeft());
    leftSide.setWidth(SELECTOR_BORDER_SIZE);
    leftSide.setHeight(m_selectArea->height());
    painter.fillRect(leftSide, QBrush(QColor(SELECTOR_DEFAULT_COLOR)));

    QRect rightSide;
    rightSide.setTopLeft(m_selectArea->topRight());
    rightSide.setWidth(SELECTOR_BORDER_SIZE);
    rightSide.setHeight(m_selectArea->height() + SELECTOR_BORDER_SIZE - 1);
    painter.fillRect(rightSide, QBrush(QColor(SELECTOR_DEFAULT_COLOR)));

    // Draw a ResizePoint and Move that point
    // rpTopLeft->show();

}

void RecordScreen::drawResizePoint() {
    // Create Resize points
    rpTopLeft = new ResizePoint(this,
                                m_selectArea->topLeft().x() + SELECTOR_RESIZEPOINT_PADDING,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING,
                                ReSizeDirection::TOP_LEFT);
    rpTopMiddle = new ResizePoint(this,
                                m_selectArea->topLeft().x() + m_selectArea->width()/2 + SELECTOR_RESIZEPOINT_PADDING/2,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING,
                                ReSizeDirection::TOP_MIDDLE);
    rpTopRight = new ResizePoint(this,
                                m_selectArea->topLeft().x() + m_selectArea->width() - RESIZE_POINT_SIZE - SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING,
                                ReSizeDirection::TOP_RIGHT);
    rpMiddleLeft = new ResizePoint(this,
                                m_selectArea->topLeft().x() + SELECTOR_RESIZEPOINT_PADDING,
                                m_selectArea->topLeft().y() + m_selectArea->height()/2 + SELECTOR_RESIZEPOINT_PADDING/2,
                                ReSizeDirection::MIDDLE_LEFT);
    rpMiddleRight = new ResizePoint(this,
                                rpTopRight->getRect().x(),
                                rpMiddleLeft->getRect().y(),
                                ReSizeDirection::MIDDLE_RIGHT);
    rpBottomLeft = new ResizePoint(this,
                                rpTopLeft->getRect().x(),
                                m_selectArea->topLeft().y() + m_selectArea->height() - RESIZE_POINT_SIZE - SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                ReSizeDirection::BOTTOM_LEFT);
    rpBottomMiddle = new ResizePoint(this,
                                rpTopMiddle->getRect().x(),
                                rpBottomLeft->getRect().y(),
                                ReSizeDirection::BOTTOM_MIDDLE);
    rpBottomRight = new ResizePoint(this,
                                rpTopRight->getRect().x(),
                                rpBottomLeft->getRect().y(),
                                ReSizeDirection::BOTTOM_RIGHT);
    QPainter painter(this);
    painter.fillRect(rpTopLeft->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpTopMiddle->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpTopRight->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpMiddleLeft->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpMiddleRight->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpBottomLeft->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpBottomMiddle->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
    painter.fillRect(rpBottomRight->getRect(), QBrush(QColor(RESIZE_POINT_COLOR)));
}

void RecordScreen::drawMovePoint() {


}

void RecordScreen::setMoveableSelector() {

}

bool RecordScreen::isResizePointUnderMouse(ResizePoint *rsPoint, QPoint mousePoint) {
    int mouseX = mousePoint.x();
    int mouseY = mousePoint.y();
    int rsPointX = rsPoint->getRect().x();
    int rsPointY = rsPoint->getRect().y();
    int size = RESIZE_POINT_SIZE;
    return (mouseX >=  rsPointX && mouseX <= (rsPointX + size) && mouseY >= rsPointY && mouseY <= (rsPointY + RESIZE_POINT_SIZE));
}

bool RecordScreen::isResizePointUnderMouse(QPoint mousePoint) {
    if(isResizePointUnderMouse(rpTopLeft, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpTopMiddle, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpTopRight, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpMiddleLeft, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpMiddleRight, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpBottomLeft, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpBottomMiddle, mousePoint)) {
        return true;
    }

    if(isResizePointUnderMouse(rpBottomRight, mousePoint)) {
        return true;
    }

    return false;
}

ResizePoint* RecordScreen::getUnderMouseResizePoint() {
    auto mousePoint = QCursor::pos();
    if(isResizePointUnderMouse(rpTopLeft, mousePoint)) {
        return rpTopLeft;
    }

    if(isResizePointUnderMouse(rpTopMiddle, mousePoint)) {
        return rpTopMiddle;
    }

    if(isResizePointUnderMouse(rpTopRight, mousePoint)) {
        return rpTopRight;
    }

    if(isResizePointUnderMouse(rpMiddleLeft, mousePoint)) {
        return rpMiddleLeft;
    }

    if(isResizePointUnderMouse(rpMiddleRight, mousePoint)) {
        return rpMiddleRight;
    }

    if(isResizePointUnderMouse(rpBottomLeft, mousePoint)) {
        return rpBottomLeft;
    }

    if(isResizePointUnderMouse(rpBottomMiddle, mousePoint)) {
        return rpBottomMiddle;
    }

    if(isResizePointUnderMouse(rpBottomRight, mousePoint)) {
        return rpBottomRight;
    }

    return nullptr;
}

void RecordScreen::changeCursorToResizeCursor() {
    currentResizePoint = getUnderMouseResizePoint();
    if(currentResizePoint) {
        ReSizeDirection dirt = currentResizePoint->getDirt();
        switch (dirt) {
        case ReSizeDirection::TOP_LEFT:
        case ReSizeDirection::BOTTOM_RIGHT:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case ReSizeDirection::TOP_MIDDLE:
        case ReSizeDirection::BOTTOM_MIDDLE:
            setCursor(Qt::SizeVerCursor);
            break;
        case ReSizeDirection::TOP_RIGHT:
        case ReSizeDirection::BOTTOM_LEFT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case ReSizeDirection::MIDDLE_LEFT:
        case ReSizeDirection::MIDDLE_RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;

        default:
            unsetCursor();
            break;

        }

    } else {
        unsetCursor();
    }
}

void RecordScreen::hanleResizeEvent(QMouseEvent *event) {
    std::cout << "handle resize event" << std::endl;
    if(mouseStatus != MouseStatus::LEFT_BUTTON_PRESS || cursorType != CursorType::RESIZE_CURSOR) {
        // Cannot resize
        std::cout << "Cannot resize" << std::endl;
        return;

    }

    auto dirt = currentResizePoint->getDirt();
    int mousePosX = event->pos().x();
    int mousePosY = event->pos().y();
    switch (dirt) {
    case ReSizeDirection::TOP_LEFT:
        if((mousePosX >= 0 && mousePosX < m_selectArea->bottomRight().x() &&
            mousePosY >= 0 && mousePosY < m_selectArea->bottomRight().y())) {
            m_selectArea->setTopLeft(event->pos());
            std::cout << "Resize to the top_left" << std::endl;
        } else {
            std::cout << "Cannot resize to the top_left" << std::endl;
        }
        break;
    case ReSizeDirection::TOP_MIDDLE:
        if(mousePosY >= 0 && mousePosY < m_selectArea->bottomRight().y()) {
            // int oldHeight = m_selectArea->height();
            m_selectArea->setTopLeft(QPoint({m_selectArea->topLeft().x(), mousePosY}));
        }
        break;
    case ReSizeDirection::TOP_RIGHT:
        if(mousePosX >= 0 && mousePosX > m_selectArea->bottomLeft().x() && mousePosY >= 0 && mousePosY < m_selectArea->bottomLeft().y()) {
            // int oldTopRightX = m_selectArea->topRight().x();
            // int oldTopRightY = m_selectArea->topRight().y();
            // int oldHeight = m_selectArea->height();
            // int oldWidth = m_selectArea->width();
            // m_selectArea->setTopLeft(QPoint({m_selectArea->topLeft().x(), mousePosY}));
            // m_selectArea->setHeight(oldHeight + (mousePosY - oldTopRightY));
            // m_selectArea->setWidth(oldWidth + (mousePosX - oldTopRightX));
            m_selectArea->setTopRight(QPoint({mousePosX, mousePosY}));
        }
        break;
    case ReSizeDirection::MIDDLE_LEFT:
        if(mousePosX >= 0 && mousePosX < m_selectArea->topRight().x()) {
            m_selectArea->setTopLeft(QPoint({mousePosX, m_selectArea->topLeft().y()}));
        }
        break;
    case ReSizeDirection::MIDDLE_RIGHT:
        if(mousePosX > m_selectArea->topLeft().x()) {
            m_selectArea->setTopRight(QPoint({mousePosX, m_selectArea->topLeft().y()}));
        }
        break;
    case ReSizeDirection::BOTTOM_LEFT:
        if(mousePosX < m_selectArea->topRight().x() && mousePosY > m_selectArea->topRight().y()) {
            m_selectArea->setBottomLeft(QPoint({mousePosX, mousePosY}));
        }
        break;
    case ReSizeDirection::BOTTOM_MIDDLE:
        if(mousePosY > m_selectArea->topRight().y()) {
            m_selectArea->setBottomLeft(QPoint({m_selectArea->topLeft().x(), mousePosY}));
        }
        break;
    case ReSizeDirection::BOTTOM_RIGHT:
        if(mousePosX > m_selectArea->topLeft().x() && mousePosY > m_selectArea->topLeft().y()) {
            m_selectArea->setBottomRight(event->pos());
        }
        break;

    }

    update();
}

void RecordScreen::mouseMoveEvent(QMouseEvent *event) {
    if(mouseStatus == MouseStatus::RELEASE) {
        if(isResizePointUnderMouse(event->pos())) {
            changeCursorToResizeCursor();
            cursorType = CursorType::RESIZE_CURSOR;
        } else {
            unsetCursor();
            cursorType = CursorType::ARROW_CURSOR;
        }
    } else if(mouseStatus == MouseStatus::LEFT_BUTTON_PRESS) {
        switch (cursorType) {
        case CursorType::RESIZE_CURSOR:
            hanleResizeEvent(event);
            break;
        default:
            break;
        }
    } else {

    }

}

void RecordScreen::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        mouseStatus = MouseStatus::LEFT_BUTTON_PRESS;
    } else if(event->button() == Qt::RightButton) {
        mouseStatus = MouseStatus::RIGHT_BUTTON_PRESS;
    } else {
        mouseStatus = MouseStatus::RELEASE;
    }
}

void RecordScreen::mouseReleaseEvent(QMouseEvent *event) {
    mouseStatus = MouseStatus::RELEASE;
}

void RecordScreen::paintEvent(QPaintEvent *event) {
    drawScreenSelector();
    drawResizePoint();
}

