#include "keyboard.h"
#include "configmanager.h"
#include "ui_keyboard.h"
#include <QPushButton>
#include <QCoreApplication>
#include <QStackedWidget>
#include <QDebug>
#include <QTimer>

//字符串矩阵，用于大小写字母切换
QString Letter[][3] =
{
     //objectName               小写                   大写
      "button_2",              "abc",                 "ABC",
      "button_3",              "def",                 "DEF",
      "button_4",              "ghi",                 "GHI",
      "button_5",              "jkl",                 "JKL",
      "button_6",              "mno",                 "MNO",
      "button_7",              "pqrs",                "PQRS",
      "button_8",              "tuv",                 "TUV",
      "button_9",              "wxyz",                "WXYZ",
};

//数字矩阵，用于字母数字切换
QString Number[][2] =
{
     //objectName               //
      "button_1",               "1",
      "button_2",               "2",
      "button_3",               "3",
      "button_4",               "4",
      "button_5",               "5",
      "button_6",               "6",
      "button_7",               "7",
      "button_8",               "8",
      "button_9",               "9",
      "button_shift",           "abc",
};

KeyBoard::KeyBoard(QString text, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KeyBoard)
    , m_isCap(false) //小写
    , m_count(0) //计次
    , m_inputText(text) //初始化有效文本
    , m_curButton(NULL) //当前按钮
    , m_cursorShow(false) //闪烁标识不显示
    , m_isLetter(true) //初始状态为字母键盘
{
    ui->setupUi(this);
    ui->lineEdit->setText(m_inputText+QString("_"));
    ui->lineEdit->setEnabled(false);
    ui->lineEdit->clearFocus();

    initForm(); //初始化窗口

    m_timer = new QTimer();
    m_timer->setSingleShot(true);
    m_timer->setInterval(600);
    connect(m_timer, SIGNAL(timeout()), SLOT(validInput())); //600毫秒不继续点击算一次有效输入

    m_flikerTimer = new QTimer();
    m_flikerTimer->setSingleShot(false);
    m_flikerTimer->setInterval(500);
    m_flikerTimer->start();
    connect(m_flikerTimer, SIGNAL(timeout()), SLOT(cursorFliker())); //闪烁标识500毫秒闪烁一次

    m_numButtons << ui->button_1
                 << ui->button_2
                 << ui->button_3
                 << ui->button_4
                 << ui->button_5
                 << ui->button_6
                 << ui->button_7
                 << ui->button_8
                 << ui->button_9
                 << ui->button_shift;

}

KeyBoard::~KeyBoard()
{
    delete m_curButton;
    delete ui;
}

/*!
 * \brief KeyBoard::onBtnPressed 按键按下背景变色
 */
void KeyBoard::onBtnPressed()
{
    QPushButton *btn = (QPushButton *)sender();
    QString  objectName = btn->objectName();
    if(objectName != "button_clear")
    {
        btn->setStyleSheet(QString("background-color: #80cef9"));
    }
}

void KeyBoard::onBtnReleased()
{
    QPushButton *btn = (QPushButton *)sender();
    QString  objectName = btn->objectName();
    if(objectName != "button_clear")
    {
        btn->setStyleSheet(QString("background-color: #d9edf9"));
    }

    if(objectName == "button_cancel")
    {
        emit sigBack();
    }
    else if("button_backSpace" == objectName)
    {
        validInput(); //删除之前将临时文本有效化
        backSpace(); //回删
    }
    else if("button_shift" == objectName)
    {
        m_isLetter = !m_isLetter; //字母键盘
        convertKeyboard(); //字母数字键盘切换
    }
    else if("button_space" == objectName)
    {
        inputCharacter(" "); //输入空格
    }
    else if("button_cap" == objectName)
    {
        if(m_isLetter)
        {
            m_isCap = !m_isCap;
            updateCap(); //字母键盘状态下切换大小写
        }
        else
        {
            inputNumber(QString("0")); //数字键盘模式当做0键盘使用
        }
    }
    else if("button_confirm" == objectName)
    {
        confirmInput(); //确认输入
    }
    else
    {
        //字符按键
        if(m_isLetter)
        {
            //符号键
            if(objectName == "button_1")
            {
                updateButton(btn);

                inputCharacter(QString(".,!@#$%&*/()_-+=\"':;?<>[]{}^`|\\~"));
                return;
            }

            for(int i=0; i<8; i++)
            {
                //字母键，如果当前的按键变了，之前的输入就算有效，否则就开启有效计时器等待
                if(objectName == Letter[i])
                {
                    updateButton(btn);

                    inputCharacter(btn->text());
                    break;
                }
            }
        }
        else
        {
            //输入数字
            for(int i=0; i<9; i++)
            {
                if(objectName == Number[i])
                {
                    inputNumber(btn->text());
                    break;
                }
            }
        }
    }
}

