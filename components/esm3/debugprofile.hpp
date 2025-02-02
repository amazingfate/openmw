#ifndef COMPONENTS_ESM_DEBUGPROFILE_H
#define COMPONENTS_ESM_DEBUGPROFILE_H

#include <string>

#include "components/esm/defs.hpp"

namespace ESM
{
    class ESMReader;
    class ESMWriter;

    struct DebugProfile
    {
        constexpr static RecNameInts sRecordId = REC_DBGP;

        enum Flags
        {
            Flag_Default = 1, // add to newly opened scene subviews
            Flag_BypassNewGame = 2, // bypass regular game startup
            Flag_Global = 4 // make available from main menu (i.e. not location specific)
        };

        unsigned int mRecordFlags;
        std::string mId;

        std::string mDescription;

        std::string mScriptText;

        unsigned int mFlags;

        void load(ESMReader& esm, bool& isDeleted);
        void save(ESMWriter& esm, bool isDeleted = false) const;

        /// Set record to default state (does not touch the ID).
        void blank();
    };
}

#endif
