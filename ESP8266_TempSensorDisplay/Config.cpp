#include "Config.h"
#include "Log.h"
#include "Cust.h"

//flag for saving data
bool shouldSaveConfig = false;

int load_config(void) {
  if (SPIFFS.begin()) {
    //gLog.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      //gLog.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        //gLog.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        //json.printTo(gLog); gLog.println("");
        if (json.success()) {
          //gLog.println("parsed json");
          load_user_config(json);
        } else {
          gLog.println("failed to load json config");
        }
      }
    }
  } else {
    gLog.println("failed to mount FS");
#if AUTO_FORMAT_FS
    gLog.println("Formatting FS");
    SPIFFS.format();
#endif
  }
  //end read
}

void save_config(void) {
  //save the custom parameters to FS
  gLog.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  save_user_config(json);

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    gLog.println("failed to open config file for writing");
  }

  //json.printTo(gLog); gLog.println();
  json.printTo(configFile);
  configFile.close();
  //end save
}

