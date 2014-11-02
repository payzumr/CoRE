#ifndef IMAGEINFODLG_H
#define IMAGEINFODLG_H

#include <QDialog>
#include "ui_imageinfodlg.h"
#include <uEye.h>

class Mainview;

/*! \brief imageinfo dialog class declaration */
class imageinfodlg : public QDialog
{
    Q_OBJECT

public:
    /*! \brief std constructor */
    imageinfodlg(QWidget *parent = 0);
    /*! \brief std destructor */
    ~imageinfodlg();
    /*!\brief clears the dialog image info controls */
    void ClearControls ();

public slots:
    /*! \brief resets the dialogs counter control values */
    void ResetCounters();
    /*! \brief updates the dialogs counter control with current values */
    void UpdateCounters();
    /*! \brief updates the dialogs image parameter controls
     *  \param pImageinfo pointer to a imageinfo struct */
    void UpdateControls(UEYEIMAGEINFO *pImageinfo);

signals:
    /*! \brief close signal for the parent */
    void close();

protected:
    /*! \brief overloads parents 'closeEvent' function
     *  \param e pointer to Qt close event */
    void closeEvent (QCloseEvent * e);
    /*! \brief overloads parents 'showEvent' function
     *  \param e pointer to Qt show event */
    void showEvent (QShowEvent * e);

private:
    /*! \brief dialogs GUI member class, build by Qt */
    Ui::imageinfodlgClass ui;
    /*! \brief MainView parent ref */
    Mainview* m_pParent;
};

#endif // IMAGEINFODLG_H
