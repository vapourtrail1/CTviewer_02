#include "CTViewer.h"
#include <QApplication>
#include <QDockWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QStatusBar>
#include <QFrame>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>

#if USE_VTK
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#endif

CTViewer::CTViewer(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("VGStudio-Lite"));
    resize(1280, 820);

    // 应用简易的深色调色板，让整体观感更贴近截图中的产品风格。
    setStyleSheet(QStringLiteral(
        "QMainWindow{background-color:#121212;}"
        "QDockWidget{background-color:#1a1a1a;color:#f0f0f0;}"
        "QMenuBar, QStatusBar{background-color:#1a1a1a;color:#e0e0e0;}"));

    buildCentral();
    buildNavDock();
    buildPropDock();
    wireSignals();
    setDefaults();

#if USE_VTK
    setupVTKViews();
#endif

    statusBar()->showMessage(QStringLiteral("就绪"));
}

CTViewer::~CTViewer() = default;

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

    // 顶部操作条：设置黑色背景并在中间显示产品名称，同时加入两个操作按钮。
    auto topBar = new QFrame(pageWelcome_);
    topBar->setObjectName(QStringLiteral("topBar"));
    topBar->setStyleSheet(QStringLiteral(
        "QFrame#topBar{background:#202020; border-radius:10px;}"
        "QFrame#topBar QLabel{color:#f5f5f5; font-size:16px; font-weight:600;}"
        "QFrame#topBar QPushButton{background:#2f2f2f; color:#f5f5f5; border:1px solid #3c3c3c;"
        " border-radius:6px; padding:8px 18px;}"
        "QFrame#topBar QPushButton:hover{background:#3a3a3a; border-color:#4d6fff;}"));
    auto topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 10, 20, 10);
    topLayout->setSpacing(12);

    btnUndo_ = new QPushButton(QStringLiteral("撤回"), topBar);
    btnUndo_->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(btnUndo_);

    topLayout->addStretch();

    auto productLabel = new QLabel(QStringLiteral("VGStudio-Lite"), topBar);
    productLabel->setAlignment(Qt::AlignCenter);
    topLayout->addWidget(productLabel);

    topLayout->addStretch();

    btnKeep_ = new QPushButton(QStringLiteral("不撤回"), topBar);
    btnKeep_->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(btnKeep_);

    vl->addWidget(topBar);

    // 顶部横幅，显示产品名称与版本信息。
    auto banner = new QFrame(pageWelcome_);
    banner->setObjectName(QStringLiteral("heroBanner"));
    banner->setStyleSheet(QStringLiteral(
        "QFrame#heroBanner{background:#262626; border-radius:10px;}"
        "QFrame#heroBanner QLabel{color:#f9f9f9;}"));
    auto bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(20, 16, 20, 16);
    bannerLayout->setSpacing(8);
    auto title = new QLabel(QStringLiteral("欢迎使用 VGSTUDIO MAX 2024.4 64 bit"), banner);
    title->setStyleSheet(QStringLiteral("font-size:24px; font-weight:700;"));
    bannerLayout->addWidget(title);
    auto subtitle = new QLabel(QStringLiteral("继续最近项目，或通过下方模块快速开始您的工业 CT 工作流程。"), banner);
    subtitle->setStyleSheet(QStringLiteral("font-size:14px; color:#bbbbbb;"));
    subtitle->setWordWrap(true);
    bannerLayout->addWidget(subtitle);
    vl->addWidget(banner);

    // 快速开始与提示区域，采用左右布局提升信息密度。
    auto quickRow = new QHBoxLayout();
    quickRow->setSpacing(16);

    auto quickFrame = new QFrame(pageWelcome_);
    quickFrame->setObjectName(QStringLiteral("quickActions"));
    quickFrame->setStyleSheet(QStringLiteral(
        "QFrame#quickActions{background:#202020; border-radius:10px;}"
        "QFrame#quickActions QLabel{color:#f5f5f5;}"
        "QFrame#quickActions QPushButton{background:#2f2f2f; color:#f5f5f5; border:1px solid #3c3c3c;"
        " border-radius:6px; padding:10px 16px; font-size:15px;}"
        "QFrame#quickActions QPushButton:hover{background:#3a3a3a; border-color:#4d6fff;}"));
    auto quickLayout = new QVBoxLayout(quickFrame);
    quickLayout->setContentsMargins(20, 18, 20, 18);
    quickLayout->setSpacing(12);
    auto quickTitle = new QLabel(QStringLiteral("快速开始"), quickFrame);
    quickTitle->setStyleSheet(QStringLiteral("font-size:16px; font-weight:600;"));
    quickLayout->addWidget(quickTitle);

    // 三个快捷按钮，分别对应打开、创建和加载示例工程。
    btnOpenFile_ = new QPushButton(QStringLiteral("打开数据集"), quickFrame);
    btnOpenFile_->setCursor(Qt::PointingHandCursor);
    quickLayout->addWidget(btnOpenFile_);

    btnCreateProject_ = new QPushButton(QStringLiteral("新建重建项目"), quickFrame);
    btnCreateProject_->setCursor(Qt::PointingHandCursor);
    quickLayout->addWidget(btnCreateProject_);

    btnLoadDemo_ = new QPushButton(QStringLiteral("加载示例数据"), quickFrame);
    btnLoadDemo_->setCursor(Qt::PointingHandCursor);
    quickLayout->addWidget(btnLoadDemo_);

    quickLayout->addStretch();

    quickRow->addWidget(quickFrame, 2);

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
    auto tips = new QLabel(QStringLiteral("• 支持加载 VGProject (*.vgl) 与 VGArchive (*.vgi) 项目。\n"
        "• 可直接导入 DICOM、TIFF、RAW 等常见工业 CT 数据。\n"
        "• 若需培训资料，可访问帮助中心以获取最新教程。"), tipsFrame);
    tips->setWordWrap(true);
    tips->setStyleSheet(QStringLiteral("font-size:13px; line-height:20px;"));
    tipsLayout->addWidget(tips);
    tipsLayout->addStretch();

    quickRow->addWidget(tipsFrame, 3);

    vl->addLayout(quickRow);

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
    auto moduleTitle = new QLabel(QStringLiteral("核心模块"), moduleFrame);
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
    dockNav_ = new QDockWidget(QStringLiteral("欢迎"), this);
    dockNav_->setObjectName("navDock");
    auto w = new QWidget(dockNav_);
    auto v = new QVBoxLayout(w);
    v->setContentsMargins(6, 6, 6, 6);
    listNav_ = new QListWidget(w);
    listNav_->setMinimumWidth(180);
    listNav_->setStyleSheet("QListWidget{background:#222;color:#ddd;} QListWidget::item{height:30px;}");
    for (auto s : { QStringLiteral("开始"), QStringLiteral("打开"), QStringLiteral("保存"),
                   QStringLiteral("导入"), QStringLiteral("导出"), QStringLiteral("CT 重建"),
                   QStringLiteral("帮助"), QStringLiteral("退出") }) {
        listNav_->addItem(s);
    }
    v->addWidget(listNav_);
    dockNav_->setWidget(w);
    addDockWidget(Qt::LeftDockWidgetArea, dockNav_);
}

