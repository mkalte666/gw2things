#ifndef _account_overview_h
#define _account_overview_h

#include "endpoint.h"
#include "guildpublic.h"

struct AccountOverviewData : public AbstractEndpoint {
    AccountOverviewData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/account", 60)
    {
    }
    void fetch() override;
    void show() override;
    virtual void onFetchComplete() override;

    std::string id = "";
    std::string name = "Unknown";
    int age = 0;
    int world = 0;
    std::vector<std::shared_ptr<GuildPublicData>> guilds;
    std::vector<std::string> access;
    bool commander = false;
    int fractalLevel = 0;
    int dailyAp = 0;
    int monthlyAp = 0;
    int wvwRank = 0;
};
json_helper(AccountOverviewData);

#endif //_account_overview_h