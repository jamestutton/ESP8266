#include <FS.h>
#include "WebServer.h"
#include "Wifi.h"
#include "Helper.h"
#include "Log.h"

extern char device_name[];

char dns_host[32];
const char* webserver_username = "admin";
const char* webserver_password = "admin";

ESP8266WebServer *s_httpServer = NULL;
File fsUploadFile;

void http_handle_reconfig(void) {
  if (s_httpServer == NULL) {
    return;
  }
  if (webserver_username != NULL && webserver_password != NULL && !s_httpServer->authenticate(webserver_username, webserver_password))
    return s_httpServer->requestAuthentication();
  s_httpServer->send(200, "text/html", "Clear configration and reboot. Please connect to AP and reconfig!");
  s_httpServer->handleClient();
  struct station_config conf;
  *conf.ssid = 0;
  *conf.password = 0;
  wifi_station_set_config(&conf);
  WiFi.enableSTA(false);
  delay(100);
  ESP.reset();
  delay(100);
}

void http_handle_reboot(void) {
  if (s_httpServer == NULL) {
    return;
  }
  if (webserver_username != NULL && webserver_password != NULL && !s_httpServer->authenticate(webserver_username, webserver_password))
    return s_httpServer->requestAuthentication();
  s_httpServer->send(200, "text/html", "System reboot!");
  s_httpServer->handleClient();
  delay(100);
  ESP.reset();
  delay(100);
}

void http_handle_info(void) {
  if (s_httpServer == NULL) {
    return;
  }
  String temp;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  temp = "Build data: " __DATE__ " " __TIME__ "<br>";
  temp += "Host Name: ESP_" + macToName(mac) + "_" + device_name + "<br>";
  temp += "IP: " + WiFi.localIP().toString() + "<br>";
  temp += "RSSI: ";
  temp += WiFi.RSSI();
  temp += "<br>";
  s_httpServer->send(200, "text/html", temp);
  s_httpServer->handleClient();
}

