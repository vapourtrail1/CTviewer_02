#include "MainWindow.h"
#include <QApplication>
#include <QDockWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStatusBar>
#include <QFrame>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStyle>
#include <QMouseEvent>
#include <QEvent>
#if USE_VTK
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#endif

CTViewer::CTViewer(QWidget* parent)
    : QMainWindow(parent)
{
    // ---- 启用自定义无边框窗口，以便自行绘制标题栏 ----
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowTitle(QStringLiteral("CTviewer_demo"));


    //颜色
    setStyleSheet(QStringLiteral(
        "QMainWindow{background-color:#121212;}"
        "QDockWidget{background-color:#1a1a1a;color:#f0f0f0;}"
        "QMenuBar, QStatusBar{background-color:#1a1a1a;color:#e0e0e0;}"));

    // ---- 构建自定义标题栏，放置中心标题与撤回按钮 ----
    buildTitleBar();
    buildCentral();
    buildNavDock();
    wireSignals();
    setDefaults();

#if USE_VTK
    setupVTKViews();
#endif

    statusBar()->showMessage(QStringLiteral("就绪"));
}

CTViewer::~CTViewer() = default;


void CTViewer::buildTitleBar()
{
    // ---- 创建标题栏主体并设置样式，保持深色主题 ----
    titleBar_ = new QWidget(this);
    titleBar_->setObjectName(QStringLiteral("customTitleBar"));
    titleBar_->setFixedHeight(38);
    titleBar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleBar_->setStyleSheet(QStringLiteral(
        "QWidget#customTitleBar{background-color:#202020;}"
        "QToolButton{background:transparent; border:none; color:#f5f5f5; padding:6px;}"
        "QToolButton:hover{background-color:rgba(255,255,255,0.12);}"
        "QLabel#titleLabel{color:#f5f5f5; font-size:14px; font-weight:600;}"));

    auto* barLayout = new QHBoxLayout(titleBar_);
    barLayout->setContentsMargins(10, 0, 4, 0);
    barLayout->setSpacing(0);

    // ---- 左侧区域：放置撤回和按钮，并允许拖拽移动窗口 ----
    titleLeftArea_ = new QWidget(titleBar_);//这句话的意思是创建一个新的 QWidget 对象，并将其父对象设置为 titleBar_，这样 titleLeftArea_ 就成为 titleBar_ 的子部件
    auto* leftLayout = new QHBoxLayout(titleLeftArea_);//这句话的意思是创建一个水平布局管理器，并将其设置为 titleLeftArea_ 小部件的布局管理器
    leftLayout->setContentsMargins(0, 0, 0, 0);//这句话的意思是将布局管理器的边距设置为0，这样布局中的小部件就会紧贴着布局的边缘，没有额外的空白区域
    leftLayout->setSpacing(6);//这句话的意思是将布局管理器中小部件之间的间距设置为6像素，这样布局中的小部件之间会有一定的空隙，而不是紧挨在一起

    btnTitleUndo_ = new QToolButton(titleLeftArea_);//这句话的意思是创建一个新的 QToolButton 对象，并将其父对象设置为 titleLeftArea_，这样 btnTitleUndo_ 就成为 titleLeftArea_ 的子部件
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

    // ---- 中间区域：放置标题文本，保持居中显示 ----
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

    // ---- 右侧区域：构建最小化、最大化和关闭按钮 ----
    auto* rightContainer = new QWidget(titleBar_);
    auto* rightLayout = new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    btnMinimize_ = new QToolButton(rightContainer);
    btnMinimize_->setToolTip(QStringLiteral("最小化"));
    btnMinimize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    btnMinimize_->setStyleSheet(
        "QToolButton {"
        "  color: white;"              // 图标/文字颜色
        "  border-radius: 4px;"        // 圆角
        "}"
        "QToolButton:hover {"
        "  background-color: #6EAD3E;" // 悬停时颜色
        "}"
    );
    rightLayout->addWidget(btnMinimize_);

    btnMaximize_ = new QToolButton(rightContainer);
    btnMaximize_->setToolTip(QStringLiteral("最大化"));
    btnMaximize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    btnMaximize_->setStyleSheet(
        "QToolButton {"
        "  color: white;"              // 图标/文字颜色
        "  border-radius: 4px;"        // 圆角
        "}"
        "QToolButton:hover {"
        "  background-color: #6EAD3E;" // 悬停时颜色
        "}"
    );
    rightLayout->addWidget(btnMaximize_);

    btnClose_ = new QToolButton(rightContainer);
    btnClose_->setToolTip(QStringLiteral("关闭"));
    btnClose_->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    btnClose_->setStyleSheet(
        "QToolButton {"
        "  color: white;"              // 图标/文字颜色
        "  border-radius: 4px;"        // 圆角
        "}"
        "QToolButton:hover {"
        "  background-color: #6EAD3E;" // 悬停时颜色
        "}"
    );
    rightLayout->addWidget(btnClose_);

    rightContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    barLayout->addWidget(rightContainer, 0);

    // ---- 安装事件过滤器以便整条标题栏可拖拽 ----
    titleBar_->installEventFilter(this);

    // ---- 将标题栏挂载到窗口顶部（替换默认菜单栏区域） ----
    setMenuWidget(titleBar_);

    // ---- 连接窗口控制按钮的逻辑 ----
    connect(btnMinimize_, &QToolButton::clicked, this, [this]() {
        // 通过调用 showMinimized() 实现窗口最小化
        showMinimized();
        });
    connect(btnMaximize_, &QToolButton::clicked, this, [this]() {
        // 双态：当前最大化则恢复，否则执行最大化
        if (isMaximized()) {
            showNormal();
        }
        else {
            showMaximized();
        }
        updateMaximizeButtonIcon();
        });
    connect(btnClose_, &QToolButton::clicked, this, [this]() {
        // 保持 close() 调用，确保行为与系统标题栏一致
        close();
        });

    connect(btnTitleUndo_, &QToolButton::clicked, this, [this]() {
        // 当欢迎页的撤回按钮存在时，同步触发其点击逻辑
        if (btnUndo_) {
            btnUndo_->click();
        }
        });

    updateMaximizeButtonIcon();
}

