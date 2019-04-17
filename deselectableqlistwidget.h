#ifndef DESELECTABLEQLISTWIDGET_H
#define DESELECTABLEQLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QListWidget>

class DeselectableQListWidget : public QListWidget
{
public:
    DeselectableQListWidget(QWidget *parent = nullptr) : QListWidget(parent) {}
    virtual ~DeselectableQListWidget() {}

private:
    virtual void focusOutEvent(QFocusEvent *event)
    {
        clearSelection();
    }
};

#endif // DESELECTABLEQLISTWIDGET_H