void http_handle_log(void) {
  if (s_httpServer == NULL) {
    return;
  }
  String log;
  gLog.export_log(log);
  s_httpServer->send(200, "text/html", "<!DOCTYPE html><head>Log</head><title>Log</title><br><br><textarea name=""text"" readonly=""readonly"" wrap=""off"" cols=""150"" rows=""100"" id=""syslog"">" + log /*+ WMDEBUG*/ + "</textarea>");
  //log.replace("\n", "<br/><br>");
  //s_httpServer->send(200, "text/html", "<!DOCTYPE html><html><head><style>table, th, td {border: 1px solid black; border-collapse: collapse;}</style><title>Log</title></head><body><h1>Logs</h1><table><tr><td>" + log + "<br/></td></tr></table></body></html>");
  s_httpServer->handleClient();
}

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
  if(s_httpServer->hasArg("download")) return "application/octet-stream";
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
  else if(filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool handleFileRead(String path){
  gLog.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = s_httpServer->streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(s_httpServer->uri() != "/edit") return;
  HTTPUpload& upload = s_httpServer->upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    gLog.print("handleFileUpload Name: "); gLog.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //gLog.print("handleFileUpload Data: "); gLog.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    gLog.print("handleFileUpload Size: "); gLog.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(s_httpServer->args() == 0) return s_httpServer->send(500, "text/plain", "BAD ARGS");
  String path = s_httpServer->arg(0);
  gLog.println("handleFileDelete: " + path);
  if(path == "/")
    return s_httpServer->send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return s_httpServer->send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  s_httpServer->send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(s_httpServer->args() == 0)
    return s_httpServer->send(500, "text/plain", "BAD ARGS");
  String path = s_httpServer->arg(0);
  gLog.println("handleFileCreate: " + path);
  if(path == "/")
    return s_httpServer->send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return s_httpServer->send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return s_httpServer->send(500, "text/plain", "CREATE FAILED");
  s_httpServer->send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!s_httpServer->hasArg("dir")) {s_httpServer->send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = s_httpServer->arg("dir");
  gLog.println("handleFileList: " + path);
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
  s_httpServer->send(200, "text/json", output);
}

const char *edit_htm_page =
R"(<!DOCTYPE html>
<html lang="en">
   <head>
      <title>ESP Editor</title>
      <style type="text/css" media="screen">.cm{z-index:300;position:absolute;left:5px;border:1px solid #444;background-color:#F5F5F5;display:none;box-shadow:0 0 10px rgba(0,0,0,.4);font-size:12px;font-family:sans-serif;font-weight:700}.cm ul{list-style:none;top:0;left:0;margin:0;padding:0}.cm li{position:relative;min-width:60px;cursor:pointer}.cm span{color:#444;display:inline-block;padding:6px}.cm li:hover{background:#444}.cm li:hover span{color:#EEE}.tvu ul,.tvu li{padding:0;margin:0;list-style:none}.tvu input{position:absolute;opacity:0}.tvu{font:normal 12px Verdana,Arial,Sans-serif;-moz-user-select:none;-webkit-user-select:none;user-select:none;color:#444;line-height:16px}.tvu span{margin-bottom:5px;padding:0 0 0 18px;cursor:pointer;display:inline-block;height:16px;vertical-align:middle;background:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAQAAAC1+jfqAAAABGdBTUEAAK/INwWK6QAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAADoSURBVBgZBcExblNBGAbA2ceegTRBuIKOgiihSZNTcC5LUHAihNJR0kGKCDcYJY6D3/77MdOinTvzAgCw8ysThIvn/VojIyMjIyPP+bS1sUQIV2s95pBDDvmbP/mdkft83tpYguZq5Jh/OeaYh+yzy8hTHvNlaxNNczm+la9OTlar1UdA/+C2A4trRCnD3jS8BB1obq2Gk6GU6QbQAS4BUaYSQAf4bhhKKTFdAzrAOwAxEUAH+KEM01SY3gM6wBsEAQB0gJ+maZoC3gI6iPYaAIBJsiRmHU0AALOeFC3aK2cWAACUXe7+AwO0lc9eTHYTAAAAAElFTkSuQmCC) no-repeat;background-position:0 0}.tvu span:hover{text-decoration:underline}@media screen and (-webkit-min-device-pixel-ratio:0){.tvu{-webkit-animation:webkit-adjacent-element-selector-bugfix infinite 1s}}@-webkit-keyframes webkit-adjacent-element-selector-bugfix{from{padding:0}to{padding:0}}#uploader{position:absolute;top:0;right:0;left:0;height:28px;line-height:24px;padding-left:10px;background-color:#444;color:#EEE}#tree{position:absolute;top:28px;bottom:0;left:0;width:160px;padding:8px}#editor,#preview{position:absolute;top:28px;right:0;bottom:0;left:160px;border-left:1px solid #EEE}#preview{background-color:#EEE;padding:5px}</style>
      <script>
         eval(function(p,a,c,k,e,r){e=function(c){return(c<a?'':e(parseInt(c/a)))+((c=c%a)>35?String.fromCharCode(c+29):c.toString(36))};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('3 2d(d,f,g){5 h;5 i=6.r("2e");i.1a="1e";i.2o=w;i.G="1C";6.v(d).s(i);5 j=6.r("2e");j.K="3h-17";j.1a="13";j.G="17";j.2G="/";6.v(d).s(j);5 k=6.r("2i");k.J=\'2t\';6.v(d).s(k);5 l=6.r("2i");l.J=\'37\';6.v(d).s(l);3 10(){7(h.Y==4){7(h.B!=W)1t("1s["+h.B+"]: "+h.U);L{f.1E(j.t)}}}3 1H(p){h=u T();h.R=10;5 a=u 1c();a.1d("17",p);h.P("2s","/X");h.Q(a)}l.H=3(e){7(j.t.3n(".")===-1)q;1H(j.t);g.15(j.t)};k.H=3(e){7(i.16.C===0){q}h=u T();h.R=10;5 a=u 1c();a.1d("1C",i.16[0],j.t);h.P("1M","/X");h.Q(a)};i.3j=3(e){7(i.16.C===0)q;5 a=i.16[0].G;5 b=/(?:\\.([^.]+))?$/.E(a)[1];5 c=/(.*)\\.[^.]+$/.E(a)[1];7(z c!==y){a=c}7(z b!==y){7(b==="1p")b="1b";L 7(b==="3c")b="1N";a=a+"."+b}7(j.t==="/"||j.t.1X("/")===0){j.t="/"+a}L{j.t=j.t.3k(0,j.t.1X("/")+1)+a}}}3 25(k,l){5 m=6.v("29");5 n=6.r("2f");n.2g="2I";6.v(k).s(n);3 2h(a){6.v(\'2n-3f\').2l=a+"?2n=A"}3 1m(a){6.v("1l").D.O="1V";m.D.O="18";m.J=\'<2y 2l="\'+a+\'?2B=\'+2E.2F()+\'" D="20-2H:22%; 20-2K:22%; 2R:2Y; O:18;" />\'}3 23(a,b){5 c=6.r("24");a.s(c);5 d=6.r("19");c.s(d);7(1k(b)){d.J="<x>3x</x>";d.H=3(e){l.15(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}L 7(1q(b)){d.J="<x>2u</x>";d.H=3(e){1m(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}5 f=6.r("19");c.s(f);f.J="<x>2v</x>";f.H=3(e){2h(b);7(6.9.11(\'N\').C>0)6.9.I(a)};5 g=6.r("19");c.s(g);g.J="<x>2w</x>";g.H=3(e){2j(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}3 2k(e,a,b){5 c=6.r("2f");5 d=6.9.1f?6.9.1f:6.1D.1f;5 f=6.9.1g?6.9.1g:6.1D.1g;5 g=1F.1h+f;5 h=1F.1i+d;c.2g=\'N\';c.D.O=\'18\';c.D.2Q=g+\'1I\';c.D.2T=h+\'1I\';23(c,a);6.9.s(c);5 i=c.2U;5 j=c.2W;c.2X=3(e){7(e.1h<g||e.1h>(g+i)||e.1i<h||e.1i>(h+j)){7(6.9.11(\'N\').C>0)6.9.I(c)}}}3 1J(a,b,c){5 d=6.r("19");d.K=(((a=="/")?"":a)+"/"+b);5 f=6.r("x");f.30=b;d.s(f);d.H=3(e){7(1k(d.K.1K())){l.15(d.K)}L 7(1q(d.K.1K())){1m(d.K)}};d.38=3(e){e.3a();e.3b();2k(e,d.K,A)};q d}3 1L(a,b,c){5 d=6.r("24");a.s(d);5 e=c.C;3d(5 i=0;i<e;i++){7(c[i].1a==="1e")d.s(1J(b,c[i].G,c[i].3e))}}3 1k(a){5 b=/(?:\\.([^.]+))?$/.E(a)[1];7(z b!==y){1j(b){8"1O":8"1b":8"1P":8"c":8"1Q":8"1R":8"1S":q A}}q w}3 1q(a){5 b=/(?:\\.([^.]+))?$/.E(a)[1];7(z b!==y){1j(b){8"2q":8"1N":8"2r":q A}}q w}1T.1E=3(a){n.I(n.1U[0]);F(n,"/")};3 1W(a){q 3(){7(o.Y==4){7(o.B!=W){1t("1s["+o.B+"]: "+o.U)}L{n.I(n.1U[0]);F(n,"/")}}}}3 2j(a){o=u T();o.R=1W(a);5 b=u 1c();b.1d("17",a);o.P("2x","/X");o.Q(b)}3 1Y(a,b){q 3(){7(o.Y==4){7(o.B==W)1L(a,b,2z.2A(o.U))}}}3 F(a,b){o=u T(a,b);o.R=1Y(a,b);o.P("1Z","/2C?2D="+b,A);o.Q(1n)}F(n,"/");q 1T}3 21(e,f,g,h,i){3 1o(a){5 b="V";5 c=/(?:\\.([^.]+))?$/.E(a)[1];7(z c!==y){1j(c){8"1O":b="V";12;8"1b":b="1p";12;8"1P":b="2J";12;8"c":b="1r";12;8"1Q":b="1r";12;8"1R":8"2L":8"2M":8"1p":8"2N":8"1S":b=c}}q b}7(z f==="y")f="/2O.1b";7(z g==="y"){g=1o(f)}7(z h==="y")h="2P";7(z i==="y"){i="13/"+g;7(g==="1r")i="13/V"}5 j=1n;5 k=14.X(e);3 10(){7(j.Y==4){7(j.B!=W)1t("1s["+j.B+"]: "+j.U)}}3 26(a,b,c){j=u T();j.R=10;5 d=u 1c();d.1d("1C",u 2S([b],{1a:c}),a);j.P("1M","/X");j.Q(d)}3 27(){7(j.Y==4){6.v("29").D.O="1V";6.v("1l").D.O="18";7(j.B==W)k.28(j.U);L k.28("");k.2V()}}3 F(a){j=u T();j.R=27;j.P("1Z",a,A);j.Q(1n)}7(g!=="V")k.M().2a("14/2b/"+g);k.2Z("14/2c/"+h);k.$31=32;k.M().33(A);k.M().34(2);k.35(A);k.36(w);k.1u.1v({G:\'39\',1w:{1x:\'1y-S\',1z:\'1A-S\'},E:3(a){26(f,a.3g()+"",i)},1B:w});k.1u.1v({G:\'3i\',1w:{1x:\'1y-Z\',1z:\'1A-Z\'},E:3(a){a.M().2m().3l(w)},1B:w});k.1u.1v({G:\'3m\',1w:{1x:\'1y-1G-Z\',1z:\'1A-1G-Z\'},E:3(a){a.M().2m().3o(w)},1B:w});F(f);k.15=3(a){f=a;g=1o(f);i="13/"+g;7(g!=="V")k.M().2a("14/2b/"+g);F(f)};q k}3 3p(){5 c={};5 d=3q.3r.3s.3t(/[?&]+([^=&]+)=([^&]*)/3u,3(m,a,b){c[a]=b});5 e=21("1l",c.1e,c.3v,c.2c);5 f=25("3w",e);2d("2p",f,e)};',62,220,'|||function||var|document|if|case|body|||||||||||||||xmlHttp||return|createElement|appendChild|value|new|getElementById|false|span|undefined|typeof|true|status|length|style|exec|httpGet|name|onclick|removeChild|innerHTML|id|else|getSession|cm|display|open|send|onreadystatechange||XMLHttpRequest|responseText|plain|200|edit|readyState||httpPostProcessRequest|getElementsByClassName|break|text|ace|loadUrl|files|path|block|li|type|htm|FormData|append|file|scrollTop|scrollLeft|clientX|clientY|switch|isTextFile|editor|loadPreview|null|getLangFromFilename|html|isImageFile|c_cpp|ERROR|alert|commands|addCommand|bindKey|win|Ctrl|mac|Command|readOnly|data|documentElement|refreshPath|event|Shift|createPath|px|createTreeLeaf|toLowerCase|addList|POST|jpg|txt|js|json|css|xml|this|childNodes|none|delCb|lastIndexOf|getCb|GET|max|createEditor|100|fillFileMenu|ul|createTree|httpPost|httpGetProcessRequest|setValue|preview|setMode|mode|theme|createFileUploader|input|div|className|loadDownload|button|httpDelete|showContextMenu|src|getUndoManager|download|multiple|uploader|png|gif|PUT|Upload|Preview|Download|Delete|DELETE|img|JSON|parse|_cb|list|dir|Date|now|defaultValue|width|tvu|javascript|height|scss|php|json|index|textmate|left|margin|Blob|top|offsetWidth|clearSelection|offsetHeight|onmouseout|auto|setTheme|innerText|blockScrolling|Infinity|setUseSoftTabs|setTabSize|setHighlightActiveLine|setShowPrintMargin|Create|oncontextmenu|saveCommand|preventDefault|stopPropagation|jpeg|for|size|frame|getValue|upload|undoCommand|onchange|substring|undo|redoCommand|indexOf|redo|onBodyLoad|window|location|href|replace|gi|lang|tree|Edit'.split('|'),0,{}))
      </script><script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.1.9/ace.js" type="text/javascript" charset="utf-8"></script>
   </head>
   <body onload="onBodyLoad();">
      <div id="uploader"></div>
      <div id="tree"></div>
      <div id="editor"></div>
      <div id="preview" style="display:none;"></div>
      <iframe id=download-frame style='display:none;'></iframe>
   </body>
</html>
)";

int setup_web_server(ESP8266WebServer *httpServer) {
  s_httpServer = httpServer;
  if (httpServer) {
    sprintf(dns_host, "ESP_%s", device_name);
    MDNS.begin(dns_host);

    httpServer->on("/reconfig", HTTP_GET, http_handle_reconfig);
    httpServer->on("/reboot", HTTP_GET, http_handle_reboot);
    httpServer->on("/info", HTTP_GET, http_handle_info);
    httpServer->on("/log", HTTP_GET, http_handle_log);

    //list directory
    httpServer->on("/list", HTTP_GET, handleFileList);
    //load editor
    httpServer->on("/edit", HTTP_GET, [](){s_httpServer->send(200, "text/html", edit_htm_page);});
    //create file
    httpServer->on("/edit", HTTP_PUT, handleFileCreate);
    //delete file
    httpServer->on("/edit", HTTP_DELETE, handleFileDelete);
    //first callback is called after the request has ended with all parsed arguments, second callback handles file uploads at that location
    httpServer->on("/edit", HTTP_POST, [](){ s_httpServer->send(200, "text/plain", ""); }, handleFileUpload);
    //called when the url is not defined here, use it to load content from SPIFFS
    httpServer->onNotFound([](){ if(!handleFileRead(s_httpServer->uri())) s_httpServer->send(404, "text/plain", "FileNotFound"); });
  
    httpServer->begin();

    MDNS.addService("http", "tcp", 80);
  }
  return 0;
}

void default_web_server_setup_callback(ESP8266WebServer *webserver)
{
  setup_web_server(webserver);
}

