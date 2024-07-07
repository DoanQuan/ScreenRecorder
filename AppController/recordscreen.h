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

#define SELECTOR_BORDER_SIZE 2
#define SELECTOR_WIDTH 600
#define SELECTOR_HEIGHT 200
#define SELECTOR_DEFAULT_COLOR "lightgreen"
#define SELECTOR_RECORD_COLOR "red"
#define RESIZE_POINT_COLOR  "cyan"
#define MOVE_POINT_COLOR "cyan"
#define SELECTOR_RESIZEPOINT_PADDING 2
#define SMALLEST_RECORD_SIZE 80
#define RESIZE_POINT_SIZE 8
#define MOVE_AREA_WIDTH 30
#define MOVE_AREA_HEIGHT 30
#define MOVE_CURSOR_SIZE 6

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
        RESIZE_CURSOR = 1,
        SIZE_ALL_CURSOR
    };

    enum MouseStatus {
        RELEASE = 0,
        LEFT_BUTTON_PRESS,
        RIGHT_BUTTON_PRESS,
    };

public:
    RecordScreen();
    ~RecordScreen() = default;
    QRect getScreenSelector(){return *m_selectArea;}
    void toRecordState();
    void toNormalState();
    bool isRecordState() {return isRecording;}

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
    bool isRecording;
    int screenHeight;
    int screenWidth;
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
    QColor selectorColor;

    // For moving screen selector
    QRect moveCursorArea;   // Area that can use to move the screen select area
    bool isMoveAreaUnderMousePoint(QPoint point);
    void prepareToMove();
    bool isValidPosToMove(QPoint newPos);
    void handleMoveEvent(QMouseEvent *event);
    void moveToNewPos(QPoint newPos);
    QPoint correctMovePoint(QPoint posToMove);
    QPoint startMousePos;
};

#endif // RECORDSCREEN_H
