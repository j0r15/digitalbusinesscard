#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>

#define DBG_OUTPUT_PORT Serial

const char *ssid = "\xF0\x9F\x92\xA1 Joris Vos \xF0\x9F\x8E\x9B";
const char *password = "";

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "\xF0\x9F\x92\xA1 Joris Vos \xF0\x9F\x8E\x9B";

const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"3; url=http://192.168.4.1/index.html\" /></head><body><p>One moment please...</p></body>";

// DNS server
const byte DNS_PORT = 53; //that port is for the redirects
DNSServer dnsServer;

//counter
int counter = 0;
int visit = 0;
int currentState = 0;


/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1); // note: update metaRefreshStr string if ip change!
IPAddress netMsk(255, 255, 255, 0);

// Web server
ESP8266WebServer server(80);

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (path == "/style.css") {
    visit = 1;
  }
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}


void setup(void){
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(false);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }



  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);


  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println ( WiFi.softAPIP() );

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  // Setup MDNS responder
  if (!MDNS.begin(myHostname)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }


  //SERVER INIT
  //list directory
  server.on("/listFiles", HTTP_GET, handleFileList);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(302, "text/html", metaRefreshStr);
  });


  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
// for activation blue onboard led  - uncomment next line
// pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void){
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();


//simple little counter
//if visit is true then + 1 in 192.168.4.1/onlyforme.html
if (visit == 1) { //counter is counting hits on the /style.css file
  visit = 0; // turn "visit" off
  counter = counter + 1;
  Serial.println(counter);
  // open file for writing
    File f = SPIFFS.open("/onlyforme.html", "w");
    if (!f) {
        Serial.println("file open failed");
    }
        f.println(counter);
  }
  else {
  currentState = 0;
  }
}
