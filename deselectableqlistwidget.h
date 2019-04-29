#ifndef DESELECTABLEQLISTWIDGET_H
#define DESELECTABLEQLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QMainWindow>
#include <QApplication>

class DeselectableQListWidget : public QListWidget
{
public:
    QList<QListWidgetItem *> lastSelected;
    DeselectableQListWidget(QWidget *parent = nullptr) : QListWidget(parent) {}
    virtual ~DeselectableQListWidget() {}
    QPushButton *deleteButton;

private:
    virtual void focusOutEvent(QFocusEvent *event)
    {
        QWidget *w = QApplication::widgetAt(QCursor::pos());
        QWidget *db = static_cast<QWidget*>(deleteButton);
        if (w != db)
        {
            lastSelected = this->selectedItems();
            clearSelection();
        }
    }
};

#endif // DESELECTABLEQLISTWIDGET_H
