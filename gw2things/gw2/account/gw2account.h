#ifndef GW2ACCOUNT_H_F
#define GW2ACCOUNT_H_F

#include <QAbstractTableModel>
#include <QDateTime>

class GW2Account : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GW2Account(QObject *parent = nullptr);
    virtual ~GW2Account() override;
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QString getId() const;

    QString getName() const;

    int getAge() const;

    int getWorld() const;

    QDateTime getCreated() const;

    bool getCommander() const;

    int getFractalLevel() const;

    int getDailyAp() const;

    int getMonthlyAp() const;

    int getWvwRank() const;

    bool getValid() const;

public slots:
    void updateData();

private:
    bool valid;
    QString id;
    QString name;
    int age;
    int world;
    QDateTime created;
    bool commander;
    int fractalLevel;
    int dailyAp;
    int monthlyAp;
    int wvwRank;
};

#endif // GW2ACCOUNT_H_F
