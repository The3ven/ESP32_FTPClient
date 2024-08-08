#include "ESP32_FTPClient.h"

ESP32_FTPClient::ESP32_FTPClient(){}

ESP32_FTPClient::ESP32_FTPClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = _port;
  timeout = _timeout;
  verbose = _verbose;
}

ESP32_FTPClient::ESP32_FTPClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = 21;
  timeout = _timeout;
  verbose = _verbose;
}

void ESP32_FTPClient::setConfig(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = _port;
  timeout = _timeout;
  verbose = _verbose;
}

void ESP32_FTPClient::setConfig(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = 21;
  timeout = _timeout;
  verbose = _verbose;
}

NetworkClient *ESP32_FTPClient::GetDataClient()
{
  return &dclient;
}

NetworkClient *ESP32_FTPClient::GetCmdClient()
{
  return &client;
}

bool ESP32_FTPClient::isConnected()
{
  if (!_isConnected)
  {
    FTPerr("FTP error: ");
    FTPerr(outBuf);
    FTPerr("\n");
  }

  return _isConnected;
}

void ESP32_FTPClient::GetLastModifiedTime(const char *fileName, char *result)
{
  FTPdbgn("Send MDTM");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("MDTM "));
  GetCmdClient()->println(F(fileName));
  GetFTPAnswer(result, 4);
}

void ESP32_FTPClient::WriteClientBuffered(NetworkClient *cli, unsigned char *data, int dataLength)
{
  if (!isConnected())
    return;

  size_t clientCount = 0;
  for (int i = 0; i < dataLength; i++)
  {
    clientBuf[clientCount] = data[i];
    // GetCmdClient() ->write(data[i])
    clientCount++;
    if (clientCount > bufferSize - 1)
    {
      cli->write(clientBuf, bufferSize);
      clientCount = 0;
    }
  }
  if (clientCount > 0)
  {
    cli->write(clientBuf, clientCount);
  }
}

void ESP32_FTPClient::GetFTPAnswer(char *result, int offsetStart)
{
  char thisByte;
  outCount = 0;

  unsigned long _m = millis();
  while (!GetCmdClient()->available() && millis() < _m + timeout)
    delay(1);

  if (!GetCmdClient()->available())
  {
    memset(outBuf, 0, sizeof(outBuf));
    strcpy(outBuf, "Offline");

    _isConnected = false;
    isConnected();
    return;
  }

  while (GetCmdClient()->available())
  {
    thisByte = GetCmdClient()->read();
    if (outCount < sizeof(outBuf))
    {
      outBuf[outCount] = thisByte;
      outCount++;
      outBuf[outCount] = 0;
    }
  }

  String res = String(outBuf);
  res.trim();
  int idxOfWhiteSpace = res.indexOf(" ");
  errCode = res.substring(0, idxOfWhiteSpace).toInt();
  errMsg = res.substring(idxOfWhiteSpace);
  errMsg.trim();
  res.toUpperCase(); // Uppercase response to do a Caseless Comparision

  if (res.indexOf("Connect") != -1 || res.indexOf("success") != -1)
  {
    _isConnected = true;
  }

  if (result != NULL)
  {
    FTPdbgn("Result start");
    // Deprecated // but fixed
    int j = 0;
    for (int i = offsetStart; i < sizeof(outBuf); i++)
    {
      result[j] = outBuf[i];
    }
    FTPdbg("Result: ");
    // Serial.write(result);
    FTPdbg(outBuf);
    FTPdbgn("Result end");
  }
}

void ESP32_FTPClient::WriteData(unsigned char *data, int dataLength)
{
  FTPdbgn(F("Writing"));
  if (!isConnected())
    return;
  WriteClientBuffered(&dclient, &data[0], dataLength);
}

void ESP32_FTPClient::CloseFile()
{
  FTPdbgn(F("Close File"));
  GetDataClient()->stop();

  if (!_isConnected)
    return;

  GetFTPAnswer();
}

void ESP32_FTPClient::Write(const char *str)
{
  FTPdbgn(F("Write File"));
  if (!isConnected())
    return;

  GetDataClient()->print(str);
}

void ESP32_FTPClient::CloseConnection()
{
  GetCmdClient()->println(F("QUIT"));
  GetCmdClient()->stop();
  FTPdbgn(F("Connection closed"));
}

void ESP32_FTPClient::OpenConnection()
{
  FTPdbg(F("Connecting to: "));
  FTPdbgn(serverAdress);
  if (GetCmdClient()->connect(serverAdress, port, timeout))
  {
    FTPdbgn(F("Command connected"));
  }

  GetFTPAnswer();

  FTPdbgn("Send USER");
  GetCmdClient()->print(F("USER "));
  GetCmdClient()->println(F(userName));
  GetFTPAnswer();

  FTPdbgn("Send PASSWORD");
  GetCmdClient()->print(F("PASS "));
  GetCmdClient()->println(F(passWord));
  GetFTPAnswer();

  FTPdbgn("Send SYST");
  GetCmdClient()->println(F("SYST"));
  GetFTPAnswer();
}

void ESP32_FTPClient::RenameFile(char *from, char *to)
{
  FTPdbgn("Send RNFR");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("RNFR "));
  GetCmdClient()->println(F(from));
  GetFTPAnswer();

  FTPdbgn("Send RNTO");
  GetCmdClient()->print(F("RNTO "));
  GetCmdClient()->println(F(to));
  GetFTPAnswer();
}

