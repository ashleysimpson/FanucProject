/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *streamingPushButton;
    QLabel *streamingLabel;
    QPushButton *pointsPushButton;
    QLabel *pointsLabel;
    QPushButton *checkConnectionPushButton;
    QLabel *checkConnectionLabel;
    QLabel *header;
    QLabel *instructions;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(457, 496);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        streamingPushButton = new QPushButton(centralWidget);
        streamingPushButton->setObjectName(QStringLiteral("streamingPushButton"));
        streamingPushButton->setGeometry(QRect(60, 350, 141, 71));
        streamingLabel = new QLabel(centralWidget);
        streamingLabel->setObjectName(QStringLiteral("streamingLabel"));
        streamingLabel->setGeometry(QRect(240, 370, 171, 31));
        pointsPushButton = new QPushButton(centralWidget);
        pointsPushButton->setObjectName(QStringLiteral("pointsPushButton"));
        pointsPushButton->setGeometry(QRect(60, 280, 141, 71));
        pointsLabel = new QLabel(centralWidget);
        pointsLabel->setObjectName(QStringLiteral("pointsLabel"));
        pointsLabel->setGeometry(QRect(240, 300, 171, 31));
        checkConnectionPushButton = new QPushButton(centralWidget);
        checkConnectionPushButton->setObjectName(QStringLiteral("checkConnectionPushButton"));
        checkConnectionPushButton->setGeometry(QRect(60, 230, 181, 51));
        checkConnectionLabel = new QLabel(centralWidget);
        checkConnectionLabel->setObjectName(QStringLiteral("checkConnectionLabel"));
        checkConnectionLabel->setGeometry(QRect(260, 240, 141, 31));
        header = new QLabel(centralWidget);
        header->setObjectName(QStringLiteral("header"));
        header->setGeometry(QRect(110, 20, 231, 31));
        instructions = new QLabel(centralWidget);
        instructions->setObjectName(QStringLiteral("instructions"));
        instructions->setGeometry(QRect(70, 60, 321, 151));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 457, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        streamingPushButton->setText(QApplication::translate("MainWindow", "Begin Streaming", 0));
        streamingLabel->setText(QApplication::translate("MainWindow", "Not Streaming", 0));
        pointsPushButton->setText(QApplication::translate("MainWindow", "Submit Points", 0));
        pointsLabel->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        checkConnectionPushButton->setText(QApplication::translate("MainWindow", "Check Connection", 0));
        checkConnectionLabel->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        header->setText(QApplication::translate("MainWindow", "Mac Side Connection", 0));
        instructions->setText(QApplication::translate("MainWindow", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
