//
// Copyright (C) 2019-2025 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
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

#include <iostream>
#include <vector>
#include <filesystem>
#include <functional>

#include "IntersectionMatlab.h"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

using namespace matlab::engine;
using namespace matlab::data;
using namespace std;
//using namespace std::placeholders;

namespace plexe {

Define_Module(IntersectionMatlab);

std::unique_ptr<MATLABEngine> matlabEngine;

void IntersectionMatlab::initialize(int stage)
{
    if(stage == 0){
        alpha = par("alpha").doubleValue();
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
        connectMatlabSession(convert.from_bytes(par("matlabSessionName").stdstringValue()));
        accA.setName("accA");
        accB.setName("accB");
        accC.setName("accC");
    }
}

void IntersectionMatlab::connectMatlabSession(const std::u16string sessionName){
    try
    {
        matlabEngine = matlab::engine::connectMATLAB(sessionName);
    }
    catch(const EngineException& e)
    {
        throw cRuntimeError("Failed to connect to MATLAB session: %s\nDid you start matlab and run the init script?", e.what());
    }   
}

void IntersectionMatlab::startMatlabEngine(){
    try
    {
        matlabEngine = startMATLAB();
        // add script path
        std::string relPath = "../../matlab";
        char* cAbsPath = realpath(relPath.c_str(), NULL);
        std::string sAbsPath(cAbsPath);
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
        std::u16string absPath = convert.from_bytes(sAbsPath);
        std::u16string addPathCmd = u"addpath('" + absPath + u"');";
        matlabEngine->eval(addPathCmd);
        delete cAbsPath;

    }
    catch(const EngineException& e)
    {
        EV_ERROR << "Failed to start MATLAB: " << e.what() << std::endl;
    }   
}

void IntersectionMatlab::finish()
{
    matlabEngine.reset();
}

void IntersectionMatlab::handleMessage(cMessage* msg)
{
    delete msg;
}

MatlabResult IntersectionMatlab::intersection(MatlabPlatoons* platoons, double safetyDistance, double maxAcc, double maxSpeed)
{   
    ArrayFactory factory;
    StructArray structArray = factory.createStructArray({platoons->size(), 1}, {"distance", "speed", "length", "vehCount", "roadId"});

    for(int i=0; i<platoons->size(); i++){
        structArray[i]["distance"] = factory.createScalar(platoons->at(i)->distance);
        structArray[i]["speed"] = factory.createScalar(platoons->at(i)->speed);
        structArray[i]["length"] = factory.createScalar(platoons->at(i)->length);
        structArray[i]["vehCount"] = factory.createScalar(platoons->at(i)->vehCount);
        structArray[i]["roadId"] = factory.createScalar(platoons->at(i)->roadId.c_str());
    }
    
    std::vector<Array> args;
    args.push_back(structArray);
    args.push_back(factory.createScalar(safetyDistance));
    args.push_back(factory.createScalar(maxAcc));
    args.push_back(factory.createScalar(maxSpeed));
    args.push_back(factory.createScalar(alpha));

    std::vector<Array> out = matlabEngine->feval(u"intersection", 4, args);
    if (out.size() != 4) {
        throw cRuntimeError("MATLAB returned a wrong number of elements");
    }

    MatlabResult res;
    for(int i=0; i<platoons->size(); i++){
        double a = out[1][i];
        double v = out[3][i];
        res.push_back(std::make_pair(a,v));
    }
    accA.record((double)out[1][0]);
    accB.record((double)out[1][1]);
    accC.record((double)out[1][2]);
    
    return res;
}

IntersectionMatlab::~IntersectionMatlab(){
}

}
