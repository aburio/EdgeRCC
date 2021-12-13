#include "server.h"

#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <Update.h>

#include "controller/controller.h"

/* local types & variables */
AsyncWebServer server(80);
AsyncWebSocket socket("/ws");
bool reboot = false;

/* private functions prototypes */
void requestFileUploadStart(AsyncWebServerRequest *request, const String filename, size_t index, uint8_t *data, size_t len, bool final);
void requestFileUploadEnd(AsyncWebServerRequest *request);
void requestNotFound(AsyncWebServerRequest *request);
void socketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void socketMessageParser(String message);

/* public functions */
void serverInit(const String local_hostname, bool OTA)
{
  // Filesystem
  FILESYSTEM.begin();

  // Server
  socket.onEvent(socketEvent);
  server.addHandler(&socket);
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.onNotFound(requestNotFound);
  server.on("/update", HTTP_POST, requestFileUploadEnd, requestFileUploadStart);
  server.begin();

  // MDNS
  MDNS.begin(local_hostname.c_str());
  MDNS.addService("http", "tcp", 80);
}

void serverTask(void *pvParameters)
{
  for (;;)
  {
    socket.cleanupClients();

    if (reboot == true)
    {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      ESP.restart();
    }

    vTaskDelay(300 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

/* private functions */
void requestFileUploadStart(AsyncWebServerRequest *request, const String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  uint8_t cmd = U_FLASH;

  if (index == 0)
  {
    log_v("Update: %s\n", filename.c_str());
    if (filename == "filesystem.bin")
    {
      cmd = U_SPIFFS;
    }

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
    {
      log_v("%s", Update.errorString());
      Update.end(false);
      request->send(503, "text/plain", "Internal error !");
    }
  }

  if (!Update.hasError())
  {
    if (Update.write(data, len) != len)
    {
      log_v("%s", Update.errorString());
      Update.end(false);
      request->send(503, "text/plain", "Internal error : " + String(Update.errorString()));
    }
    else
    {
      socket.printfAll("%zu", Update.progress());
    }
  }

  if (final)
  {
    if (Update.end(true))
    {
      log_v("Update Success: %uB\n", index + len);
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "60");
      response->addHeader("Location", "/");
      request->send(response);
    }
    else
    {
      log_v("%s", Update.errorString());
      Update.end(false);
      request->send(503, "text/plain", "Internal error : " + String(Update.errorString()));
    }
  }
}

void requestFileUploadEnd(AsyncWebServerRequest *request)
{
  reboot = !Update.hasError();
  AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
  response->addHeader("Refresh", "60");
  response->addHeader("Location", "/");
  request->send(response);
}

void requestNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "FileNotFound");
}

void socketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    log_v("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    log_v("ws[%s][%u] disconnect\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    log_v("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    log_v("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";

    if (info->final && info->index == 0 && info->len == len)
    {
      // the whole message is in a single frame and we got all of it's data
      log_v("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < info->len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < info->len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      socketMessageParser(msg);
      client->text(msg);
    }
    else
    {
      // message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
        if (info->num == 0)
          log_v("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
        log_v("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      log_v("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      log_v("%s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
        log_v("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if (info->final)
        {
          log_v("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
          client->text("I got your text message");
        }
      }
    }
  }
}

void socketMessageParser(String message)
{
  StaticJsonDocument<256> json;
  deserializeJson(json, message);
  JsonObject obj = json.as<JsonObject>();
  struct controllermessage ctrl_message;
  extern QueueHandle_t controllerqueue;

  if (obj.getMember("function") != NULL)
  {
    ctrl_message.function_name = obj.getMember("function").as<String>();
    ctrl_message.function_value = obj.getMember("value").as<int32_t>();

    log_v("function : %s", ctrl_message.function_name);
    log_v("value : %u", ctrl_message.function_value);

    xQueueSend(controllerqueue, &ctrl_message, portMAX_DELAY);

    if (obj.getMember("function") == "front-cab")
    {
      digitalWrite(FRONT_CAB, !obj.getMember("value"));
    }
    else if (obj.getMember("function") == "front-white")
    {
      digitalWrite(FRONT_WHITE, !obj.getMember("value"));
    }
    else if (obj.getMember("function") == "front-red")
    {
      digitalWrite(FRONT_RED, !obj.getMember("value"));
    }
    else if (obj.getMember("function") == "rear-cab")
    {
      digitalWrite(REAR_CAB, !obj.getMember("value"));
    }
    else if (obj.getMember("function") == "rear-white")
    {
      digitalWrite(REAR_WHITE, !obj.getMember("value"));
    }
    else if (obj.getMember("function") == "rear-red")
    {
      digitalWrite(REAR_RED, !obj.getMember("value"));
    }
  }
}