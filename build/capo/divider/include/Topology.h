﻿/*
 * Copyright (c) 2023 Peraton Labs
 * SPDX-License-Identifier: Apache-2.0
 * @author tchen
 */

#pragma once

#include <string>
#include <vector>
#include <map>

#include "nlohmann/json.hpp"

#include "Annotation.h"

using json = nlohmann::json;
using namespace std;

class Topology
{
protected:
    string sourcePath;
    vector<string> enclaves;
    vector<string> levels;
    vector<Annotation> functions;
    vector<Annotation> globalScopedVars;

    // levels -> (name -> annotation)
    map<string, map<string, Annotation>> allAnnotations;

    // not in JSON
    string outputDir;
    string fileInProcess;
    string levelInProgress;

public:
    Topology() {
    }

    string getOutputFile() {
        return outputDir + "/" + levelInProgress + "/" + fileInProcess;
    }

    bool isNameInLevel(string &name, string &level);

    void parse(string &topology);
    void parseAnnotations(nlohmann::basic_json<> values, vector<Annotation> &list);
    void parseStrings(nlohmann::basic_json<> values, vector<string> &list);

    string &getSourcePath() { 
        return this->sourcePath; 
    }

    vector<string> &getEnclaves() { 
        return this->enclaves; 
    }
    
    vector<string> &getLevels() {
        return this->levels; 
    }

    vector<Annotation> &getFunctions() { 
        return this->functions; 
    }

    vector<Annotation> &getGlobalScopedVars() {
        return this->globalScopedVars; 
    }

    string &getOutputDir() {
        return this->outputDir;
    }

    void setOutputDir(string &outputDir) {
        this->outputDir = outputDir;
    }

    string &getFileInProcess() {
        return this->fileInProcess;
    }

    void setFileInProcess(string fileInProcess) {
        this->fileInProcess = fileInProcess;
    }

    void setLevelInProgress(string levelInProgress) {
        this->levelInProgress = levelInProgress;
    }

    string &getLevelInProgress() {
        return this->levelInProgress;
    }
};
