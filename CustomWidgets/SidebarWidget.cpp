#include "SidebarWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
#include "DeviceListView.h"

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(createTopControls());
    mainLayout->addWidget(createGroupControls());
    mainLayout->addWidget(createDeviceList());
    mainLayout->addStretch();
}

// 顶部功能按钮区
QWidget* SidebarWidget::createTopControls()
{
    QWidget* top = new QWidget(this);
    auto* layout = new QVBoxLayout(top);

    // 行1: 侧边栏开关按钮 + 文本
    auto* h1 = new QHBoxLayout();
    QPushButton* toggleBtn = new QPushButton("⏩", this);
    QLabel* label = new QLabel("侧边栏", this);
    h1->addWidget(toggleBtn);
    h1->addWidget(label);
    h1->addStretch();

    connect(toggleBtn, &QPushButton::clicked, this, &SidebarWidget::toggleVisibility);

    // 行2: 控制按钮
    QPushButton* sendFileBtn = new QPushButton("发送文件");
    QPushButton* installApkBtn = new QPushButton("安装APK");
    QPushButton* adbShellBtn = new QPushButton("ADB Shell");
    QPushButton* scanWiFiBtn = new QPushButton("扫描WiFi");
    QPushButton* settingsBtn = new QPushButton("设置");

    layout->addLayout(h1);
    layout->addWidget(sendFileBtn);
    layout->addWidget(installApkBtn);
    layout->addWidget(adbShellBtn);
    layout->addWidget(scanWiFiBtn);
    layout->addWidget(settingsBtn);

    return top;
}

// 分组管理区域
QWidget* SidebarWidget::createGroupControls()
{
    QGroupBox* groupBox = new QGroupBox("分组管理", this);
    auto* layout = new QVBoxLayout(groupBox);

    auto* comboLayout = new QHBoxLayout();
    QComboBox* groupComboBox = new QComboBox(this);
    QPushButton* addBtn = new QPushButton("+", this);
    QPushButton* removeBtn = new QPushButton("-", this);
    comboLayout->addWidget(groupComboBox);
    comboLayout->addWidget(addBtn);
    comboLayout->addWidget(removeBtn);

    QLineEdit* filterBox = new QLineEdit(this);
    filterBox->setPlaceholderText("筛选设备");

    layout->addLayout(comboLayout);
    layout->addWidget(filterBox);

    return groupBox;
}

QWidget* SidebarWidget::createDeviceList()
{
    QWidget* containerWidget = new QWidget(this);  // 创建一个容器 widget

    // 创建一个垂直布局
    QVBoxLayout* layout = new QVBoxLayout(containerWidget);

    // 创建 GroupBox 作为标题区域
    QGroupBox* deviceListGroupBox = new QGroupBox("设备列表", containerWidget);  // 设置标题

    // 创建一个垂直布局，用于 GroupBox 内部
    QVBoxLayout* groupLayout = new QVBoxLayout(deviceListGroupBox);

    // 创建设备列表
    deviceListWidget = new QListWidget(deviceListGroupBox);  // 放置在 GroupBox 内部
    deviceListWidget->addItem("设备1");
    deviceListWidget->addItem("设备2");
    deviceListWidget->addItem("设备3");

    groupLayout->addWidget(deviceListWidget);  // 将设备列表添加到 GroupBox 的布局中

    deviceListGroupBox->setLayout(groupLayout);  // 设置 GroupBox 的布局
    layout->addWidget(deviceListGroupBox);  // 将 GroupBox 添加到整体布局中

    containerWidget->setLayout(layout);  // 设置布局
    return containerWidget;  // 返回包含设备列表和标题的容器
}

