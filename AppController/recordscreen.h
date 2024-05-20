/*
 * TODO:
 */

#ifndef RECORDSCREEN_H
#define RECORDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

#include <iostream>

#define SELECTOR_BORDER_SIZE 5
#define SELECTOR_WIDTH 600
#define SELECTOR_HEIGHT 200
#define SELECTOR_DEFAULT_COLOR "cyan"
#define RESIZE_POINT_COLOR  "darkCyan"
#define SELECTOR_RESIZEPOINT_PADDING 10

class RecordScreen;
class ResizePoint;

enum ReSizeDirection {
    TOP_LEFT = 0,
    TOP_MIDDLE,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_MIDDLE,
    BOTTOM_RIGHT
};

/*
 * A point (rectangle) which user can use to resize the screen selector
 */
class ResizePoint : public QWidget
{
#define RESIZE_POINT_SIZE 20

public:
    ResizePoint(RecordScreen *recordScreen = nullptr, int x = 0, int y = 0, ReSizeDirection dirt = ReSizeDirection::TOP_LEFT);

    QRect getRect() {return rect;}
    ReSizeDirection getDirt() {return m_dirt;}

private:
    int off_x;
    int off_y;
    ReSizeDirection m_dirt;
    RecordScreen* m_recordScreen;
    QRect rect;

protected:
    // void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    bool isRectUnderMouse(QPoint mountPoint);
};

class RecordScreen : public QWidget
{
    enum CursorType {
        ARROW_CURSOR = 0,
        RESIZE_CURSOR = 1
    };

    enum MouseStatus {
        RELEASE = 0,
        LEFT_BUTTON_PRESS,
        RIGHT_BUTTON_PRESS,
    };

public:
    RecordScreen();
    QRect getScreenSelector(){return *m_selectArea;}

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    ResizePoint *rpTopLeft = nullptr;
    ResizePoint *rpTopMiddle = nullptr;
    ResizePoint *rpTopRight = nullptr;
    ResizePoint *rpMiddleLeft = nullptr;
    ResizePoint *rpMiddleRight = nullptr;
    ResizePoint *rpBottomLeft = nullptr;
    ResizePoint *rpBottomMiddle = nullptr;
    ResizePoint *rpBottomRight = nullptr;

private:
    QRect *m_selectArea;
    void drawScreenSelector();
    void drawResizePoint();
    void drawMovePoint();
    void setMoveableSelector();

    // For resizing
    CursorType cursorType = CursorType::ARROW_CURSOR;
    MouseStatus mouseStatus = MouseStatus::RELEASE;
    ResizePoint* currentResizePoint = nullptr;
    bool isResizePointUnderMouse(ResizePoint *rsPoint, QPoint mousePoint);
    bool isResizePointUnderMouse(QPoint mousePoint);
    void changeCursorToResizeCursor();
    ResizePoint* getUnderMouseResizePoint();
    void hanleResizeEvent(QMouseEvent *event);
};

#endif // RECORDSCREEN_H
