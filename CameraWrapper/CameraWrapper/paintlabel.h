#ifndef PAINTLABEL_H
#define PAINTLABEL_H

#include <QLabel>
#include <QImage>
#include <QPaintEvent>

class PaintLabel : public QLabel
{
    Q_OBJECT
public:
    explicit PaintLabel(QWidget *parent = 0);

    /*! Draws an QImage on the label */
    void setImage(QImage &image);

    /*! Draws an QImage on the label */
    void setImage(QImage &image, double scaleFactor = 1.0, bool adjustSize = false);

    /*! Draws an pixmap on the label */
    void setPixmap(const QPixmap &pixmap);

    /*! Adjusts the size of the widget to fit its contents.*/
    void adjustSize();

    /*! This property holds whether the label will scale its contents to fill all available space. */
    void setScaledContents(bool scaled);

    /*! This property holds whether the label will scale its contents to fill all available space. */
    bool scaledContents();

    /*! scales the current image and paints it */
    void scale(double factor);

signals:

public slots:
    /*! Clears the previous paintings */
    void clear();

protected:
    void paintEvent(QPaintEvent *ev);

    QImage m_Image;
    bool m_bScaledContent;
};

#endif // PAINTLABEL_H
