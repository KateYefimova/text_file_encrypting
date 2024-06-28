#include <iostream>
#include <cstring>
using namespace std;

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

extern "C" {
    __declspec(dllexport) char* encrypt(char* rawText, int key) {
        int length = strlen(rawText);
        char* result = new char[length + 1];
        for (int i = 0; i < length; i++) {
            if (isupper(rawText[i])) {
                result[i] = char(int(rawText[i] + key - 'A') % 26 + 'A');
            }
            else if (islower(rawText[i])) {
                result[i] = char(int(rawText[i] + key - 'a') % 26 + 'a');
            }
            else {
                result[i] = rawText[i];
            }
        }
        result[length] = '\0';
        return result;
    }

    __declspec(dllexport) char* decrypt(char* encryptedText, int key) {
        int length = strlen(encryptedText);
        char* result = new char[length + 1];
        for (int i = 0; i < length; i++) {
            if (isupper(encryptedText[i])) {
                result[i] = char((int(encryptedText[i] - key - 'A' + 26) % 26 + 'A'));
            }
            else if (islower(encryptedText[i])) {
                result[i] = char((int(encryptedText[i] - key - 'a' + 26) % 26 + 'a'));
            }
            else {
                result[i] = encryptedText[i];
            }
        }
        result[length] = '\0';
        return result;
    }
}