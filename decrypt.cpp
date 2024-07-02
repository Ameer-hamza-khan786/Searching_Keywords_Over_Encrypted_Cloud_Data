#include<iostream>
#include<fstream>
#include<openssl/evp.h>
#include<vector>
using namespace std;

// Function to decrypt data using AES CBC mode
string decryptData(const string& ciphertext, const unsigned char* key, const unsigned char* iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv);

    int plaintextLen = ciphertext.length() + EVP_MAX_BLOCK_LENGTH; // Make room for padding
    unsigned char* plaintext = new unsigned char[plaintextLen];

    int len;
    EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.length());
    int plaintextLen1 = len;
    
    // Finalize decryption
    int finalLen;
    EVP_DecryptFinal_ex(ctx, plaintext + len, &finalLen);
    plaintextLen1 += finalLen;

    string decryptedData(reinterpret_cast<char*>(plaintext), plaintextLen1);

    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);

    return decryptedData;
}

int AES_decrypt(vector<string>& encrypted_files) {

    // Read key from the key file
    ifstream keyFile("key.txt", ios::binary);
    if (!keyFile.is_open()) {
        cerr << "Error: Unable to open key file." << endl;
        return 1;
    }
    unsigned char key[16];
    keyFile.read(reinterpret_cast<char*>(key), sizeof(key));
    keyFile.close();


    for(string file:encrypted_files){
    // Read IV and ciphertext from the encrypted file
    ifstream inputFile(file+".dat", ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open encrypted file." << endl;
        return 1;
    }

    unsigned char iv[16];
    inputFile.read(reinterpret_cast<char*>(iv), sizeof(iv));
    string ciphertext((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
    inputFile.close();

    // Decrypt the ciphertext using AES CBC mode
    string decryptedText = decryptData(ciphertext, key, iv);
    // Write the decrypted plaintext to a file
    
    ofstream outputFile(file+".txt");
    if (!outputFile.is_open()) {
        cerr << "Error: Unable to create output file." << endl;
        return 1;
    }
    outputFile << decryptedText;
    outputFile.close();
    }
    cout << "Decryption of files completed successfully.\n" << endl;
    return 0;
}
/*
g++ -o program decrypt.cpp -lssl -lcrypto
./program

g++ -o program client_main.cpp -lssl -lcrypto
*/
