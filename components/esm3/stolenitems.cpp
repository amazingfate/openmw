#include "stolenitems.hpp"

#include <components/esm3/esmreader.hpp>
#include <components/esm3/esmwriter.hpp>

namespace ESM
{

    void StolenItems::write(ESMWriter& esm) const
    {
        for (StolenItemsMap::const_iterator it = mStolenItems.begin(); it != mStolenItems.end(); ++it)
        {
            esm.writeHNString("NAME", it->first);
            for (std::map<std::pair<std::string, bool>, int>::const_iterator ownerIt = it->second.begin();
                 ownerIt != it->second.end(); ++ownerIt)
            {
                if (ownerIt->first.second)
                    esm.writeHNString("FNAM", ownerIt->first.first);
                else
                    esm.writeHNString("ONAM", ownerIt->first.first);
                esm.writeHNT("COUN", ownerIt->second);
            }
        }
    }

    void StolenItems::load(ESMReader& esm)
    {
        while (esm.isNextSub("NAME"))
        {
            std::string itemid = esm.getHString();

            std::map<std::pair<std::string, bool>, int> ownerMap;
            while (esm.isNextSub("FNAM") || esm.isNextSub("ONAM"))
            {
                const bool isFaction = (esm.retSubName() == "FNAM");
                std::string owner = esm.getHString();
                int count;
                esm.getHNT(count, "COUN");
                ownerMap.emplace(std::make_pair(std::move(owner), isFaction), count);
            }

            mStolenItems.insert_or_assign(std::move(itemid), std::move(ownerMap));
        }
    }

}
