#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <Windows.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <bitset>
#include <algorithm>
using namespace std;

#define RUS_ENCODING 1251
#define BITS_IN_BYTE 8
// ћаксимальна€ длина блока, содержащего контрольные и информационные биты
#define MAX_BLOCK_LENGTH 270

size_t getFileLength(fstream& file);
fstream encode(fstream& binaryFileToEncode, unsigned blockSize);
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath);
size_t getEncodedBlockLength(unsigned blockSize);
bitset<MAX_BLOCK_LENGTH> readBlock(fstream& binaryInputFile, unsigned blockSizeInBites);
vector<byte> encodeBlock(bitset<MAX_BLOCK_LENGTH> block, unsigned blockSizeInBites);

#endif // !MAIN_H