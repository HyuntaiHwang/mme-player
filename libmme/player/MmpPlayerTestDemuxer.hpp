/*
 *
 *  Copyright (C) 2010-2011 TokiPlayer Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MMPPLAYERTEXTDEMUXER_HPP__
#define _MMPPLAYERTEXTDEMUXER_HPP__

#include "MmpPlayer.hpp"

class CMmpPlayerTestDemuxer : public CMmpPlayer
{
friend class CMmpPlayer;

private:

protected:
    CMmpPlayerTestDemuxer(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerTestDemuxer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();
};

#endif


