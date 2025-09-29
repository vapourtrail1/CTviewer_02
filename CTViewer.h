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
class QFrame;
class QTableWidget;
class QDockWidget;
class QFormLayout;

// ----  VTK濪ظĳ 1 ȷ include ·ȷ ----
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
    // 
    QPointer<QStackedWidget> stack_;
    // ӭҳ
    QPointer<QWidget> pageWelcome_;
    QPointer<QPushButton> btnVisCheck_;
    QPointer<QPushButton> btnPorosity_;
    QPointer<QPushButton> btnMetrology_;
    QPointer<QPushButton> btnMaterial_;
    // 欢迎页顶部「快速开始」按钮
    QPointer<QPushButton> btnOpenFile_;
    QPointer<QPushButton> btnCreateProject_;
    QPointer<QPushButton> btnLoadDemo_;
    // 最近项目表格
    QPointer<QTableWidget> tableRecent_;
    // ҳ
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

    //  Dock
    QPointer<QDockWidget> dockNav_;
    QPointer<QListWidget> listNav_;
    QPointer<QDockWidget> dockProp_;

    // Ⱦؼ
    QPointer<QSlider> sliderWW_;
    QPointer<QDoubleSpinBox> dsbWW_;
    QPointer<QSlider> sliderWL_;
    QPointer<QDoubleSpinBox> dsbWL_;
    QPointer<QSpinBox> sbSlab_;
    QPointer<QComboBox> cbInterp_;
    QPointer<QComboBox> cbPreset_;
    QPointer<QPushButton> btnReset_;

private:
    // ӽ
    void buildCentral();
    void buildWelcomePage();
    void buildSlicesPage();
    void buildNavDock();
    void buildPropDock();
    void wireSignals();
    void setDefaults();

#if USE_VTK
    void setupVTKViews(); // ͼСߣڵף
#endif
};
