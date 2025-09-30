#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QPoint>

class QToolButton;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QWidget;
class QLabel;
class QPushButton;
class QEvent;
class QFrame;
class QTableWidget;
class QDockWidget;


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
    // ---- 自定义标题栏 ----
    QPointer<QWidget> titleBar_;
    QPointer<QWidget> titleLeftArea_;
    QPointer<QWidget> titleCenterArea_;
    QPointer<QLabel> titleLabel_;
    QPointer<QToolButton> btnTitleUndo_;
    QPointer<QToolButton> btnTitleUndo02_;
    QPointer<QToolButton> btnMinimize_;
    QPointer<QToolButton> btnMaximize_;
    QPointer<QToolButton> btnClose_;
    bool draggingWindow_ = false;
    QPoint dragOffset_;

private:
    //
    QPointer<QStackedWidget> stack_;
   
    QPointer<QWidget> pageWelcome_;
    QPointer<QPushButton> btnVisCheck_;
    QPointer<QPushButton> btnPorosity_;
    QPointer<QPushButton> btnMetrology_;
    QPointer<QPushButton> btnMaterial_;
    // 欢迎页顶栏的「撤回 / 不撤回」按钮
    QPointer<QPushButton> btnUndo_;
    QPointer<QPushButton> btnKeep_;
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

    
    QPointer<QDockWidget> dockNav_;
    QPointer<QListWidget> listNav_;

private:
    
    void buildTitleBar();
    void updateMaximizeButtonIcon();

protected:
    // ---- 事件处理：维护标题栏拖拽、双击等行为 ----
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    
    void buildCentral();
    void buildWelcomePage();
    void buildSlicesPage();
    void buildNavDock();
    void wireSignals();
    void setDefaults();

#if USE_VTK
    void setupVTKViews(); 
#endif
};