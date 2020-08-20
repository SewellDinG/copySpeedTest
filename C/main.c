#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "md5.c"

int encryptFile(char *originalFile, char *secretKey, char *targetFile);

int copyFile(char *originalFile, char *targetFile);

/**
 * Usage: ./copyFileTest mode originalFile targetFile (secretKey)
 * 常规拷贝mode=0: ./copyFileTest 0 originalFile targetFile
 * 加密拷贝mode=1: ./copyFileTest 1 originalFile targetFile secretKey
**/
int main(int argc, char **argv, char **environ) {
    if (NULL == argv[1]) {
        puts("Args are needed!\n");
        return 0;
    }
    char *originalFile,  // 加密的文件名
    *targetFile,  // 加密后要保存的文件名
    *secretKey,  // 文件加密的密钥
    *mode;  // 常规拷贝0，加密拷贝1
    mode = argv[1];
    if (strcmp(mode, "0") == 0) {
        if (NULL != argv[4]) {
            puts("Too many args\n");
            return 0;
        }
        originalFile = argv[2];
        targetFile = argv[3];
        if (copyFile(originalFile, targetFile)) {
            printf("恭喜你，文件[%s]拷贝成功，保存在[%s]。\n", originalFile, targetFile);
        }
    } else if (strcmp(mode, "1") == 0) {
        if (NULL != argv[5]) {
            puts("Too many args\n");
            return 0;
        }
        originalFile = argv[2];
        targetFile = argv[3];
        secretKey = argv[4];
        if (encryptFile(originalFile, secretKey, targetFile)) {
            printf("恭喜你，文件[%s]加密成功，保存在[%s]。\n", originalFile, targetFile);
        }
    }
}

/**
  * 常规拷贝文件
  * 每次4096bits
**/
int copyFile(char *originalFile, char *targetFile) {
    FILE *fpOriginal, *fpTarget;  // 要打开的文件的指针
    char buffer[4096];  // 缓冲区，用于存放从文件读取的数据
    int readCount;  // 每次从文件中读取的字节数
    // 以二进制方式读取/写入文件
    fpOriginal = fopen(originalFile, "rb");
    if (fpOriginal == NULL) {
        printf("文件[%s]打开失败，请检查文件路径和名称是否输入正确！\n", originalFile);
        return 0;
    }
    fpTarget = fopen(targetFile, "wb");
    if (fpTarget == NULL) {
        printf("文件[%s]创建/写入失败！请检查文件路径和名称是否输入正确！\n", targetFile);
        return 0;
    }
    // 不断地从文件中读取4096bits长度的数据，保存到buffer，直到文件结束
    while ((readCount = fread(buffer, 1, sizeof(buffer), fpOriginal)) > 0) {
        // 将buffer中的数据写入文件
        fwrite(buffer, 1, readCount, fpTarget);
    }
    fclose(fpOriginal);
    fclose(fpTarget);
    return 1;
}

/**
 * 加密拷贝
 * @param   originalFile    要加密的文件名
 * @param   targetFile    加密后要保存的文件名
 * @param   secretKey     密钥
 * @return  加密成功或失败的数字表示
     0：加密失败
     1：加密成功
**/
int encryptFile(char *originalFile, char *secretKey, char *targetFile) {
    FILE *fpOriginal, *fpTarget;  // 要打开的文件的指针
    int readCount,  // 每次从文件中读取的字节数
    i;  // 循环次数
//    int blockNum = 0; // 块编号，++
    int keyLen = 1024; // 密钥的长度
    char buffer[keyLen];  // 缓冲区，用于存放从文件读取的数据
    // 以二进制方式读取/写入文件
    fpOriginal = fopen(originalFile, "rb");
    if (fpOriginal == NULL) {
        printf("文件[%s]打开失败，请检查文件路径和名称是否输入正确！\n", originalFile);
        return 0;
    }
    fpTarget = fopen(targetFile, "wb");
    if (fpTarget == NULL) {
        printf("文件[%s]创建/写入失败！请检查文件路径和名称是否输入正确！\n", targetFile);
        return 0;
    }
    unsigned char *keyTmp;
    keyTmp = (unsigned char *) malloc(sizeof(char) * 32);
    strcpy(keyTmp, secretKey);
    // 不断地从文件中读取 keyLen 长度的数据，保存到buffer，直到文件结束
    while ((readCount = fread(buffer, 1, keyLen, fpOriginal)) > 0) {
//        unsigned char *keyTmp;
//        keyTmp = (unsigned char *) malloc(sizeof(char) * (strlen(secretKey) + (4 + 1)));
//        strcat(keyTmp, secretKey);
//        unsigned char *blockNumTmp;
//        blockNumTmp = (unsigned char *) malloc(sizeof(char) * 4);
//        sprintf(blockNumTmp, "%d", blockNum);
//        strcat(keyTmp, blockNumTmp);
        unsigned char encryptMD5[32];
        MD5_CTX md5;
        MD5Init(&md5);
        MD5Update(&md5, keyTmp, strlen((char *) keyTmp));
        MD5Final(&md5, encryptMD5);
        // 对buffer中的数据逐字节进行异或运算
        for (i = 0; i < readCount; i++) {
            buffer[i] ^= encryptMD5[i];
        }
        free(keyTmp);
        // 替换key，下一轮重新MD5
        keyTmp = (unsigned char *) malloc(sizeof(char) * 32);
        strcpy(keyTmp, encryptMD5);
        // 将buffer中的数据写入文件
        fwrite(buffer, 1, readCount, fpTarget);
//        free(blockNumTmp);
//        blockNum++;
    }
    fclose(fpOriginal);
    fclose(fpTarget);
    return 1;
}