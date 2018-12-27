#include "price.h"

using nlohmann::json;

void from_json(const json& j, PriceData& data)
{
    json buys = j.value("buys", json());
    json sells = j.value("sells", json());
    if (sells.is_null() || buys.is_null()) {
        return;
    }
    data.buyPrice = buys.value("unit_price", 0);
    data.buyCount = buys.value("quantity", 0);
    data.sellPrice = sells.value("unit_price", 0);
    data.sellCount = sells.value("quantity", 0);
    data.whitelisted = j.value("whitelisted", false);
}

void PriceData::fetch()
{
    Fetcher::fetcher.drop(fetchId);
    fetchId = Fetcher::fetcher.fetch(endpoint + std::to_string(itemId), jsonProcBinder(*this), maxCacheAge);
}

std::string PriceData::niceString(int stack)
{
    if (sellCount == 0) {
        return std::string("Not Traded");
    }

    std::string result;
    result = "Buy: ";
    result += std::to_string(buyPrice);
    result += "\nSell: ";
    result += std::to_string(sellPrice);
    if (stack != 1) {
        result += "\nBuy All";
        result += std::to_string(buyPrice * stack);
        result += "\nSell All: ";
        result += std::to_string(sellPrice * stack);
    }
    return result;
}