#ifndef _account_characters_h
#define _account_characters_h

#include "character.h"
#include "endpoint.h"

struct AccountCharacterData : public AbstractEndpoint {
    AccountCharacterData()
        : AbstractEndpoint("https://api.guildwars2.com/v2/characters", 10)
    {
    }

    virtual void fetch() override;
    virtual void show() override;
    virtual void onFetchComplete() override;

    std::vector<std::string> characterNames;
    std::vector<std::shared_ptr<CharacterData>> characters;
};
json_helper(AccountCharacterData);

#endif