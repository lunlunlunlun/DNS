/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.1
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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAaaaa;
    QWidget *centralWidget;
    QLabel *label;
    QPushButton *pushButton;
    QLineEdit *lineEdit;
    QLabel *label_3;
    QLabel *label_4;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QTextEdit *textEdit;
    QTextEdit *textEdit_2;
    QLabel *label_2;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(915, 421);
        MainWindow->setStyleSheet(QStringLiteral("background-image: url(:/2345_image_file_copy_11.jpg);"));
        actionAaaaa = new QAction(MainWindow);
        actionAaaaa->setObjectName(QStringLiteral("actionAaaaa"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 30, 151, 51));
        label->setStyleSheet(QString::fromUtf8("background:transparent;\n"
"font: 9pt \"\346\226\271\346\255\243\350\210\222\344\275\223\";"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(310, 190, 141, 51));
        pushButton->setStyleSheet(QString::fromUtf8("border-image: url(:/2345_image_file_copy_5_WPS\345\233\276\347\211\207.jpg);"));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(200, 40, 251, 41));
        lineEdit->setStyleSheet(QStringLiteral("border-image: url(:/2345_image_file_copy_12.jpg);"));
        lineEdit->setMaxLength(255);
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(30, 240, 151, 41));
        label_3->setStyleSheet(QString::fromUtf8("background:transparent;font: 9pt \"\346\226\271\346\255\243\350\210\222\344\275\223\";"));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(30, 130, 151, 51));
        label_4->setStyleSheet(QString::fromUtf8("background:transparent;font: 9pt \"\346\226\271\346\255\243\350\210\222\344\275\223\";"));
        radioButton = new QRadioButton(centralWidget);
        radioButton->setObjectName(QStringLiteral("radioButton"));
        radioButton->setGeometry(QRect(210, 130, 121, 51));
        radioButton->setStyleSheet(QString::fromUtf8("background:transparent;font: 9pt \"\351\232\266\344\271\246\";"));
        radioButton_2 = new QRadioButton(centralWidget);
        radioButton_2->setObjectName(QStringLiteral("radioButton_2"));
        radioButton_2->setGeometry(QRect(340, 130, 111, 51));
        radioButton_2->setStyleSheet(QString::fromUtf8("background:transparent;font: 9pt \"\351\232\266\344\271\246\";"));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(40, 300, 411, 71));
        textEdit->setStyleSheet(QStringLiteral("border-image: url(:/2345_image_file_copy_12.jpg);"));
        textEdit_2 = new QTextEdit(centralWidget);
        textEdit_2->setObjectName(QStringLiteral("textEdit_2"));
        textEdit_2->setGeometry(QRect(540, 30, 331, 341));
        textEdit_2->setStyleSheet(QStringLiteral("border-image: url(:/2345_image_file_copy_12.jpg);"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(760, 60, 91, 91));
        label_2->setStyleSheet(QStringLiteral("border-image: url(:/2345_image_file_copy_7.jpg);"));
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionAaaaa->setText(QApplication::translate("MainWindow", "aaaaa", 0));
        label->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:20pt;\">\346\237\245\350\257\242\345\237\237\345\220\215\357\274\232</span></p></body></html>", 0));
        pushButton->setText(QString());
        lineEdit->setInputMask(QString());
        label_3->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:20pt;\">\346\237\245\350\257\242\347\273\223\346\236\234\357\274\232</span></p></body></html>", 0));
        label_4->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:20pt;\">\346\237\245\350\257\242\346\226\271\345\274\217\357\274\232</span></p></body></html>", 0));
        radioButton->setText(QApplication::translate("MainWindow", "\350\277\255\344\273\243\346\237\245\350\257\242", 0));
        radioButton_2->setText(QApplication::translate("MainWindow", "\345\244\215\345\220\210\346\237\245\350\257\242", 0));
        label_2->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
