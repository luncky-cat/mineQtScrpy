#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QListWidget>
#include <QWidget>

class DeviceListView;

class SidebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

signals:
    void toggleVisibility();  // 控制隐藏/显示

private:
    QWidget* createTopControls();
    QWidget* createGroupControls();
    QWidget* createDeviceList();

    // 你可以将控件暴露出来方便主界面控制
    QListWidget* deviceListWidget;
};

#endif // SIDEBARWIDGET_H
