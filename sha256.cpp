#include <bitset>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>

const int SIZE = 1024;

unsigned int constants[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};
unsigned int hashValues[] = { 0x6a09e667, 0xbb67ae85, 
                              0x3c6ef372, 0xa54ff53a, 
                              0x510e527f, 0x9b05688c, 
                              0x1f83d9ab, 0x5be0cd19 };
unsigned int hashValuesBeforeOperations[8];
unsigned long long len(char* str) {
  unsigned long long counter = 0;
  while(str[counter++] != '\0');
  return counter - 1; 
}
unsigned int sizeOfBinaryBlock(char* str) {
  unsigned int size = 8 * len(str);
  unsigned int wantedSize = 0;
  while(wantedSize < size + 1 + 64) wantedSize += 512;
  return wantedSize / 8;
}
unsigned int rotr(unsigned int num, short numTimes) {
  return (num >> numTimes) | (num << (-numTimes & 4294967295)); // 2 ** 32 - 1
}

unsigned int sigma(unsigned int word, short rotrOne, short rotrTwo, short shift) {
  int rotrFirst = rotr(word, rotrOne);
  int rotrSecond = rotr(word, rotrTwo);
  int shiftedRight = word >> shift;
  int res = rotrFirst ^ rotrSecond ^ shiftedRight;
  
  return res;
}
unsigned int SIGMA(unsigned int word, short rotrOne, short rotrTwo, short rotrThree) {
  unsigned int rotrFirst = rotr(word, rotrOne);
  unsigned int rotrSecond = rotr(word, rotrTwo);
  unsigned int rotrThird = rotr(word, rotrThree);
  unsigned int res = rotrFirst ^ rotrSecond ^ rotrThird;
  
  return res;
}
unsigned int sigma0(unsigned int word) {
  return sigma(word, 7, 18, 3);
}
unsigned int sigma1(unsigned int word) {
  return sigma(word, 17, 19, 10);
}
unsigned int SIGMA0(unsigned int word) {
  return SIGMA(word, 2, 13, 22);
}
unsigned int SIGMA1(unsigned int word) {
  return SIGMA(word, 6, 11, 25);
}
bool getBit(unsigned int num, short bit) {
  return ((num >> bit) & 1);
}

unsigned int choice(unsigned int selector, unsigned int word1, unsigned int word2) {
  return (selector & word1) ^ (~selector & word2);
}
unsigned int majority(unsigned int w1, unsigned int w2, unsigned int w3) {
  return (w1 & w2) ^ (w1 & w3) ^ (w2 & w3);
}
void fillUpIntHolder(unsigned int* intHolder, char* str, int sizeOfBinary) {

  // alphanumeric values of the letters in the message
  for(int i = 0; i < len(str); i++) {
    intHolder[i] = str[i];
  }

  // adding the 1 separator
  short oneAsSeparator = 128; // 10000000
  intHolder[len(str)] = oneAsSeparator;

  // filling up with zeros until the last 64 bits come up
  for(int i = len(str) + 1; i < sizeOfBinary - 2; i++) {
    intHolder[i] = 0;
  }

  // last 64 bits holding the message length
  std::bitset<64> lengthOfMessage = len(str) * 8;
  for(short i = sizeOfBinary - 8; i < sizeOfBinary; i++) {
    int temp = 0;
    for(int j = 0; j < 8; j++) {
        temp += lengthOfMessage[64 - 1 - 8 * (i % (sizeOfBinary - 8)) - j] * pow(2, 8 - 1 - j);
    }
    intHolder[i] = temp;
  }

}

bool hashShouldBeSavedToFile() {
  char answer[3];
  std::cout << "Do you want to save the hash into a file? (y/n) ";
  std::cin >> answer;
  if(answer[0] == 'n') return 0;
  return 1;
}

void validateInput(char* input) {
  bool hasEnded;
  
  for (size_t i = 0; i < SIZE; i++) {
    if(input[i] == '\0') hasEnded = 1; 
  }
  if(!hasEnded) {
    std::cerr << "Message is too long. Please, cut it down to 1024 characters or less.";
    exit(1);
  }
}

