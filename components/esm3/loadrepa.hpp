#ifndef OPENMW_ESM_REPA_H
#define OPENMW_ESM_REPA_H

#include <string>

#include "components/esm/defs.hpp"

namespace ESM
{

    class ESMReader;
    class ESMWriter;

    struct Repair
    {
        constexpr static RecNameInts sRecordId = REC_REPA;

        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string_view getRecordType() { return "Repair"; }

        struct Data
        {
            float mWeight;
            int mValue;

            int mUses;
            float mQuality;
        }; // Size = 16

        Data mData;
        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mIcon, mScript;

        void load(ESMReader& esm, bool& isDeleted);
        void save(ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };

}
#endif
