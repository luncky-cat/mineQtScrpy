#ifndef DEVICELISTVIEW_H
#define DEVICELISTVIEW_H

class DeviceListModel;

#include <QListView>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionViewItem>

class DeviceListView:public QListView
{
public:
    Q_OBJECT
public:
    explicit DeviceListView(QWidget *parent = nullptr);
    void setDeviceModel(DeviceListModel *model); // 设置自定义模型
protected:
   void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const; //override
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    DeviceListModel *m_deviceModel;
};

#endif // DEVICELISTVIEW_H