// ---- 根据窗口状态刷新最大化按钮的图标与提示 ----
void CTViewer::updateMaximizeButtonIcon()
{
    if (!btnMaximize_) {
        return;
    }

    if (isMaximized()) {
        btnMaximize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        btnMaximize_->setToolTip(QStringLiteral("还原"));
    }
    else {
        btnMaximize_->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        btnMaximize_->setToolTip(QStringLiteral("最大化"));
    }
}

// ---- 监听窗口状态与标题变化，保持自定义标题栏同步 ----
void CTViewer::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);

    if (!event) {
        return;
    }

    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeButtonIcon();
    }
    else if (event->type() == QEvent::WindowTitleChange) {
        if (titleLabel_) {
            titleLabel_->setText(windowTitle());
        }
    }
}

// ---- 处理标题栏的鼠标事件，实现拖动与双击切换窗口状态 ----
bool CTViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (!event) {
        return QMainWindow::eventFilter(watched, event);
    }

    const bool titleArea = (watched == titleBar_.data() || watched == titleLeftArea_.data() || watched == titleCenterArea_.data() || watched == titleLabel_.data());
    if (titleArea) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                draggingWindow_ = true;
                dragOffset_ = mouseEvent->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            if (draggingWindow_) {
                auto* mouseEvent = static_cast<QMouseEvent*>(event);
                move(mouseEvent->globalPos() - dragOffset_);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                draggingWindow_ = false;
                return true;
            }
            break;
        }
        case QEvent::MouseButtonDblClick: {
            // 双击标题栏切换最大化状态
            draggingWindow_ = false;
            if (isMaximized()) {
                showNormal();
            }
            else {
                showMaximized();
            }
            updateMaximizeButtonIcon();
            return true;
        }
        default:
            break;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// ------------------ 构造 ------------------

