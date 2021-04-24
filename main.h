#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <Windows.h>
#include <fstream>
using namespace std;

#define RUS_ENCODING 1251
#define BITS_IN_BYTE 8

size_t getFileLength(fstream& file);
fstream encode(fstream& binaryFileToEncode, unsigned blockSize);
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, fstream& fileWithDecodedInformation);

#endif // !MAIN_H