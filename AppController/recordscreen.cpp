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
}

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
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(true);

    // Form the record screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
    screenHeight = height;
    screenWidth = width;
    resize(width, height);
    m_selectArea = new QRect(QPoint({width/4, height/4}), QSize({ width/2, height/2}));
    isRecording = false;

    selectorColor = QColor(SELECTOR_DEFAULT_COLOR);
}

void RecordScreen::toNormalState() {
    isRecording = false;
    selectorColor = QColor(SELECTOR_DEFAULT_COLOR);
    repaint();
}

void RecordScreen::toRecordState() {
    isRecording = true;
    selectorColor = QColor(SELECTOR_RECORD_COLOR);
    repaint();
}

void RecordScreen::drawScreenSelector() {
    QPainter painter(this);
    QRect topSide;
    topSide.setTopLeft(m_selectArea->topLeft());
    topSide.setWidth(m_selectArea->width() - 1);
    topSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(topSide, QBrush(QColor(selectorColor)));

    QRect bottomSide;
    bottomSide.setTopLeft(m_selectArea->bottomLeft()+QPoint(0, -SELECTOR_BORDER_SIZE ));
    bottomSide.setWidth(m_selectArea->width()- 1);
    bottomSide.setHeight(SELECTOR_BORDER_SIZE);
    painter.fillRect(bottomSide, QBrush(QColor(selectorColor)));

    QRect leftSide;
    leftSide.setTopLeft(m_selectArea->topLeft());
    leftSide.setWidth(SELECTOR_BORDER_SIZE);
    leftSide.setHeight(m_selectArea->height() - 1);
    painter.fillRect(leftSide, QBrush(QColor(selectorColor)));

    QRect rightSide;
    rightSide.setTopLeft(m_selectArea->topRight() + QPoint(-SELECTOR_BORDER_SIZE, 0));
    rightSide.setWidth(SELECTOR_BORDER_SIZE);
    rightSide.setHeight(m_selectArea->height() - 1);
    painter.fillRect(rightSide, QBrush(QColor(selectorColor)));
}

void RecordScreen::drawResizePoint() {
    // Create Resize points
    rpTopLeft = new ResizePoint(this,
                                m_selectArea->topLeft().x() + SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                ReSizeDirection::TOP_LEFT);
    rpTopMiddle = new ResizePoint(this,
                                m_selectArea->topLeft().x() +  m_selectArea->width()/2 - RESIZE_POINT_SIZE/2,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                ReSizeDirection::TOP_MIDDLE);
    rpTopRight = new ResizePoint(this,
                                m_selectArea->topRight().x() - SELECTOR_BORDER_SIZE - SELECTOR_RESIZEPOINT_PADDING - RESIZE_POINT_SIZE,
                                m_selectArea->topLeft().y() + SELECTOR_RESIZEPOINT_PADDING + SELECTOR_BORDER_SIZE,
                                ReSizeDirection::TOP_RIGHT);
    rpMiddleLeft = new ResizePoint(this,
                                rpTopLeft->getRect().x(),
                                m_selectArea->topLeft().y() + m_selectArea->height()/2 - RESIZE_POINT_SIZE/2,
                                ReSizeDirection::MIDDLE_LEFT);
    rpMiddleRight = new ResizePoint(this,
                                rpTopRight->getRect().x(),
                                rpMiddleLeft->getRect().y(),
                                ReSizeDirection::MIDDLE_RIGHT);
    rpBottomLeft = new ResizePoint(this,
                                rpTopLeft->getRect().x(),
                                m_selectArea->bottomLeft().y() - SELECTOR_BORDER_SIZE - SELECTOR_RESIZEPOINT_PADDING - RESIZE_POINT_SIZE,
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
    // Draw a cross
    QPoint middlePoint;
    middlePoint.setX(m_selectArea->topLeft().x() + m_selectArea->width()/2);
    middlePoint.setY(m_selectArea->topLeft().y() + m_selectArea->height()/2);

    // Create move cursor area
    moveCursorArea.setTopLeft(QPoint(middlePoint.x() - MOVE_AREA_WIDTH/2, middlePoint.y() - MOVE_AREA_HEIGHT/2));
    moveCursorArea.setHeight(MOVE_AREA_HEIGHT);
    moveCursorArea.setWidth(MOVE_AREA_WIDTH);

    // Draw vertical rectangle
    QRect vRect;
    vRect.setTopLeft(QPoint(middlePoint.x() - MOVE_CURSOR_SIZE/2, moveCursorArea.y()));
    vRect.setWidth(MOVE_CURSOR_SIZE);
    vRect.setHeight(MOVE_AREA_HEIGHT);
    // Draw horizontal rectangle
    QRect yRect;
    yRect.setTopLeft(QPoint(moveCursorArea.x(), middlePoint.y() - MOVE_CURSOR_SIZE/2));
    yRect.setHeight(MOVE_CURSOR_SIZE);
    yRect.setWidth(MOVE_AREA_HEIGHT);    QPainter painter(this);
    painter.fillRect(vRect, QBrush(QColor(MOVE_POINT_COLOR)));
    painter.fillRect(yRect, QBrush(QColor(MOVE_POINT_COLOR)));

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

bool RecordScreen::isMoveAreaUnderMousePoint(QPoint point) {
    return moveCursorArea.contains(point);
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
    cursorType = CursorType::RESIZE_CURSOR;
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
            cursorType = CursorType::ARROW_CURSOR;
            break;
        }

    } else {
        unsetCursor();
        cursorType = CursorType::ARROW_CURSOR;
    }
}

