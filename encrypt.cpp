#include<iostream>
#include<fstream>
#include<openssl/evp.h>
#include<openssl/rand.h>
#include<vector>

using namespace std;

unsigned char key[16], iv[16]; // as we use 128 cbc means 16 byte

// Function to encrypt data using AES CBC mode
string encryptData(const string& plaintext, const unsigned char* key, const unsigned char* iv) {
    //creates a new cipher context for encryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    /*
      initializes the encryption operation with the specified cipher algorithm 
      (EVP_aes_128_cbc() in this case), key, and initialization vector (IV).
      CBC (Cipher Block Chaining) mode is used here.
    */
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv);
    /*
    Memory is allocated for the ciphertext buffer. 
    The length of the ciphertext buffer is calculated as plaintext.length() + EVP_MAX_BLOCK_LENGTH. 
    This accounts for the maximum possible size of the ciphertext, which might be larger than the plaintext due to padding.
    */
    int ciphertextLen = plaintext.length() + EVP_MAX_BLOCK_LENGTH;
    unsigned char* ciphertext = new unsigned char[ciphertextLen];

    /*
    performs the encryption operation. 
    It encrypts the input plaintext (plaintext.c_str()) and writes the encrypted data to the ciphertext buffer.
    The len parameter is used to store the actual length of the output.
    */

    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length());
    int ciphertextLen1 = len;

    /*
    completes the encryption operation.
    It writes any remaining ciphertext bytes to the ciphertext buffer.
    The len parameter is updated with the number of bytes written
    */
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    // make ciphertext actual length 
    ciphertextLen1 += len;


    // The ciphertext is converted to a string using its constructor, 
    // which takes a pointer to the ciphertext buffer and its length
    string encryptedData(reinterpret_cast<char*>(ciphertext), ciphertextLen1);

    // deallocating memory of ciphertext
    delete[] ciphertext;

    // The cipher context is freed using EVP_CIPHER_CTX_free() 
    // to release any resources allocated during encryption
    EVP_CIPHER_CTX_free(ctx);

    return encryptedData;
}
void key_generate(){
    // Generate a random key and IV
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));
    cout<< "KEY generated\n";
    // Write key to a text file
    ofstream keyFile("key.txt");
    if (!keyFile.is_open()) {
        cerr << "Error: Unable to create key file.\n" << endl;
        exit(0);
    }
    keyFile.write(reinterpret_cast<const char*>(key), sizeof(key));
    keyFile.close();
}
int AES_encrypt(string& file_name) {

    // Read plaintext from a file
    ifstream inputFile(file_name+".txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open "<<file_name<<" file." << endl;
        return 1;
    }
    /*
    1. (istreambuf_iterator<char>(inputFile)): This part creates an istreambuf_iterator object, 
    which is an iterator used to read characters from an input stream buffer (inputFile in this case).
    It reads characters of type char from the input stream.

    2. (istreambuf_iterator<char>()): This part creates another istreambuf_iterator object, 
    but without any arguments. This creates an end-of-stream iterator, indicating the end of the input.
    */
    string plaintext((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
    // convert string from  range between the two iterators.
    inputFile.close();


    // Encrypt the plaintext using AES CBC mode
    string ciphertext = encryptData(plaintext, key, iv);

    // Write ciphertext to an output file
    ofstream outputFile(file_name+".dat", ios::binary);
    if (!outputFile.is_open()) {
        cerr << "Error: Unable to create output file." << endl;
        return 1;
    }
    // we stored the initialization vector in file only then append the actual file
    outputFile.write(reinterpret_cast<const char*>(iv), sizeof(iv));
    outputFile.write(ciphertext.c_str(), ciphertext.length());
    outputFile.close();
    
    

    cout << "Encryption of "<<file_name<<" completed successfully.\n" << endl;
    return 0;
}
/*
g++ -o program encrypt.cpp -lssl -lcrypto
./program
*/
