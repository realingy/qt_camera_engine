#include "itemlabel.h"
#include "ui_itemlabel.h"
#include <QPainter>
#include <QString>
#include <QDebug>

/*!
 * \brief ItemLabel::ItemLabel
 * \param pixmap 图片
 * \param file 文件名
 * \param size 尺寸
 * \param parent
 * 预览界面的元素控件,显示文件的缩略图和选中图标以及播放标识
 */
ItemLabel::ItemLabel(const QPixmap pixmap, const QString file, QSize size, QWidget*parent)
    : QLabel(parent)
    , ui(new Ui::ItemLabel)
    , m_pixmap(pixmap)
    , m_name(file)
{
    ui->setupUi(this);
    this->resize(size);

    QString name = file;
    name.chop(4);
    ui->label->setText(name);
    ui->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_pixmap = m_pixmap.scaled(width(), height());
    ui->label_select->hide();
    update();
}

ItemLabel::~ItemLabel()
{
    delete ui;
}

void ItemLabel::setSelected()
{
    ui->label_select->show();
}

void ItemLabel::unSelected()
{
    ui->label_select->hide();
}

void ItemLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing );
    painter.setRenderHint(QPainter::SmoothPixmapTransform );

    QPoint orgPoint;
    orgPoint.setX(width()/2);
    orgPoint.setY(height()/2);
    QPoint center(m_pixmap.width()/2,m_pixmap.height()/2);
    painter.translate(orgPoint - center);

    painter.drawPixmap(0, 0, m_pixmap.width(), m_pixmap.height(), m_pixmap);
}
