#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <vector>

class FullScreenWidget : public QWidget
{
    Q_OBJECT

public:
    FullScreenWidget(QWidget* parent);

    void addWidget(QWidget* widget);
    void childEvent(QChildEvent* e);

    int indexOf(QWidget* widget) const { return layout->indexOf(widget); }
    int getCount() const { return count; }

private:

    QHBoxLayout* layout;
    QScreen* screen;
    int count = 0; // Strangely, layout->count() always returns 0, so i reimplement it;

signals:
    void numberChildrenChanged();
};