void CTViewer::buildCentral()
{
    auto central = new QWidget(this);
    auto v = new QVBoxLayout(central);
    v->setContentsMargins(0, 0, 0, 0);
    stack_ = new QStackedWidget(central);
    v->addWidget(stack_);
    setCentralWidget(central);

    buildWelcomePage();
    buildSlicesPage();

    stack_->setCurrentWidget(pageWelcome_);
}

void CTViewer::buildWelcomePage()
{
    pageWelcome_ = new QWidget(stack_);
    pageWelcome_->setObjectName(QStringLiteral("pageWelcome"));

    // 通过统一的样式表设置欢迎页背景以及文字颜色，打造与示例界面类似的深色风格。
    pageWelcome_->setStyleSheet(QStringLiteral(
        "QWidget#pageWelcome{background-color:#181818;}"
        "QLabel{color:#f2f2f2;}"));

    auto vl = new QVBoxLayout(pageWelcome_);
    vl->setContentsMargins(18, 18, 18, 18);
    vl->setSpacing(16);


    // 顶部横幅，显示产品名称与版本信息。
    auto banner = new QFrame(pageWelcome_);
    banner->setObjectName(QStringLiteral("heroBanner"));
    banner->setStyleSheet(QStringLiteral(
        "QFrame#heroBanner{background:#262626; border-radius:10px;}"
        "QFrame#heroBanner QLabel{color:#f9f9f9;}"));
    auto bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(20, 16, 20, 16);
    bannerLayout->setSpacing(8);
    auto title = new QLabel(QStringLiteral("欢迎使用 CTviewer_demo"), banner);
    title->setStyleSheet(QStringLiteral("font-size:24px; font-weight:700;"));
    bannerLayout->addWidget(title);
    auto subtitle = new QLabel(QStringLiteral("继续最近项目，或通过下方模块快速开始您的工业 CT 工作流程。"), banner);
    subtitle->setStyleSheet(QStringLiteral("font-size:14px; color:#bbbbbb;"));
    subtitle->setWordWrap(true);
    bannerLayout->addWidget(subtitle);
    vl->addWidget(banner);

    // 操作提示卡片：去除「快速开始」模块后，单独保留提示信息以简化界面。
    auto tipsFrame = new QFrame(pageWelcome_);
    tipsFrame->setObjectName(QStringLiteral("tipsFrame"));
    tipsFrame->setStyleSheet(QStringLiteral(
        "QFrame#tipsFrame{background:#202020; border-radius:10px;}"
        "QFrame#tipsFrame QLabel{color:#d8d8d8;}"));
    auto tipsLayout = new QVBoxLayout(tipsFrame);
    tipsLayout->setContentsMargins(20, 18, 20, 18);
    tipsLayout->setSpacing(10);
    auto tipsTitle = new QLabel(QStringLiteral("操作提示"), tipsFrame);
    tipsTitle->setStyleSheet(QStringLiteral("font-size:16px; font-weight:600;"));
    tipsLayout->addWidget(tipsTitle);
    auto tips = new QLabel(QStringLiteral("1.可直接导入 DICOM、TIFF、RAW 等常见工业 CT 数据。\n"
        "2.若需培训资料，可访问帮助中心以获取最新教程。"), tipsFrame);
    tips->setWordWrap(true);
    tips->setStyleSheet(QStringLiteral("font-size:13px; line-height:20px;"));
    tipsLayout->addWidget(tips);
    tipsLayout->addStretch();

    vl->addWidget(tipsFrame);

    // 模块入口按钮，使用网格布局保持紧凑。
    auto moduleFrame = new QFrame(pageWelcome_);
    moduleFrame->setObjectName(QStringLiteral("moduleFrame"));
    moduleFrame->setStyleSheet(QStringLiteral(
        "QFrame#moduleFrame{background:#202020; border-radius:10px;}"
        "QFrame#moduleFrame QPushButton{background:#262626; border-radius:8px; border:1px solid #333;"
        " color:#f5f5f5; font-size:16px; padding:18px 12px;}"
        "QFrame#moduleFrame QPushButton:hover{background:#313131; border-color:#4d6fff;}"
        "QFrame#moduleFrame QLabel{color:#f5f5f5;}"));
    auto moduleLayout = new QVBoxLayout(moduleFrame);
    moduleLayout->setContentsMargins(20, 18, 20, 18);
    moduleLayout->setSpacing(12);
    auto moduleTitle = new QLabel(QStringLiteral("选择最适合您工作流程的“开始”选项卡"), moduleFrame);
    moduleTitle->setStyleSheet(QStringLiteral("font-size:16px; font-weight:600;"));
    moduleLayout->addWidget(moduleTitle);

    auto grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    btnVisCheck_ = new QPushButton(QStringLiteral("视觉检查"), moduleFrame);
    btnPorosity_ = new QPushButton(QStringLiteral("孔隙度"), moduleFrame);
    btnMetrology_ = new QPushButton(QStringLiteral("计量"), moduleFrame);
    btnMaterial_ = new QPushButton(QStringLiteral("材料"), moduleFrame);
    for (auto* b : { btnVisCheck_.data(), btnPorosity_.data(), btnMetrology_.data(), btnMaterial_.data() }) {
        b->setMinimumSize(160, 70);
    }
    grid->addWidget(btnVisCheck_, 0, 0);
    grid->addWidget(btnPorosity_, 0, 1);
    grid->addWidget(btnMetrology_, 0, 2);
    grid->addWidget(btnMaterial_, 0, 3);
    moduleLayout->addLayout(grid);

    vl->addWidget(moduleFrame);

    // 最近项目列表，使用 QTableWidget 填充示例数据，模拟真实历史记录。
    auto recentFrame = new QFrame(pageWelcome_);
    recentFrame->setObjectName(QStringLiteral("recentFrame"));
    recentFrame->setStyleSheet(QStringLiteral(
        "QFrame#recentFrame{background:#202020; border-radius:10px;}"
        "QFrame#recentFrame QLabel{color:#f5f5f5;}"
        "QFrame#recentFrame QHeaderView::section{background:#2c2c2c; color:#f0f0f0; border:0;}"
        "QFrame#recentFrame QTableWidget{background:transparent; border:0; color:#f5f5f5;}"
        "QFrame#recentFrame QTableWidget::item:selected{background-color:#3d65f5;}"));
    auto recentLayout = new QVBoxLayout(recentFrame);
    recentLayout->setContentsMargins(20, 18, 20, 18);
    recentLayout->setSpacing(12);
    auto recentTitle = new QLabel(QStringLiteral("最近项目"), recentFrame);
    recentTitle->setStyleSheet(QStringLiteral("font-size:16px; font-weight:600;"));
    recentLayout->addWidget(recentTitle);

    tableRecent_ = new QTableWidget(0, 3, recentFrame);
    tableRecent_->setHorizontalHeaderLabels({ QStringLiteral("名称"), QStringLiteral("位置"), QStringLiteral("上次打开") });
    tableRecent_->horizontalHeader()->setStretchLastSection(true);
    tableRecent_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tableRecent_->verticalHeader()->setVisible(false);
    tableRecent_->setShowGrid(false);
    tableRecent_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableRecent_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableRecent_->setAlternatingRowColors(true);
    tableRecent_->setStyleSheet(QStringLiteral(
        "QTableWidget{alternate-background-color:#1f1f1f;}"
        "QTableWidget QTableCornerButton::section{background:#2c2c2c;}"));

    struct RecentItem
    {
        QString name;
        QString path;
        QString time;
    };
    const QList<RecentItem> recents = {
        { QStringLiteral("发动机缸体.vgl"), QStringLiteral("D:/Projects/CT/EngineBlock"), QStringLiteral("今天 09:24") },
        { QStringLiteral("齿轮箱.vgl"), QStringLiteral("D:/Projects/CT/GearBox"), QStringLiteral("昨天 17:42") },
        { QStringLiteral("叶片扫描.vgi"), QStringLiteral("E:/Scan/Blade"), QStringLiteral("2024-05-12") },
        { QStringLiteral("材料试样.raw"), QStringLiteral("E:/Lab/Materials"), QStringLiteral("2024-04-28") }
    };
    for (const auto& item : recents) {
        const int row = tableRecent_->rowCount();
        tableRecent_->insertRow(row);
        tableRecent_->setItem(row, 0, new QTableWidgetItem(item.name));
        tableRecent_->setItem(row, 1, new QTableWidgetItem(item.path));
        tableRecent_->setItem(row, 2, new QTableWidgetItem(item.time));
    }
    tableRecent_->setMinimumHeight(220);
    recentLayout->addWidget(tableRecent_);

    vl->addWidget(recentFrame);

    vl->addStretch();

    stack_->addWidget(pageWelcome_);
}

