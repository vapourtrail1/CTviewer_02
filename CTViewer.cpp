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

    auto vl = new QVBoxLayout(pageWelcome_);
    vl->setContentsMargins(18, 18, 18, 18);
    vl->setSpacing(16);

    auto lbl = new QLabel(QStringLiteral("欢迎使用Viewer_demo"), pageWelcome_);
    lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    lbl->setStyleSheet("font-size:20px; font-weight:600;");
    vl->addWidget(lbl);

    // 四个大按钮
    auto grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    btnVisCheck_ = new QPushButton(QStringLiteral("视觉检查"));
    btnPorosity_ = new QPushButton(QStringLiteral("孔隙度"));
    btnMetrology_ = new QPushButton(QStringLiteral("计量"));
    btnMaterial_ = new QPushButton(QStringLiteral("材料"));
    for (auto* b : { btnVisCheck_.data(), btnPorosity_.data(), btnMetrology_.data(), btnMaterial_.data() }) {
        b->setMinimumSize(160, 60);
        b->setStyleSheet("font-size:16px; padding:12px;");
    }
    grid->addWidget(btnVisCheck_, 0, 0);
    grid->addWidget(btnPorosity_, 0, 1);
    grid->addWidget(btnMetrology_, 0, 2);
    grid->addWidget(btnMaterial_, 0, 3);
    vl->addLayout(grid);

    auto hint = new QLabel(QStringLiteral("选择模块或左侧导航以开始"), pageWelcome_);
    hint->setAlignment(Qt::AlignHCenter);
    hint->setStyleSheet("color:#888;");
    vl->addWidget(hint);

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