void RecordScreen::hanleResizeEvent(QMouseEvent *event) {
    if(mouseStatus != MouseStatus::LEFT_BUTTON_PRESS || cursorType != CursorType::RESIZE_CURSOR) {
        // Cannot resize
        return;

    }

    auto dirt = currentResizePoint->getDirt();
    int mousePosX = event->pos().x();
    int mousePosY = event->pos().y();
    switch (dirt) {
    case ReSizeDirection::TOP_LEFT:
        if((mousePosX >= 0 && mousePosX < (m_selectArea->bottomRight().x() - SMALLEST_RECORD_SIZE) &&
            mousePosY >= 0 && mousePosY < (m_selectArea->bottomRight().y() - SMALLEST_RECORD_SIZE))) {
            m_selectArea->setTopLeft(event->pos());
        }
        break;
    case ReSizeDirection::TOP_MIDDLE:
        if(mousePosY >= 0 && mousePosY <( m_selectArea->bottomRight().y() - SMALLEST_RECORD_SIZE)) {
            // int oldHeight = m_selectArea->height();
            m_selectArea->setTopLeft(QPoint({m_selectArea->topLeft().x(), mousePosY}));
        }
        break;
    case ReSizeDirection::TOP_RIGHT:
        if(mousePosX >= 0 &&
                mousePosX > (m_selectArea->bottomLeft().x() + SMALLEST_RECORD_SIZE) &&
                mousePosY >= 0 &&
                mousePosY < (m_selectArea->bottomLeft().y() - SMALLEST_RECORD_SIZE)) {
            m_selectArea->setTopRight(QPoint({mousePosX, mousePosY}));
        }
        break;
    case ReSizeDirection::MIDDLE_LEFT:
        if(mousePosX >= 0 && mousePosX < (m_selectArea->topRight().x() - SMALLEST_RECORD_SIZE)) {
            m_selectArea->setTopLeft(QPoint({mousePosX, m_selectArea->topLeft().y()}));
        }
        break;
    case ReSizeDirection::MIDDLE_RIGHT:
        if(mousePosX > (m_selectArea->topLeft().x() + SMALLEST_RECORD_SIZE)) {
            m_selectArea->setTopRight(QPoint({mousePosX, m_selectArea->topLeft().y()}));
        }
        break;
    case ReSizeDirection::BOTTOM_LEFT:
        if(mousePosX < (m_selectArea->topRight().x() - SMALLEST_RECORD_SIZE) &&
                mousePosY > (m_selectArea->topRight().y() + SMALLEST_RECORD_SIZE)) {
            m_selectArea->setBottomLeft(QPoint({mousePosX, mousePosY}));
        }
        break;
    case ReSizeDirection::BOTTOM_MIDDLE:
        if(mousePosY > (m_selectArea->topRight().y() + SMALLEST_RECORD_SIZE)) {
            m_selectArea->setBottomLeft(QPoint({m_selectArea->topLeft().x(), mousePosY}));
        }
        break;
    case ReSizeDirection::BOTTOM_RIGHT:
        if(mousePosX > (m_selectArea->topLeft().x() + SMALLEST_RECORD_SIZE) &&
                mousePosY > (m_selectArea->topLeft().y() + SMALLEST_RECORD_SIZE)) {
            m_selectArea->setBottomRight(event->pos());
        }
        break;

    }

    update();
}

void RecordScreen::moveToNewPos(QPoint newPos) {
    int width = m_selectArea->width();
    int height = m_selectArea->height();
    m_selectArea->setTopLeft(newPos);
    m_selectArea->setWidth(width);
    m_selectArea->setHeight(height);
}

bool RecordScreen::isValidPosToMove(QPoint newPos) {
    int x = newPos.x();
    int y = newPos.y();

    return (x >= 0 && y >= 0 && (x + m_selectArea->width() <= screenWidth) && (y + m_selectArea->height() <= screenHeight));
}

QPoint RecordScreen::correctMovePoint(QPoint posToMove) {
    QPoint newPos = posToMove;
    if(newPos.x() < 0) {
        newPos.setX(0);
    }

    if(newPos.y() < 0) {
        newPos.setY(0);
    }

    if(newPos.x() + m_selectArea->width() > screenWidth) {
        newPos.setX(screenWidth - m_selectArea->width());
    }

    if(newPos.y() + m_selectArea->height() > screenHeight) {
        newPos.setY(screenHeight -  m_selectArea->height());
    }

    return newPos;
}

void RecordScreen::handleMoveEvent(QMouseEvent *event) {
    auto currMousePos = event->pos();
    auto newPosToMove = QPoint(m_selectArea->topLeft() + QPoint(currMousePos - startMousePos));

    moveToNewPos(correctMovePoint(newPosToMove));
    startMousePos = currMousePos;
    update();
}

void RecordScreen::prepareToMove() {
    startMousePos = QCursor::pos();
}

void RecordScreen::mouseMoveEvent(QMouseEvent *event) {
    if(mouseStatus == MouseStatus::RELEASE) {
        if(isResizePointUnderMouse(event->pos())) {
            changeCursorToResizeCursor();
        } else if(isMoveAreaUnderMousePoint(event->pos())) {
            // prepareToMove();
            setCursor((Qt::SizeAllCursor));
            cursorType = CursorType::SIZE_ALL_CURSOR;
        } else {
            unsetCursor();
            cursorType = CursorType::ARROW_CURSOR;
        }
    } else if(mouseStatus == MouseStatus::LEFT_BUTTON_PRESS) {
        switch (cursorType) {
        case CursorType::RESIZE_CURSOR:
            hanleResizeEvent(event);
            break;
        case CursorType::SIZE_ALL_CURSOR:
            handleMoveEvent(event);
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
        if(cursorType == CursorType::SIZE_ALL_CURSOR) {
            prepareToMove();
        }
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
    if(!isRecording) {
        drawResizePoint();
        drawMovePoint();
    }
}