char* getInput() {
  char* input = new char[SIZE];
  std::cout << "Do you want to hash the contents of a file? (y/n) ";
  char answer[3];
  std::cin >> answer;
  if(answer[0] == 'n') {
    std::cout << "Enter the message you want to hash: ";
    std::cin >> input;
  } else {
    std::cout << "Enter file name: ";
    char fileName[SIZE];
    std::cin >> fileName;
    std::ifstream inputFile;
    inputFile.open("s2");
    if(inputFile) {
      for (size_t i = 0; i < SIZE; i++) {
        fileName[i] = inputFile.get();
        std::cout << (fileName[i] != '\0'); 
      }
    } else {
      std::cerr << "Error: file could not be opened.";
      exit(1);
    }
  }
  validateInput(input);
  return input;
}

void saveHashValuesBeforeOperations(unsigned int* hashValues, unsigned int* hashValuesBeforeOperations) {
  for (short i = 0; i < 8; i++) {
    hashValuesBeforeOperations[i] = hashValues[i];
  }
}

void fillUpBlock(unsigned int* block, int whichBlock, unsigned int* intHolder) {
  for(short i = 0; i < 16; i++) { 
      unsigned int word = 0;
      for(short j = 0; j < 4; j++) {
        word = word << 8;
        word ^= intHolder[whichBlock * 64 + i * 4 + j];
      }
      block[i] = word;
    }

  for(short i = 16; i < 64; i++) {
    block[i] = sigma1(block[i - 2]) + 
               block[i - 7] +
               sigma0(block[i - 15]) + 
               block[i - 16];
  }
    
}

void doACompressionRound(unsigned int* block) {
  for (short i = 0; i < 64; i++) {
    unsigned int t1 = SIGMA1(hashValues[4]) + 
        choice(hashValues[4], hashValues[5], hashValues[6]) + 
        hashValues[7] + block[i] + constants[i];
    
    unsigned int t2 = SIGMA0(hashValues[0]) + 
        majority(hashValues[0], hashValues[1], hashValues[2]);
    
    unsigned int tempHashValues[8] = { t1 + t2 };
    
    for(short j = 1; j < 8; j++) {
      tempHashValues[j] = hashValues[j - 1];
    }
    tempHashValues[4] += t1;
    std::swap(tempHashValues, hashValues);
  }
  for(short i = 0; i < 8; i++) {
    hashValues[i] += hashValuesBeforeOperations[i];
  }

}

void chooseOutputMeans() {
  bool toFile = hashShouldBeSavedToFile();

  if(toFile) {
    std::ofstream outdata;
    char fileName[SIZE];
    std::cout << "Enter file name: ";
    std::cin >> fileName;
    bool hasEnded;
    for (size_t i = 0; i < SIZE; i++) {
      if(fileName[i] == '\0') hasEnded = 1; 
    }
    if(!hasEnded) {
      std::cerr << "File name is too long. Please, use less than 1024 characters.";
      exit(1);
    } 
    
    outdata.open(fileName);
    if(!outdata) {
      std::cerr << "Error: file could not be opened.";
      exit(1);
    }

    outdata << std::setfill('0');
    for(short i = 0; i < 8; i++) {
      outdata << std::hex << std::setw(8) << hashValues[i];
    }
    std::cout << "Successfully written to \"" << fileName << "\".";
    outdata.close();

  } else {
    std::cout << "Hashed message: ";
    std::cout << std::setfill('0');
    for(short i = 0; i < 8; i++) {
      std::cout << std::hex << std::setw(8) << hashValues[i];
    }
  }
}

void sha256() {
  char* str = new char[SIZE]; 
  str = getInput();

  int sizeOfBinary = sizeOfBinaryBlock(str);
  unsigned int* intHolder = new unsigned int[sizeOfBinary];
  fillUpIntHolder(intHolder, str, sizeOfBinary);

  // Split the 1's and 0's into blocks
  int numberOfBlocks = sizeOfBinary / (512 / 8);
  unsigned int** blocks = new unsigned int*[numberOfBlocks];

  for(short i = 0; i < numberOfBlocks; i++) {
    saveHashValuesBeforeOperations(hashValues, hashValuesBeforeOperations);
    
    blocks[i] = new unsigned int[64];
    fillUpBlock(blocks[i], i, intHolder);
    doACompressionRound(blocks[i]);
  }
  chooseOutputMeans();
}


int main() {
  sha256(); 
  return 0;
}

