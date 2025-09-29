#pragma once
#include <QMainWindow>
#include <QPointer>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QWidget;
class QLabel;
class QPushButton;
class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QDockWidget;
class QFormLayout;

// ---- 如果配好 VTK，把下面开关改成 1 并确认 include 路径正确 ----
#define USE_VTK 0

#if USE_VTK
class QVTKOpenGLNativeWidget;
#endif

class CTViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit CTViewer(QWidget* parent = nullptr);
    ~CTViewer();

private:
    // 中央区
    QPointer<QStackedWidget> stack_;
    // 欢迎页
    QPointer<QWidget> pageWelcome_;
    QPointer<QPushButton> btnVisCheck_;
    QPointer<QPushButton> btnPorosity_;
    QPointer<QPushButton> btnMetrology_;
    QPointer<QPushButton> btnMaterial_;
    // 三正交页
    QPointer<QWidget> pageSlices_;
#if USE_VTK
    QPointer<QVTKOpenGLNativeWidget> viewAxial_;
    QPointer<QVTKOpenGLNativeWidget> viewSagittal_;
    QPointer<QVTKOpenGLNativeWidget> viewCoronal_;
#else
    QPointer<QWidget> viewAxial_;
    QPointer<QWidget> viewSagittal_;
    QPointer<QWidget> viewCoronal_;
#endif

    // 左右 Dock
    QPointer<QDockWidget> dockNav_;
    QPointer<QListWidget> listNav_;
    QPointer<QDockWidget> dockProp_;

    // 渲染调整控件
    QPointer<QSlider> sliderWW_;
    QPointer<QDoubleSpinBox> dsbWW_;
    QPointer<QSlider> sliderWL_;
    QPointer<QDoubleSpinBox> dsbWL_;
    QPointer<QSpinBox> sbSlab_;
    QPointer<QComboBox> cbInterp_;
    QPointer<QComboBox> cbPreset_;
    QPointer<QPushButton> btnReset_;

private:
    // 构造子界面
    void buildCentral();
    void buildWelcomePage();
    void buildSlicesPage();
    void buildNavDock();
    void buildPropDock();
    void wireSignals();
    void setDefaults();

#if USE_VTK
    void setupVTKViews(); // 给三视图挂最小管线（黑底）
#endif
};