void CTViewer::buildSlicesPage()
{
    pageSlices_ = new QWidget(stack_);
    auto grid = new QGridLayout(pageSlices_);
    grid->setContentsMargins(6, 6, 6, 6);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);

#if USE_VTK
    viewAxial_ = new QVTKOpenGLNativeWidget(pageSlices_);
    viewSagittal_ = new QVTKOpenGLNativeWidget(pageSlices_);
    viewCoronal_ = new QVTKOpenGLNativeWidget(pageSlices_);
#else
    auto makePlaceholder = [this](const QString& name) {
        auto w = new QWidget(pageSlices_);
        w->setObjectName(name);
        w->setStyleSheet("background:#111; border:1px solid #222;");
        return w;
        };
    viewAxial_ = makePlaceholder("viewAxial");
    viewSagittal_ = makePlaceholder("viewSagittal");
    viewCoronal_ = makePlaceholder("viewCoronal");
#endif

    auto placeholder = new QWidget(pageSlices_);
    placeholder->setStyleSheet("background:#1a1a1a; border:1px dashed #333;");

    grid->addWidget(viewAxial_, 0, 0);
    grid->addWidget(viewSagittal_, 1, 0);
    grid->addWidget(viewCoronal_, 0, 1);
    grid->addWidget(placeholder, 1, 1);

    stack_->addWidget(pageSlices_);
}