void ESP32_FTPClient::NewFile(const char *fileName)
{
  FTPdbgn("Send STOR");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("STOR "));
  GetCmdClient()->println(F(fileName));
  GetFTPAnswer();
}

void ESP32_FTPClient::InitFile(const char *type)
{
  FTPdbgn("Send TYPE");
  if (!isConnected())
    return;
  FTPdbgn(type);
  GetCmdClient()->println(F(type));
  GetFTPAnswer();

  FTPdbgn("Send PASV");
  GetCmdClient()->println(F("PASV"));
  GetFTPAnswer();

  char *tStr = strtok(outBuf, "(,");
  int array_pasv[6];
  for (int i = 0; i < 6; i++)
  {
    tStr = strtok(NULL, "(,");
    if (tStr == NULL)
    {
      FTPdbgn(F("Bad PASV Answer"));
      CloseConnection();
      return;
    }
    array_pasv[i] = atoi(tStr);
  }
  unsigned int hiPort, loPort;
  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  IPAddress pasvServer(array_pasv[0], array_pasv[1], array_pasv[2], array_pasv[3]);

  FTPdbg(F("Data port: "));
  hiPort = hiPort | loPort;
  FTPdbgn(hiPort);
  if (GetDataClient()->connect(pasvServer, hiPort, timeout))
  {
    FTPdbgn(F("Data connection established"));
  }
}

void ESP32_FTPClient::AppendFile(char *fileName)
{
  FTPdbgn("Send APPE");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("APPE "));
  GetCmdClient()->println(F(fileName));
  GetFTPAnswer();
}

void ESP32_FTPClient::ChangeWorkDir(const char *dir)
{
  FTPdbgn("Send CWD");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("CWD "));
  GetCmdClient()->println(F(dir));
  GetFTPAnswer();
}

void ESP32_FTPClient::DeleteFile(const char *file)
{
  FTPdbgn("Send DELE");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("DELE "));
  GetCmdClient()->println(F(file));
  GetFTPAnswer();
}

void ESP32_FTPClient::MakeDir(const char *dir)
{
  FTPdbgn("Send MKD");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("MKD "));
  GetCmdClient()->println(F(dir));
  GetFTPAnswer();
}

void ESP32_FTPClient::ContentList(const char *dir, String *list)
{
  char _resp[sizeof(outBuf)];
  uint16_t _b = 0;

  FTPdbgn("Send MLSD");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("MLSD"));
  GetCmdClient()->println(F(dir));
  GetFTPAnswer(_resp);

  // Convert char array to string to manipulate and find response size
  // each server reports it differently, TODO = FEAT
  // String resp_string = _resp;
  // resp_string.substring(resp_string.lastIndexOf('matches')-9);
  // FTPdbgn(resp_string);

  unsigned long _m = millis();
  while (!GetDataClient()->available() && millis() < _m + timeout)
    delay(1);

  while (GetDataClient()->available())
  {
    if (_b < 128)
    {
      list[_b] = GetDataClient()->readStringUntil('\n');
      // FTPdbgn(String(_b) + ":" + list[_b]);
      _b++;
    }
  }
}

void ESP32_FTPClient::ContentListWithListCommand(const char *dir, String *list)
{
  char _resp[sizeof(outBuf)];
  uint16_t _b = 0;

  FTPdbgn("Send LIST");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("LIST"));
  GetCmdClient()->println(F(dir));
  GetFTPAnswer(_resp);

  // Convert char array to string to manipulate and find response size
  // each server reports it differently, TODO = FEAT
  // String resp_string = _resp;
  // resp_string.substring(resp_string.lastIndexOf('matches')-9);
  // FTPdbgn(resp_string);

  unsigned long _m = millis();
  while (!GetDataClient()->available() && millis() < _m + timeout)
    delay(1);

  while (GetDataClient()->available())
  {
    if (_b < 128)
    {
      String tmp = GetDataClient()->readStringUntil('\n');
      list[_b] = tmp.substring(tmp.lastIndexOf(" ") + 1, tmp.length());
      // FTPdbgn(String(_b) + ":" + tmp);
      _b++;
    }
  }
}

void ESP32_FTPClient::DownloadString(const char *filename, String &str)
{
  FTPdbgn("Send RETR");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("RETR "));
  GetCmdClient()->println(F(filename));

  char _resp[sizeof(outBuf)];
  GetFTPAnswer(_resp);

  unsigned long _m = millis();
  while (!GetDataClient()->available() && millis() < _m + timeout)
    delay(1);

  while (GetDataClient()->available())
  {
    str += GetDataClient()->readString();
    delay(1);
  }
}

void ESP32_FTPClient::DownloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART)
{
  FTPdbgn("Send RETR");
  if (!isConnected())
    return;
  GetCmdClient()->print(F("RETR "));
  GetCmdClient()->println(F(filename));

  char _resp[sizeof(outBuf)];
  GetFTPAnswer(_resp);

  char _buf[2];

  unsigned long _m = millis();
  while (!GetDataClient()->available() && millis() < _m + timeout)
    delay(1);

  while (GetDataClient()->available())
  {
    if (!printUART)
      GetDataClient()->readBytes(buf, length);

    else
    {
      for (size_t _b = 0; _b < length; _b++)
      {
        GetDataClient()->readBytes(_buf, 1),
            Serial.print(_buf[0], HEX);
      }
    }
  }
}

String ESP32_FTPClient::lastErrMsg()
{
  return errMsg;
}
uint16_t ESP32_FTPClient::lastErrCode()
{
  return errCode;
}