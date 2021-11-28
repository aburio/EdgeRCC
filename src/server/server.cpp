#include "server.h"

#include <ESPmDNS.h>

#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

#include <Update.h>
#include <WebServer.h>

/* local types & variables */
WebServer server(80);

/* private functions prototypes */
String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);

/* public functions */
void server_init(const String local_hostname, bool OTA)
{
  MDNS.begin(local_hostname.c_str());

  FILESYSTEM.begin();
  {
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while (file)
    {
      String fileName = file.name();
      size_t fileSize = file.size();
      log_v("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      file = root.openNextFile();
    }
  }

  server.onNotFound([]()
                    {
                      if (!handleFileRead(server.uri()))
                      {
                        server.send(404, "text/plain", "FileNotFound");
                      }
                    });

  server.on("/function", []()
            {
              String param_name = server.arg("param");
              String param_value = server.arg("value");

              if (server.method() == HTTP_POST && param_name == "front-cabine")
              {
                if (param_value == "on")
                {
                  server.sendHeader("Connection", "close");
                  server.send(200, "text/plain", "Success");
                  return;
                }
                else if (param_value == "off")
                {
                  server.sendHeader("Connection", "close");
                  server.send(200, "text/plain", "Success");
                  return;
                }

                server.sendHeader("Connection", "close");
                server.send(400, "text/plain", "Bad request !");
              }
              else
              {
                server.sendHeader("Connection", "close");
                server.send(200, "text/plain", "GPIO X value : digitalRead(X)");
              }
            });

  server.on("/motor", []()
            {
              String param_name = server.arg("param");
              String param_value = server.arg("value");

              if (server.method() == HTTP_POST)
              {
                if (param_name == "direction")
                {
                  if (param_value == "forward")
                  {
                    digitalWrite(INPUT1_PIN, HIGH);
                    digitalWrite(INPUT2_PIN, LOW);
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/plain", "Success");
                    return;
                  }
                  else if (param_value == "stop")
                  {
                    digitalWrite(INPUT1_PIN, LOW);
                    digitalWrite(INPUT2_PIN, LOW);
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/plain", "Success");
                    return;
                  }
                  else if (param_value == "backward")
                  {
                    digitalWrite(INPUT1_PIN, LOW);
                    digitalWrite(INPUT2_PIN, HIGH);
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/plain", "Success");
                    return; 
                  }
                }
                else if (param_name == "traction")
                {
                  ledcWrite(0, param_value.toInt());
                  server.sendHeader("Connection", "close");
                  server.send(200, "text/plain", "Success");
                  return;
                }

                server.sendHeader("Connection", "close");
                server.send(400, "text/plain", "Bad request !");
              }
            });

  server.on(
      "/update", HTTP_POST,
      []()
      {
        // TODO : Remove reboot when uploading filesystem ?
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      },
      []()
      {
        HTTPUpload &upload = server.upload();
        uint8_t cmd = U_FLASH;

        if (upload.status == UPLOAD_FILE_START)
        {
          log_v("Update: %s\n", upload.filename.c_str());
          if (upload.filename == "filesystem.bin")
          {
            cmd = U_SPIFFS;
          }

          if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
          { //start with max available size
            log_v("%s", Update.errorString());
            Update.end(false);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            log_v("%s", Update.errorString());
            Update.end(false);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { //true to set the size to the current progress
            log_v("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            log_v("%s", Update.errorString());
            Update.end(false);
          }
        }
        else
        {
          log_v("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
        }
      });

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void server_task(void *pvParameters)
{
  for (;;)
  {
    server.handleClient();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

/* private functions */
String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename)
{
  if (filename.endsWith(".htm"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".html"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    return "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    return "application/javascript";
  }
  else if (filename.endsWith(".png"))
  {
    return "image/png";
  }
  else if (filename.endsWith(".gif"))
  {
    return "image/gif";
  }
  else if (filename.endsWith(".jpg"))
  {
    return "image/jpeg";
  }
  else if (filename.endsWith(".ico"))
  {
    return "image/x-icon";
  }
  else if (filename.endsWith(".xml"))
  {
    return "text/xml";
  }
  else if (filename.endsWith(".json"))
  {
    return "text/json";
  }

  return "text/plain";
}

bool handleFileRead(String path)
{
  log_v("handleFileRead: %s", path.c_str());
  if (path.endsWith("/"))
  {
    path += "index.html";
  }
  String contentType = getContentType(path);
  if (FILESYSTEM.exists(path))
  {
    File file = FILESYSTEM.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}