void CTViewer::buildNavDock()
{
    // 创建 DockWidget，隐藏标题栏，固定左侧，禁止拖动
    dockNav_ = new QDockWidget(QStringLiteral("欢迎使用"), this);
    dockNav_->setObjectName("navDock");
    dockNav_->setAllowedAreas(Qt::LeftDockWidgetArea);
    dockNav_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dockNav_->setTitleBarWidget(new QWidget());

    // Dock 样式
    dockNav_->setStyleSheet(R"(
        QDockWidget {
            border: none;
            background-color: #222;
        }
    )");

    // 去除主窗口的 separator（边缝）
    this->setStyleSheet(this->styleSheet() + R"(
        QMainWindow::separator {
            width: 0px;
            height: 0px;
            background: transparent;
        }
    )");

    // Dock 内容容器
    auto w = new QWidget(dockNav_);
    auto v = new QVBoxLayout(w);
    v->setContentsMargins(5, 0, 0, 0);
    v->setSpacing(1);

    // 列表控件
    listNav_ = new QListWidget(w);
    listNav_->setFrameShape(QFrame::NoFrame);
    listNav_->setMinimumWidth(180);
    listNav_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listNav_->setStyleSheet(R"(
        QListWidget {
            background: #222;
            color: #ddd;
            border: none;
        }
        QListWidget::item {
            height: 30px;
        }
        QListWidget::item:selected {
            background: #444;
            color: #fff;
        }
    )");

    // === 辅助函数：添加文字项 ===
    auto addItem = [this](const QString& text) {
        auto item = new QListWidgetItem(text);
        item->setSizeHint(QSize(180, 30));
        listNav_->addItem(item);
        };

    // === 辅助函数：添加分割线 ===
    auto addSeparator = [this]() {
        auto sepItem = new QListWidgetItem();
        sepItem->setFlags(Qt::NoItemFlags);
        sepItem->setSizeHint(QSize(180, 12));
        listNav_->addItem(sepItem);

        auto line = new QFrame(listNav_);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setStyleSheet("color: #444; background: #444; margin: 6px 12px;");
        listNav_->setItemWidget(sepItem, line);
        };

    // === 分组添加条目 ===

    // 文件组
    addItem(QStringLiteral("新建"));
    addItem(QStringLiteral("打开"));
    addItem(QStringLiteral("保存"));
    addItem(QStringLiteral("另存为"));
    addItem(QStringLiteral("打包"));
    addItem(QStringLiteral("导出为mvgl"));
    addSeparator();

    // 数据操作组
    addItem(QStringLiteral("快速导入"));
    addItem(QStringLiteral("CT重建"));
    addItem(QStringLiteral("导入"));
    addItem(QStringLiteral("导出"));
    addSeparator();

    // 对象保存组
    addItem(QStringLiteral("合并对象"));
    addItem(QStringLiteral("保存对象"));
    addItem(QStringLiteral("保存图像"));
    addItem(QStringLiteral("保存影像/图像堆栈"));
    addSeparator();

    // 工具设置组
    addItem(QStringLiteral("批处理"));
    addItem(QStringLiteral("首选项"));
    addSeparator();

    // 退出
    addItem(QStringLiteral("退出"));

    // 加入布局
    v->addWidget(listNav_);
    dockNav_->setWidget(w);
    addDockWidget(Qt::LeftDockWidgetArea, dockNav_);
}