void KeyBoard::validInput()
{
    m_count = 0;
    m_inputText += m_tempText;
    m_tempText = QString::null;
    m_flikerTimer->start();
    updateClearButtonState(); //
}

void KeyBoard::cursorFliker()
{
    m_cursorShow = !m_cursorShow;
    if(m_cursorShow)
    {
        ui->lineEdit->setText(m_inputText+QString("_"));
    }
    else
    {
        ui->lineEdit->setText(m_inputText);
    }
}

//初始化界面，并连接所有的按钮点击槽函数
void KeyBoard::initForm()
{
    QList<QPushButton *> buttons = findChildren<QPushButton *>();
    m_buttons = buttons;
    foreach (QPushButton * btn, buttons) {
        connect(btn, SIGNAL(pressed()), SLOT(onBtnPressed()));
        connect(btn, SIGNAL(released()), SLOT(onBtnReleased()));
    }
    updateCap();
    updateClearButtonState(); //
}

//更新字母大小写状态
void KeyBoard::updateCap()
{
    QString objectName;
    foreach (QPushButton * button, m_buttons)
    {
        objectName = button->objectName();

        for(int i = 0; i < 8; i++)
        {
            if(objectName == Letter[i][0])
            {
                if(true == m_isCap)
                {
                    //大写
                    ui->button_cap->setStyleSheet(QString("image: url(:/res/icons/ic_cap_true.png);"));
                    button->setText(Letter[i][2]);
                }
                else
                {
                    //小写
                    ui->button_cap->setStyleSheet(QString("image: url(:/res/icons/ic_cap_false.png);"));
                    button->setText(Letter[i][1]);
                }
            }
        }
    }
}

void KeyBoard::clearInput()
{
    m_tempText = QString::null;
    m_inputText = QString::null;
    ui->lineEdit->setText(m_inputText);
    updateClearButtonState();
}

void KeyBoard::inputCharacter(QString msg)
{
    m_flikerTimer->stop();
    int sum = msg.count();
    int index = (0 == m_count%sum) ? (sum) : (m_count%sum);
    m_tempText = msg.left(index).right(1);
    ui->lineEdit->setText(m_inputText + m_tempText);
    updateClearButtonState();
    m_timer->start();
}

void KeyBoard::inputNumber(QString msg)
{
    m_flikerTimer->stop();
    m_inputText.append(msg);
    ui->lineEdit->setText(m_inputText);
    updateClearButtonState();
    m_flikerTimer->start();
}

void KeyBoard::convertKeyboard()
{
    if(false == m_isLetter)
    {
        ui->button_cap->setStyleSheet(QString("image: url(:/res/icons/ic_background.png);"));
        ui->button_cap->setText(QString("0"));
        QString objectName;
        foreach (QPushButton *button, m_numButtons)
        {
            objectName = button->objectName();
            for(int i = 0; i < 10; i++)
            {
                if(objectName == Number[i][0])
                {
                    button->setText(Number[i][1]);
                    break;
                }
            }
        }
    }
    else
    {
        m_isCap = false;
        ui->button_cap->setText(QString(""));
        ui->button_1->setText(QString(".,"));
        ui->button_shift->setText(QString("123"));
        updateCap();
    }
}

void KeyBoard::backSpace()
{
    QString text = ui->lineEdit->text();
    if(!text.isEmpty())
    {
        if(text.endsWith(QString("_")))
        {
            text.chop(2);
        }
        else
        {
            text.chop(1);
        }
        ui->lineEdit->setText(text);
        m_inputText = text;
        m_tempText = QString::null;
        updateClearButtonState();
    }
}

void KeyBoard::confirmInput()
{
    validInput();

    emit sigConfirmInput(m_inputText);
    emit sigBack();
}

void KeyBoard::updateButton(QPushButton *btn)
{
    if(m_curButton != btn)
    {
        validInput();
        m_curButton = btn;
    }

    if(m_timer->isActive())
        m_timer->stop();
    m_count++;
}

void KeyBoard::updateClearButtonState()
{
    if(m_inputText.isEmpty() && m_tempText.isEmpty())
    {
        ui->button_clear->setStyleSheet(QString("image: url(:/res/icons/ic_clear_1.png);"));
    }
    else
    {
        ui->button_clear->setStyleSheet(QString("image: url(:/res/icons/ic_clear_2.png);"));
    }
}

void KeyBoard::on_button_clear_pressed()
{
    ui->button_clear->setStyleSheet(QString("image: url(:/res/icons/ic_clear_1.png);"));
}

void KeyBoard::on_button_clear_released()
{
    clearInput();
}
