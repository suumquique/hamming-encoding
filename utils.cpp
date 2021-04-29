#include "main.h"

/* � �������� ��������� ������ ���� ������� ��� �������� �� ������ � �������� ������� �������� �����.
���������� ������ ����� � ����� */
size_t getFileLength(fstream& file) {

	// �������� ��������� �� ����� �����
	file.seekg(0, ios_base::end);
	// ������, �� ����� ����� ����� ���������
	size_t length_in_bytes = static_cast<size_t>(file.tellg());
	// �������� ��������� ����� � ������ �����
	file.seekg(0, ios_base::beg);

	return length_in_bytes * BITS_IN_BYTE;
}

// ���������� ������ ��������������� �����, �� ���� ���������� �������������� � ����������� ���, ������ ������
size_t getEncodedBlockLength(unsigned blockSize) {
	// ���������� ����������� ��� - �������� �������� �� ���������� ��� � �����, ����������� � ������� �������
	unsigned controlBitsNumber = floor(log2(blockSize) + 1);

	/* ������ ��� ���������� ����������� ��� ���������� ��������, ��� � ����� � ��������������� �� ���������� �������,
	��� ��������� ��������� ��� ���� ����������� ��� (��������, 12 ����� �������������� + 4 ����������� => ���������� 16 ���, 
	� 16 ������ ���� �����������, �������������, ��������� �������� ��� ���� ����������� ��� �� 16 ������� � ��������� ��������� 
	�������������� �� 17-� �������) */
	while (floor(log2(blockSize + controlBitsNumber) + 1) > controlBitsNumber) {
		controlBitsNumber++;
	}

	return blockSize + controlBitsNumber;
}

/*������� ��������� �� ����� ��������� ���������� ��� (�� ������� �����), ����� ��� ���������.
���� ���������� ��������� ��� ��� ���������� �� ������ ���������� ��� � ����� (������), �� ������������ � ������� ����
������� ����������� � ����������� ���������� �� ���������� ������ �������. ��� ��������� ������ � ������ ������� ����������.

� �������� ���������� ��������� �������� ����� � �����������, �������� �� ������ � �������� ���� (binaryInputFile) �
������ ������ ����� ��� ����������� ��� (������ ���������� �������������� - blockSizeInBites).

���������� ������ �����, � ����� �������� �������� �������� �������� �����; �� ����, ���� ���� - 64 ����, �� 
��������� 64 ���� bitset`� ����� ��������� ��������� ��������*/
bitset<MAX_BLOCK_LENGTH> readBlock(fstream& binaryInputFile, unsigned blockSizeInBites) {
	// ����������� ���������� ��� ���������� ���, ���� ������ ����� �� ������ ������
	static bitset<MAX_BLOCK_LENGTH> residualBits;
	// ���������� ���������� � �������� ������ ������� ���
	static size_t residualBitsCount = 0;

	char temp; // ��������� ���������� ��� ���������� ������ ����� �� ������
	bitset<8> tempByte; // ��������� ���������� �� ������ ��� ��� ������ � ����� ������ �� ������
	unsigned currentBitNumber = 0; // ����� �������� ����
	bitset<MAX_BLOCK_LENGTH> currentBlock; // ������� ���� �����������

	// ���� � ��� ���� ������� � �������� ������ �������, ������� ���������� � ������� ������ �������
	while (residualBitsCount != 0 && currentBitNumber < blockSizeInBites) {
		currentBlock[currentBitNumber] = residualBits[currentBitNumber];
		currentBitNumber++;
		residualBitsCount--;
	}

	// ������ �� �����, ���� ����� �������� ���� ������, ��� ������ ����� (�� ��� �� ��������� ��������� ����)
	while (true) {
		binaryInputFile.read((char*)&temp, sizeof(temp));
		/* ��������� ���� eof ��������������� ������������� � ��� ������, ����� ��� ��������� ������� ������ �� �������� �����,
		* �� ��������� ������� ����� ����� ����������, � ���� ���� while(true) */
		if (binaryInputFile.eof()) return currentBlock;
		tempByte = bitset<8>(temp);

		// ������ ������� ���� ��������
		for (size_t i = 0; i < BITS_IN_BYTE; i++) {
			// ���� ����� �������� ���� ������, ��� ������ �����, ��������� ���� �� �������� ����� � ����
			if (currentBitNumber < blockSizeInBites) currentBlock[currentBitNumber++] = tempByte[i];
			// ���� ��� ���������� ����� �� �������� ����� �� ��������� ��������� ���� � �������� ��� ����
			else {
				// ���������� ���������� ���� � "�������" (�������) �� ���������� ������ �������
				for (size_t k = i; k < BITS_IN_BYTE; k++) {
					residualBits[k - i] = tempByte[k];
					residualBitsCount++;
				}

				// ��������� ���� ��� ��������, ���������� ���
				return currentBlock;
			}
		}

		

		if (currentBitNumber == blockSizeInBites) return currentBlock;
	}

	// ���������, ���� ��������� ����� �� ����, ���� ������� �� �����, �� �� ��������� ��������� ������� � ���������� ��� � �������
	residualBits.reset();
	residualBitsCount = 0;
}
