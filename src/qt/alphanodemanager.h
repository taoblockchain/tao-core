#ifndef ALPHANODEMANAGER_H
#define ALPHANODEMANAGER_H

#include "util.h"
#include "sync.h"

#include <QWidget>
#include <QTimer>

namespace Ui {
    class AlphanodeManager;

}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Alpha Node Manager page widget */
class AlphanodeManager : public QWidget
{
    Q_OBJECT

public:
    explicit AlphanodeManager(QWidget *parent = 0);
    ~AlphanodeManager();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);


public slots:
    void updateNodeList();
    void updateAdrenalineNode(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, QString status);
    void on_UpdateButton_clicked();

signals:

private:
    QTimer *timer;
    Ui::AlphanodeManager *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CCriticalSection cs_adrenaline;

private slots:
    void on_createButton_clicked();
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_tableWidget_2_itemSelectionChanged();
};
#endif // ALPHANODEMANAGER_H