void CTViewer::buildPropDock()
{
    dockProp_ = new QDockWidget(QStringLiteral("渲染调整"), this);
    dockProp_->setObjectName("propDock");
    auto w = new QWidget(dockProp_);
    auto form = new QFormLayout(w);
    form->setContentsMargins(8, 8, 8, 8);
    form->setSpacing(6);

    // WW
    auto hWW = new QHBoxLayout();
    sliderWW_ = new QSlider(Qt::Horizontal, w);
    dsbWW_ = new QDoubleSpinBox(w);
    dsbWW_->setDecimals(1);
    dsbWW_->setMaximum(5000.0);
    hWW->addWidget(sliderWW_);
    hWW->addWidget(dsbWW_);
    form->addRow(QStringLiteral("窗宽"), hWW);

    // WL
    auto hWL = new QHBoxLayout();
    sliderWL_ = new QSlider(Qt::Horizontal, w);
    dsbWL_ = new QDoubleSpinBox(w);
    dsbWL_->setDecimals(1);
    dsbWL_->setRange(-2000.0, 2000.0);
    hWL->addWidget(sliderWL_);
    hWL->addWidget(dsbWL_);
    form->addRow(QStringLiteral("窗位"), hWL);

    // 厚切片
    sbSlab_ = new QSpinBox(w);
    sbSlab_->setRange(1, 50);
    form->addRow(QStringLiteral("厚切片(层)"), sbSlab_);

    // 采样
    cbInterp_ = new QComboBox(w);
    cbInterp_->addItems({ QStringLiteral("Linear"), QStringLiteral("Nearest") });
    form->addRow(QStringLiteral("采样"), cbInterp_);

    // 预设
    cbPreset_ = new QComboBox(w);
    cbPreset_->addItems({ QStringLiteral("Bone"), QStringLiteral("Soft"), QStringLiteral("Lung") });
    form->addRow(QStringLiteral("LUT 预设"), cbPreset_);

    // 重置
    btnReset_ = new QPushButton(QStringLiteral("重置"), w);
    form->addRow(QString(), btnReset_);

    dockProp_->setWidget(w);
    addDockWidget(Qt::RightDockWidgetArea, dockProp_);
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
    connect(btnOpenFile_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("打开数据集")); });
    connect(btnCreateProject_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("创建新的重建项目")); });
    connect(btnLoadDemo_, &QPushButton::clicked, this, [goToSlices]() { goToSlices(QStringLiteral("加载示例数据")); });
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

    // WW/WL 双向联动（UI 级）
    connect(sliderWW_, &QSlider::valueChanged, dsbWW_, &QDoubleSpinBox::setValue);
    connect(sliderWL_, &QSlider::valueChanged, dsbWL_, &QDoubleSpinBox::setValue);
    connect(dsbWW_, qOverload<double>(&QDoubleSpinBox::valueChanged),
        this, [this](double v) { sliderWW_->setValue(int(v)); });
    connect(dsbWL_, qOverload<double>(&QDoubleSpinBox::valueChanged),
        this, [this](double v) { sliderWL_->setValue(int(v)); });

    // 重置
    connect(btnReset_, &QPushButton::clicked, this, [this]() {
        dsbWW_->setValue(2000.0);
        dsbWL_->setValue(300.0);
        sbSlab_->setValue(1);
        cbInterp_->setCurrentText("Linear");
        cbPreset_->setCurrentText("Bone");
        statusBar()->showMessage(QStringLiteral("参数已重置"), 1500);
        });
}

void CTViewer::setDefaults()
{
    sliderWW_->setRange(1, 5000);
    sliderWL_->setRange(-2000, 2000);
    dsbWW_->setRange(1.0, 5000.0);
    dsbWL_->setRange(-2000.0, 2000.0);

    dsbWW_->setValue(2000.0);
    dsbWL_->setValue(300.0);
    sbSlab_->setValue(1);
    cbInterp_->setCurrentText(QStringLiteral("Linear"));
    cbPreset_->setCurrentText(QStringLiteral("Bone"));

    // 欢迎页为默认显示，确保导航列表同步选中第一个条目。
    if (listNav_) {
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
