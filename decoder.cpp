#include "main.h"

/* ������� ��������������� ����, �������������� ������� ��������, ��������� ��������� ��������� ������.

���������:
fstream& binaryFileToDecode - ����, ������� ��������� �����������, ������ �� ������ � �������� ����.
unsigned blockSize - ������ ��������������� ����� ����������� (��� ����� ����������� ���).
string fileWithDecodedInformationPath - ���� � �����, ���� ������ ���� �������� �������������� ���������.

���������� ��� ����������, ������ ERROR_SUCCESS ��� ������. */
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath) {
	// ���������� �������� ��� � �������������� ����� (�������� ���� ����, ��� ��� � ������ ����� ����� ���������� � ����� �������)
	size_t significantBitsInEncodedFileNumber = getFileLength(binaryFileToDecode) - 1 * BITS_IN_BYTE;
	// �������� ����� ��� ������ ��������������� ���������
	ofstream fileForDecodedInformation(fileForDecodedInformationPath, ios_base::binary);
	unsigned char residualBlockSize; // ������ ����������������� ����������� ����� � ����� (���� �� ����)
	// ��������� ������ ���� �����, � ������� ����� ������ ����������� ����� � �����
	binaryFileToDecode.read((char*)&residualBlockSize, sizeof(residualBlockSize)); 
	// ������ ��������������� ����������� ����� � ����� (���� �� ����)
	unsigned char residualEncodedBlockSize = getEncodedBlockLength(residualBlockSize);
	size_t encodedBlockLength = getEncodedBlockLength(blockSize); // ������ ��������������� ����� � �����
	// ������ ������ ��������������� ����� � ����� � ����� (��������� ���� ���� ����� � ����� ���������� ����)
	size_t fullBlockSizeInBites = ceil((double)encodedBlockLength / BITS_IN_BYTE) * BITS_IN_BYTE;
	// ���������� ����� ������ � �������������� �����
	size_t fullBlockInEncodedFileNumber = significantBitsInEncodedFileNumber / fullBlockSizeInBites;
	// ������� ������ ��� ������ ����� �� ����� � ������� �������������� (���������������) ����
	bitset<MAX_BLOCK_LENGTH> currentBitset, currentDecodedBlock;
	
	for (size_t i = 0; i < fullBlockInEncodedFileNumber; i++) {
		currentBitset = readBlock(binaryFileToDecode, fullBlockSizeInBites);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, encodedBlockLength);
	}
	
	return ERROR_SUCCESS;
}

bitset<MAX_BLOCK_LENGTH> decodeAndRestoreBlock(bitset<MAX_BLOCK_LENGTH> encodedBlock, unsigned encodedBlockSize) {
	bitset<MAX_BLOCK_LENGTH> decodedBlock; // �������������� ���� ��� ����������� ���
	size_t currentBitIndex; // ������ �������� ���� � �������
	size_t errorBitPosition = NO_ERROR_BITS; // ����� ���������� ����, ������� ��������� �������������. �� ��������� ������ ���� ���
	bool currentControlBit = 0; // ������� ����������� ���, ������� �� ���������

	// ���������� �� ���� ����������� ����� (�������� ������)
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockSize + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		for (size_t i = currentBitIndex; i < encodedBlockSize; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber; k++) {
				// ������� �������� � ������� �������� xor, ��������� ������� ���� �� ������ ������ �� ��������
				if (k != currentBitIndex) currentControlBit ^= encodedBlock[k];
			}
		}

		/* ���� �������� �������� ������������ ����, ����������� �� ��������� �������������� �����, �� ���������
		* � ��������� ��������� (���������) ������������ ����, �� ��������� ��� ������� � ����� ������� ���������� ���� */
		if (currentControlBit != encodedBlock[currentBitIndex]) {
			if (errorBitPosition == NO_ERROR_BITS) errorBitPosition = 0;
			errorBitPosition += currentBitNumber;
		}
		// �������� ����������� ���, ����� �� ��������� �������� ������ � ������� �����
		currentControlBit = 0;
	}

	// ���� ������� ��������� ���
	if (errorBitPosition != NO_ERROR_BITS) {
		// �������� �������, ������ ��� �� ������ ����� ����� ��� �� �������, � ������� � ��� ��������� � �������� ������
		errorBitPosition--;
		encodedBlock.flip(errorBitPosition); // ����������� ��������� ���
	}

	// ��������� � �������������� ���� ��� �������������� ���� �� ���������������
	for (size_t i = 0, currentBitIndex = 0; i < encodedBlockSize; i++) {
		// ���������� ����������� ����
		if (isIndexPowerOfTwo(i)) continue;
		decodedBlock[currentBitIndex++] = encodedBlock[i];
	}

	return decodedBlock;
}