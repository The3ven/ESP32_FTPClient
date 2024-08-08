#pragma once
#ifndef ESP32_FTP_CLIENT_H_
#define ESP32_FTP_CLIENT_H_
#include <NetworkClient.h>

class ESP32_FTPClient
{
private:
  void WriteClientBuffered(NetworkClient *cli, unsigned char *data, int dataLength);
  char outBuf[128];
  unsigned char outCount;
  NetworkClient client;
  NetworkClient dclient;
  uint8_t verbose;

  template <typename T>
  void FTPdbg(T msg)
  {
    if (verbose == 2)
      Serial.print(msg);
  }

  template <typename T>
  void FTPdbgn(T msg)
  {
    if (verbose == 2)
      Serial.println(msg);
  }

  template <typename T>
  void FTPerr(T msg)
  {
    if (verbose == 1 || verbose == 2)
      Serial.print(msg);
  }

  char *userName;
  char *passWord;
  char *serverAdress;
  uint16_t port;
  bool _isConnected = false;
  unsigned char clientBuf[1500];
  size_t bufferSize = 1500;
  uint16_t timeout = 10000;
  uint16_t errCode = 0;
  String errMsg = "";
  NetworkClient *GetDataClient();
  NetworkClient *GetCmdClient();

public:
  ESP32_FTPClient();
  ESP32_FTPClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  ESP32_FTPClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  void setConfig(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  void setConfig(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  void OpenConnection();
  void CloseConnection();
  bool isConnected();
  void NewFile(const char *fileName);
  void AppendFile(char *fileName);
  void WriteData(unsigned char *data, int dataLength);
  void CloseFile();
  void GetFTPAnswer(char *result = NULL, int offsetStart = 0);
  void GetLastModifiedTime(const char *fileName, char *result);
  void RenameFile(char *from, char *to);
  void Write(const char *str);
  void InitFile(const char *type);
  void ChangeWorkDir(const char *dir);
  void DeleteFile(const char *file);
  void MakeDir(const char *dir);
  void ContentList(const char *dir, String *list);
  void ContentListWithListCommand(const char *dir, String *list);
  void DownloadString(const char *filename, String &str);
  void DownloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART = false);
  String lastErrMsg();
  uint16_t lastErrCode();
};
#endif