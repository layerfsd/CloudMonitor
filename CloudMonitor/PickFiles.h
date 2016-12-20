#pragma once
#ifndef _PICK_FILES__
#define _PICK_FILES__

#include <iostream>
#include <string>
#include <vector>

using namespace std;


bool PickLocalPath(vector<string>& collector);
bool ScanLocalFiles(vector<Match>& scanResults);

// 远程控制接口---全盘扫描
bool RemoteScanLocalFiles(string& message, string& args);

#endif