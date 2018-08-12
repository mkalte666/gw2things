#ifndef GW2ACCOUNT_H
#define GW2ACCOUNT_H

#include "gw2common.h"
#include <QDateTime>

namespace GW2 {

/**
 * @brief The Account class
 */
class Account
{
public:
    /**
     * Callback type for this API type
     */
    using Callback = std::function<void(const Account&)>;

    /**
     * @brief Account
     * @param data Json to load this object from
     */
    Account(const QJsonObject& data);

    /**
     * @brief getId
     * @return
     */
    QString getId() const;

    /**
     * @brief getName
     * @return
     */
    QString getName() const;

    /**
     * @brief getAge
     * @return
     */
    int getAge() const;

    /**
     * @brief getWorld
     * @return
     */
    int getWorld() const;

    /**
     * @brief getCreated
     * @return
     */
    QDateTime getCreated() const;

    /**
     * @brief getCommander
     * @return
     */
    bool getCommander() const;

    /**
     * @brief getFractalLevel
     * @return
     */
    int getFractalLevel() const;

    /**
     * @brief getDailyAp
     * @return
     */
    int getDailyAp() const;

    /**
     * @brief getMonthlyAp
     * @return
     */
    int getMonthlyAp() const;

    /**
     * @brief getWvwRank
     * @return
     */
    int getWvwRank() const;

private:
    QString id;
    QString name; ///< name of the account
    int age; ///< age of the account in seconds
    int world; ///< world id of the character
    QDateTime created; ///< creation date
    bool commander; ///< if the character is a commander
    int fractalLevel; ///<
    int dailyAp; ///<
    int monthlyAp; ///<
    int wvwRank; ///<
};

}; // namespace GW2

#endif // GW2ACCOUNT_H
