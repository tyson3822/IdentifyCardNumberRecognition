#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

class TestFile
{
private:
    char * pch;

    ifstream _inputFile;
    ofstream _outputFile;
    ofstream _resultFile;

    vector<string> imgBuffer;
    vector<string> testBuffer;
    vector<string> outputBuffer;
    vector<string> resultBuffer;

public:
    //初始化文件檔
    void InitTestFile(const char* inputFileName,const char*  outputFileName,const char*  ResultFileName)
    {
        _inputFile.open(inputFileName, std::ifstream::in);
        _outputFile.open(outputFileName , ofstream::out | std::ios::trunc);
        _resultFile.open(ResultFileName, ofstream::out | std::ios::trunc);
        InitTestData();
    }

    //初始化檔案資料＆讀取input檔
    void InitTestData()
    {
        const char *delim = ",";
        string buffer;
        string img;
        string test;
        char *charBuffer;
        while(_inputFile >> buffer)
        {
            //string to char*
            charBuffer = new char[buffer.length() + 1];
            strcpy(charBuffer, buffer.c_str());

            //strtok and push to vector
            pch = strtok(charBuffer, delim);
            img = string(pch); //char* to string
            imgBuffer.push_back(img);
            //cout << "push " << img << " to img vector" << endl;

            //strtok and push to vector
            pch = strtok(NULL, delim);
            test = string(pch); //char* to string
            testBuffer.push_back(test);
            //cout << "push " << test << " to test vector" << endl;

            outputBuffer.resize(imgBuffer.size());
        }
    }

    //取得圖片vector大小
    int GetImgVectorSize()
    {
        return imgBuffer.size();
    }

    //取得test vector大小
    int GetTestVectorSize()
    {
        return testBuffer.size();
    }

    //取得指定index圖片名稱
    string GetImgByIndex(int index)
    {
        return imgBuffer[index];
    }

    //取得指定index test名稱
    string GetTestByIndex(int index)
    {
        return testBuffer[index];
    }

    //顯示vector內容//debug用
    void ViewVector(char c)
    {
        switch (c)
        {
            case 'i':
            {
                cout << "-list image name vector-" << endl;
                for(int i  = 0; i < imgBuffer.size(); i++)
                {
                    cout << "image vector " << i << " = " << imgBuffer[i] << endl;
                }
                break;
            }
            case 't':
            {
                cout << "-list test vector-" << endl;
                for(int i  = 0; i < testBuffer.size(); i++)
                {
                    cout << "test vector " << i << " = " << testBuffer[i] << endl;
                }
                break;
            }
            case 'o':
            {
                cout << "-list output vector-" << endl;
                 for(int i  = 0; i < outputBuffer.size(); i++)
                {
                    if(outputBuffer[i].size() < 1)continue;
                    cout << "output vector " << i << " = " << outputBuffer[i] << endl;
                }
                break;
            }
            case 'r':
            {
                cout << "-list result vector-" << endl;
                 for(int i  = 0; i < resultBuffer.size(); i++)
                {
                     if(outputBuffer[i].size() < 1)continue;
                    cout << "result vector " << i << " = " << resultBuffer[i] << endl;
                }
                break;
            }
            default:
                cout << "default" << endl;
                break;
        }
    }

    //把資料寫進output vector中
    void WriteToOutput(string str)
    {
        outputBuffer.push_back(str);
    }

     //把資料寫進output vector指定Index中
    void WriteToOutputByIndex(string str, int index)
    {
        outputBuffer[index] = str;
    }

    //把資料寫進文件檔中
    void WriteDownOutput()
    {
        for(int i = 0; i < outputBuffer.size(); i++)
        {
            _outputFile << outputBuffer[i] << endl;
        }
    }

    //測試結果output
    void MatchResult()
    {
        ViewVector('o');
        //ViewVector('t');
        char outputCharArr[100];
        char testCharArr[100];
        for(int index = 0; index < outputBuffer.size(); index++)
         {
            //String To Char[] 轉型
            strcpy(outputCharArr, outputBuffer[index].c_str());
            strcpy(testCharArr, testBuffer[index].c_str());

            //比對測資與output是否一樣
            int result = MatchChar(outputCharArr, testCharArr);

            //將結果丟進resultBuffer中
            if(result == 1)resultBuffer.push_back("true");
            if(result == -1)resultBuffer.push_back("false");
        }
        ViewVector('r');

        //所有測試結果寫進文件中
        for(int index = 0; index < resultBuffer.size(); index++)
        {
            _resultFile << resultBuffer[index] << endl;
        }
    }

