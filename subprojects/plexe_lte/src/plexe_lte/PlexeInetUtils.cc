//
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "PlexeInetUtils.h"

#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include <deque>
#include "plexe_lte/messages/PlatoonUpdateMessage_m.h"

namespace plexe {

PlexeInetUtils::PlexeInetUtils() {}

PlexeInetUtils::~PlexeInetUtils() {}

inet::Packet* PlexeInetUtils::encapsulate(omnetpp::cPacket* packet, const char* name)
{
    inet::Packet* container = new inet::Packet(name);
    auto data = inet::makeShared<inet::cPacketChunk>(packet);
    container->insertAtBack(data);
    return container;

}
omnetpp::cPacket* PlexeInetUtils::decapsulate(inet::Packet* packet)
{
    const inet::cPacketChunk* pc;

    pc = dynamic_cast<const inet::cPacketChunk*>(packet->peekAll().get());

    if (pc)
        return pc->getPacket()->dup();

    const inet::SequenceChunk* data = dynamic_cast<const inet::SequenceChunk*>(packet->peekAll().get());
    if (data) {
        const std::deque<inet::Ptr<const inet::Chunk>> chunks = data->getChunks();
        //        std::cout << "Sequence chunk with:\n";
        //        for (int i = 0; i < chunks.size(); i++) {
        //            const inet::Chunk* ch = chunks[i].get();
        //            std::cout << "\t" << typeid(*ch).name();
        //            pc = dynamic_cast<const inet::cPacketChunk*>(ch);
        //            if (pc) {
        //                std::cout << " -> " << typeid(*(pc->getPacket())).name();
        //                const PlatoonUpdateMessage* pum = dynamic_cast<const PlatoonUpdateMessage*>(pc->getPacket());
        //                if (pum) {
        //                    std::cout << " UPDATE";
        //                }
        //            }
        //            std::cout  << "\n";
        //        }
        pc = dynamic_cast<const inet::cPacketChunk*>(chunks[chunks.size()-1].get());
        if (pc)
            return pc->getPacket()->dup();
        else
            return nullptr;
    }
    //    else {
    //        const inet::Chunk* ch = packet->peekAll().get();
    //        std::cout << "Packet with chunk of type: " << typeid(*ch).name() << "\n";
    //        return nullptr;
    //    }
    return nullptr;
}

} // namespace plexe
