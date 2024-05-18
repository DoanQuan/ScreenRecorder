/*
 * TODO:
 */

#ifndef RECORDSCREEN_H
#define RECORDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPainter>

#define SELECTOR_BORDER_SIZE 5
#define SELECTOR_WIDTH 600
#define SELECTOR_HEIGHT 200
class RecordScreen : public QWidget
{
public:
    RecordScreen();
    QRect getScreenSelector(){return *m_screenSelector;}

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QRect *m_screenSelector;
};

#endif // RECORDSCREEN_H