void CTViewer::wireSignals()
{
    // 左侧导航切页
    connect(listNav_, &QListWidget::itemClicked, this,
        [this](QListWidgetItem* it) {
            const QString t = it ? it->text() : QString();
            if (t == QStringLiteral("开始")) {
                stack_->setCurrentWidget(pageWelcome_);
            }
            else if (t == QStringLiteral("打开")
                || t == QStringLiteral("导入")
                || t == QStringLiteral("CT 重建")) {
                stack_->setCurrentWidget(pageSlices_);
            }
            else if (t == QStringLiteral("退出")) {
                close();
            }
        });

    // 欢迎页上的快捷按钮，统一跳转到切片浏览页并给出状态栏提示。
    const auto goToSlices = [this](const QString& message) {
        stack_->setCurrentWidget(pageSlices_);
        statusBar()->showMessage(message, 1500);
        };
    connect(btnVisCheck_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("进入视觉检查模块")); });
    connect(btnPorosity_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("进入孔隙度分析模块")); });
    connect(btnMetrology_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("进入计量模块")); });
    connect(btnMaterial_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("进入材料分析模块")); });
    connect(tableRecent_, &QTableWidget::itemDoubleClicked, this,
        [goToSlices](QTableWidgetItem* item) {
            const QString projectName = item ? item->text() : QStringLiteral("项目");
            goToSlices(QStringLiteral("正在打开 %1 ...").arg(projectName));
        });

    // 顶栏按钮点击后在状态栏提示当前操作，模拟撤回/保持逻辑。
    connect(btnUndo_, &QPushButton::clicked, this, [this]() {
        statusBar()->showMessage(QStringLiteral("已执行撤回操作"), 1500);
        });
    connect(btnKeep_, &QPushButton::clicked, this, [this]() {
        statusBar()->showMessage(QStringLiteral("保持当前更改"), 1500);
        });

}

void CTViewer::setDefaults()
{
    // 欢迎页为默认显示，确保导航列表同步选中第一个条目。
    if (listNav_) {
        // 这里通过默认选中第一项，确保导航栏与当前页面一致。
        listNav_->setCurrentRow(0);
    }
}

#if USE_VTK
void CTViewer::setupVTKViews()
{
    auto setup = [](QVTKOpenGLNativeWidget* w) {
        auto rw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        auto ren = vtkSmartPointer<vtkRenderer>::New();
        ren->SetBackground(0.08, 0.08, 0.10);
        rw->AddRenderer(ren);
        w->setRenderWindow(rw);
        };
    setup(viewAxial_);
    setup(viewSagittal_);
    setup(viewCoronal_);
}
#endif