    //列出成功的test
    void ListSuccessTest()
    {
        char trueChar[10];
        char resultCharArr[100];
        strcpy(trueChar, "true");
         for(int i  = 0; i < resultBuffer.size(); i++)
        {
            if(outputBuffer[i].size() < 1)continue;//忽略掉沒有輸出的
            strcpy(resultCharArr, resultBuffer[i].c_str());
             if(MatchChar(resultCharArr, trueChar) != 1)continue;//忽略掉不是true的
            cout << "the result vector index by " << i << " = " << resultBuffer[i] << " is success." << endl;
        }
    }

    //列出失敗的test
    void ListFailureTest()
    {
        char falseChar[10];
        char ignoreChar[10];
         char resultCharArr[100];
         char outputCharArr[100];
        strcpy(falseChar, "false");
        strcpy(ignoreChar, "ignore");
         for(int i  = 0; i < resultBuffer.size(); i++)
        {
             if(outputBuffer[i].size() < 1)continue;//忽略掉沒有輸出的

             strcpy(resultCharArr, resultBuffer[i].c_str());
             if(MatchChar(resultCharArr, falseChar) != 1)continue;//忽略掉不是false的

             strncpy(outputCharArr, outputBuffer[i].c_str(), 6);
             if(MatchChar(outputCharArr, ignoreChar) == 1)continue;//忽略掉是ignore的

            cout << "the result vector index by " << i << " = " << resultBuffer[i] << " is failure." << endl;
        }
    }

    void ListIgnoreTest()
    {
        char ignoreChar[10];
        char outputCharArr[100];
        strcpy(ignoreChar, "ignore");
        for(int i  = 0; i < resultBuffer.size(); i++)
        {
             if(outputBuffer[i].size() < 1)continue;//忽略掉沒有輸出的
             strncpy(outputCharArr, outputBuffer[i].c_str(), 6);
             if(MatchChar(outputCharArr, ignoreChar) != 1)continue;//忽略掉不是ignore的
            cout << "the result vector index by " << i << " is " << outputBuffer[i] << endl;
        }
    }

    int MatchString(string str1, string str2)//output,test
    {
        int result = 1;
        int charIndex = 0;
        char p1 = str1[charIndex];
        char p2 = str2[charIndex];
        if(p1 == '\0')return -1;
        while(p1 != '\0')
        {
            //cout << "p1 = " << p1 << endl;
            //cout << "p2 = " << p2 << endl;
            if(p1 != p2)return -1;
            if(p2 == '\0')return -1;
           charIndex++;
            p1 = str1[charIndex];
            p2 = str2[charIndex];
        }
        return result;
    }

    //對比文字(Char) 回傳1一樣，回傳-1不一樣
    int MatchChar(char *str1, char *str2)//output,test
    {
        int result = 1;
        int charIndex = 0;
        char p1 = str1[charIndex];
        char p2 = str2[charIndex];
        if(p1 == '\0')return -1;
        while(p1 != '\0')
        {
            //cout << "p1 = " << p1 << endl;
            //cout << "p2 = " << p2 << endl;
            if(p1 != p2)return -1;
            if(p2 == '\0')return -1;
           charIndex++;
            p1 = str1[charIndex];
            p2 = str2[charIndex];
        }
        return result;
    }

    //關閉文字檔
    void Close()
    {
        _inputFile.close();
        _outputFile.close();
        _resultFile.close();
    }

    //輸入檔名，取直到'.'前的字做output
    char* FileNameWithoutType(char* input)
    {
        int charIndex = 0;
        char p1 = input[charIndex];
        char *output;
        while(p1 != '.')
        {
            output[charIndex] = input[charIndex];
            cout << "test charIndex = " << charIndex << endl;
            cout << "output = " << output << endl;
           charIndex++;
           p1 = input[charIndex];
        }
        return output;
    }

    //新增以圖片名稱為資料夾名稱的路徑
    char* AddImageFolderPath(char* oldPath, char* imgName)
    {
        char *newPath;
        newPath = new char[50];
        strcpy(newPath, oldPath);
        strcat(newPath, imgName);
        strcat(newPath, "/");
        return newPath;
    }

    //將字串1-1000修改成0001-1000
    char *FillDigit(char *input)
    {
        int digitalShift = 0;
        int zeroShift = 0;
        while(input[digitalShift] != '\0')
            digitalShift++;
        zeroShift = 4 - digitalShift;

        char* output;
        output = new char[5];
        int index = 0;

        for(index = 0; index < zeroShift; index++)
            output[index] = '0';

        for(int digit = 0; digit < digitalShift; digit++)
        {
            output[index] = input[digit];
            index++;
        }
        output[4] = '\0';
        return output;
    }

    //輸出路徑整歸
    char* ImageOutputPath(char* basePath, char* folderPath, char* fileName)
    {
        char* output;
        output = new char[50];
        strcpy(output, basePath);
        strcat(output, folderPath);
        strcat(output, fileName);
        return output;
    }
};
