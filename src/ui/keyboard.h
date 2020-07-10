#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class KeyBoard;
}

class KeyBoard : public QWidget
{
    Q_OBJECT

public:
    explicit KeyBoard(QString text, QWidget *parent = 0);
    ~KeyBoard();

signals:
    void sigBack(); //返回
    void sigConfirmInput(QString input); //确认输入

private slots:
    void onBtnPressed(); //按键按下
    void onBtnReleased(); //按键松开
    void validInput();  //使输入有效
    void cursorFliker(); //输入图标闪烁
    void on_button_clear_pressed(); //清空按键按下
    void on_button_clear_released(); //清空按键松开

private:
    void initForm(); //初始化窗体数据
    void updateCap(); //大小写切换
    void clearInput(); //清空输入
    void inputCharacter(QString msg); //输入字母
    void inputNumber(QString msg); //输入数字
    void convertKeyboard(); //切换数字字母键盘
    void backSpace(); //回删
    void confirmInput(); //
    void updateButton(QPushButton *btn);
    void updateClearButtonState();

private:
    Ui::KeyBoard *ui;

    QList<QPushButton *> m_buttons; //字母按键
    QList<QPushButton *> m_numButtons; //数字按键
    QPushButton *m_curButton; //当前按下的

    QString     m_inputText; //有效的输入文本
    QString     m_tempText; //临时输入文本

    QTimer     *m_timer; //有效计时器
    QTimer     *m_flikerTimer; //闪烁计时器

    int         m_count;

    bool        m_isCap; //大写
    bool        m_cursorShow; //输入标识显示
    bool        m_isLetter; //字母键盘

};

#endif // KEYBOARD_H
