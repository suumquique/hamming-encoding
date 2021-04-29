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
//  останта, показывающа€, что ошибочных битов нет
#define NO_ERROR_BITS UINT_MAX

size_t getFileLength(fstream& file);
DWORD encode(fstream& binaryFileToEncode, unsigned blockSize, string fileForEncodedInformationPath);
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath);
size_t getEncodedBlockLength(unsigned blockSize);
bitset<MAX_BLOCK_LENGTH> readBlock(fstream& binaryInputFile, unsigned blockSizeInBites);
vector<byte> encodeBlock(bitset<MAX_BLOCK_LENGTH> block, unsigned blockSizeInBites);
bitset<MAX_BLOCK_LENGTH> decodeAndRestoreBlock(bitset<MAX_BLOCK_LENGTH> encodedBlock, unsigned encodedBlockSize);
vector<byte> convertDecodedBlockAndAddToByteArray(bitset<MAX_BLOCK_LENGTH> decodedBlock, unsigned blockSizeInBites, vector<byte> currentVector, BOOL end=FALSE);
BOOL isIndexPowerOfTwo(size_t index);
size_t getIntegerBlockSizeInBytes(size_t blockSizeInBites);

#endif // !MAIN_H