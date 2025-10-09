#include "c_ui/MainWindow.h"
#include "c_ui/workbenches/DocumentPage.h"   
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QStyle>
#include <QStatusBar>
#include <QMouseEvent>
#include <QEvent>

#if USE_VTK
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#endif

// 构造函数
CTViewer::CTViewer(QWidget* parent)
    : QMainWindow(parent)
{
    // ---- 无边框窗口 + 深色主题 ----
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowTitle(QStringLiteral("CTviewer_demo"));
    setStyleSheet(QStringLiteral(
        "QMainWindow{background-color:#121212;}"
        "QMenuBar, QStatusBar{background-color:#1a1a1a; color:#e0e0e0;}"));

    // ---- 搭建结构 ----
    buildTitleBar();
    buildCentral();     // DocumentPage
    wireSignals();      // 连接信号
    setDefaults();      // 默认状态

    statusBar()->showMessage(QStringLiteral("就绪"));
}

CTViewer::~CTViewer() = default;



// 自定义标题栏

void CTViewer::buildTitleBar()
{
    titleBar_ = new QWidget(this);
    titleBar_->setObjectName(QStringLiteral("customTitleBar"));
    titleBar_->setFixedHeight(38);
    titleBar_->setStyleSheet(QStringLiteral(
        "QWidget#customTitleBar{background-color:#202020;}"
        "QToolButton{background:transparent; border:none; color:#f5f5f5; padding:6px;}"
        "QToolButton:hover{background-color:rgba(255,255,255,0.12);}"
        "QLabel#titleLabel{color:#f5f5f5; font-size:14px; font-weight:600;}"));

    auto* barLayout = new QHBoxLayout(titleBar_);
    barLayout->setContentsMargins(10, 0, 4, 0);
    barLayout->setSpacing(0);

    // ---- 左侧撤回/前进按钮 ----
    titleLeftArea_ = new QWidget(titleBar_);
    auto* leftLayout = new QHBoxLayout(titleLeftArea_);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);

    btnTitleUndo_ = new QToolButton(titleLeftArea_);
    btnTitleUndo_->setToolTip(QStringLiteral("撤回"));
    btnTitleUndo_->setCursor(Qt::PointingHandCursor);
    btnTitleUndo_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    btnTitleUndo_->setAutoRaise(true);
    leftLayout->addWidget(btnTitleUndo_);

    btnTitleUndo02_ = new QToolButton(titleLeftArea_);
    btnTitleUndo02_->setToolTip(QStringLiteral("前进"));
    btnTitleUndo02_->setCursor(Qt::PointingHandCursor);
    btnTitleUndo02_->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    btnTitleUndo02_->setAutoRaise(true);
    leftLayout->addWidget(btnTitleUndo02_);

    titleLeftArea_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    titleLeftArea_->installEventFilter(this);
    barLayout->addWidget(titleLeftArea_, 0);

    // ---- 中间标题 ----
    titleCenterArea_ = new QWidget(titleBar_);
    auto* centerLayout = new QHBoxLayout(titleCenterArea_);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    titleLabel_ = new QLabel(windowTitle(), titleCenterArea_);
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    centerLayout->addWidget(titleLabel_);
    titleCenterArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleCenterArea_->installEventFilter(this);
    titleLabel_->installEventFilter(this);
    barLayout->addWidget(titleCenterArea_, 1);

    // ---- 右侧控制按钮 ----
    auto* rightContainer = new QWidget(titleBar_);
    auto* rightLayout = new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    auto makeBtn = [&](QPointer<QToolButton>& btn, QStyle::StandardPixmap icon, const QString& tip) {
        btn = new QToolButton(rightContainer);
        btn->setToolTip(tip);
        btn->setIcon(style()->standardIcon(icon));
        btn->setStyleSheet(
            "QToolButton { color:white; border-radius:4px; }"
            "QToolButton:hover { background-color:#6EAD3E; }");
        rightLayout->addWidget(btn);
        };
    makeBtn(btnMinimize_, QStyle::SP_TitleBarMinButton, QStringLiteral("最小化"));
    makeBtn(btnMaximize_, QStyle::SP_TitleBarMaxButton, QStringLiteral("最大化"));
    makeBtn(btnClose_, QStyle::SP_TitleBarCloseButton, QStringLiteral("关闭"));
    barLayout->addWidget(rightContainer, 0);

    // ---- 安装拖拽事件 ----
    titleBar_->installEventFilter(this);
    setMenuWidget(titleBar_);

    // ---- 连接标题栏按钮 ----
    connect(btnMinimize_, &QToolButton::clicked, this, &CTViewer::showMinimized);
    connect(btnMaximize_, &QToolButton::clicked, this, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
        updateMaximizeButtonIcon();
        });
    connect(btnClose_, &QToolButton::clicked, this, &CTViewer::close);

    updateMaximizeButtonIcon();
}

void CTViewer::updateMaximizeButtonIcon()
{
    if (!btnMaximize_) return;
    if (isMaximized()) {
        btnMaximize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        btnMaximize_->setToolTip(QStringLiteral("还原"));
    }
    else {
        btnMaximize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        btnMaximize_->setToolTip(QStringLiteral("最大化"));
    }
}


// 事件过滤器（实现标题栏拖动）
bool CTViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (!event) return false;
    bool titleArea = (watched == titleBar_.data()
        || watched == titleLeftArea_.data()
        || watched == titleCenterArea_.data()
        || watched == titleLabel_.data());
    if (titleArea) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto* e = static_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton) {
                draggingWindow_ = true;
                dragOffset_ = e->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            if (draggingWindow_) {
                auto* e = static_cast<QMouseEvent*>(event);
                move(e->globalPos() - dragOffset_);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease:
            draggingWindow_ = false;
            break;
        case QEvent::MouseButtonDblClick:
            draggingWindow_ = false;
            if (isMaximized()) showNormal();
            else showMaximized();
            updateMaximizeButtonIcon();
            return true;
        default: break;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}


// 中央内容区域（只加载 DocumentPage）

void CTViewer::buildCentral()
{
    auto central = new QWidget(this);
    auto v = new QVBoxLayout(central);
    v->setContentsMargins(0, 0, 0, 0);

    stack_ = new QStackedWidget(central);
    v->addWidget(stack_);
    setCentralWidget(central);

    //  加载 DocumentPage 两个页面
    pageDocument_ = new DocumentPage(stack_);
    stack_->addWidget(pageDocument_);
    stack_->setCurrentWidget(pageDocument_);

    // ---- 连接 DocumentPage 发出的信号 ----
    connect(pageDocument_, &DocumentPage::moduleClicked, this, [this](const QString& msg) {
        statusBar()->showMessage(msg, 1500);
        });
    connect(pageDocument_, &DocumentPage::requestSwitchTo, this, [this](const QString& key) {
        statusBar()->showMessage(QStringLiteral("请求切换到页面：%1").arg(key), 1500);
        // 未来：可在此根据 key 切换其他工作台
        });
    connect(pageDocument_, &DocumentPage::recentOpenRequested, this, [this](const QString& name) {
        statusBar()->showMessage(QStringLiteral("正在打开 %1 ...").arg(name), 1500);
        });
}

// 空函数（为兼容老结构保留）
void CTViewer::wireSignals() {}
void CTViewer::setDefaults() {}
