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
                for(int i  = 0; i < imgBuffer.size(); i++)
                {
                    cout << "image vector " << i << " = " << imgBuffer[i] << endl;
                }
                break;
            }
            case 't':
            {
                for(int i  = 0; i < testBuffer.size(); i++)
                {
                    cout << "test vector " << i << " = " << testBuffer[i] << endl;
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

    //把資料寫進文件檔中
    void WriteDownOutput()
    {
        for(int i = 0; i < outputBuffer.size(); i++)
        {
            _outputFile << outputBuffer[i] << endl;
        }
    }

    //關閉文字檔
    void Close()
    {
        _inputFile.close();
        _outputFile.close();
        _resultFile.close();
    }
};
