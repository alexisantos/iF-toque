/*
  ACIONADOR TOQUE ESCOLAR ELETRÔNICO AUTOMÁTICO IFRN-SPP
  Funções e bibliotecas de apoio e referência
     Função timeToStudy: https://github.com/dvcirilo/alarme_ifrn
     Stuct tm:           https://cplusplus.com/reference/ctime/tm/
     Esp32 NTP client:   https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
     NTPSync Callback:   https://techtutorialsx.com/2021/09/03/esp32-sntp-additional-features/#Setting_the_SNTP_sync_interval
     Espressif:          https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html#_CPPv419sntp_sync_time_cb_t
     Espressif:          https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html
     Espressif:          https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html
     Arduinojson:        https://arduinojson.org
  Autor: Alex Santos - CTI/SPP (Outubro/2017)
  Último Update: Setembro/2022
*/

#include <WiFi.h>
#include "time.h"
#include <TimeLib.h>
#include <ArduinoJson.h> //https://arduinojson.org

#define SINETA 26 //mesma porta do led interno
#define SINETA_TESTE 27 //SEMPRE ON

#define LED_AZUL_STATUS 2 //mesma porta do led interno. Se on, hora ok, se off sem horário.

// Variáveis Globais:
// Wifi:
const char* ssid     = "SSID";
const char* password = "Senha wifi";

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset = -3;
const int   daylightOffset_sec = 0;
long int last_ntp_update = 0;
struct tm timeinfo;
time_t agora;

//Uptime
static uint32_t locUpTime = 0;
uint32_t prevMillis = millis();

// Para controle de aula e turno
byte atualAula = 0;                     // Armazena a Aula atual  : 1 a 6 ou 0, 10, 20 ou 30
byte proxAula = 0;                      // Armazena a Proxima aula: 1 a 6 (nao considera intervalo como aula)
byte turno = 0;                         // Armazena o turno atual

// Para exibicao em web: Controle de tempo para proximo toque
tmElements_t tmRestante;                 // Estrutura que armazena (hh, mm, ss) restante para o proximo toque

// Para exibicao da hora atual em hh:mm:ss
time_t prevDisplay = 0;                 // Controla para só exibir da data/hora no display/serial caso a hora atual seja diferente da atual


DynamicJsonDocument doc(1000);
WiFiServer server(80);

/* ---------Protótipos das funções: -------------------*/
boolean DiadeAula();                                            // Retorna true caso não seja sabado, domingo e feriados (Configurar feriados nesta função)
boolean timeToStudy(byte hora, byte minuto, byte segundo);      // Retorna true se estiver na hora de acionar o toque escolar

/*   Funções de conectividade wifi */
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);     //Funções de Wifi
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);                //Funções de Wifi
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);  //Funções de Wifi

/*   Formatação e exibicao de data/hora: */
void    Imprime_Info_DataHora_Serial();
String  diaSemana(byte dia);                                    // Recebe um numero (1-7) e retorna dia da semana em string (-,Domingo, Segunda, (...), Sabado)
String  Printzero(byte a);                                      // Recebe um número
void    printLocalTime();                                        // Exibe a data em diversos formatos

/*    Exibicao de informações da aula atual */
byte    AulaAtualNum ();                                        // Retorna um numero correspondente a aula atual: (1 a 6) ou (0, 10, 20 ou 30)
String  AulaStr (byte aula, byte opcao);                        // Retorna uma String com a aula atual formatada: '(1º-6º) horário', 'Intervalo', 'Troca de turno'
byte    ProximaAula (byte aula_atual);                          // Retorna um numero correspondente a próxima Aula: 1 a 6 (nao considera intervalo como aula)
String  HorarioAulaStr (byte aula, byte opcao);                 // Retorna uma String com o horário da hora atual ou da próxima Aula: '11:15 até 12:00'..
byte    Turno ();                                               // Retorna um número correspondente ao  turno atual (Manhã, Tarde ou Noite): 1, 2 ou 3
String  TurnoStr ();                                            // Retorna uma String correspondente ao turno atual (1, 2 ou 3)            : 'Manhã', 'Tarde" ou 'Noite'
String  TurnoStrRange();                                        // Retorna a duracao do turno atual: '07h as 12h', '13 as 18' ou '19h as 22h10min'

/*    Funcoes que retornam tempo no formato Unix epoch/Unix timestamp  */
unsigned long getTime();                                        // retorna o horario atual em unixtime
time_t  TempoProxAula ();                                       // Calcula o tempo para a proxima aula, retorna um epoch time deste tempo. Quebra esta hora em tmRestante.hora, min e s
time_t  HoraEpoch (byte hh, byte mm);                           // Converte uma hora e minuto fornecida em um epoch time do dia atual. Útil para comparar com now()

/*    Funcoes com rede e monitoramento*/
void    ServidorWeb  ();                                        // Monta um servidor web, exibe as informações de hora caso a hora tenha sido obtida
void    Uptime();                                               // Calcula o tempo que o dispositivo se encontra ligado
void    timeSyncCallback(struct timeval *tv);                   // Callback para obter momento de sincronização NTP


void setup(){  
  pinMode(LED_AZUL_STATUS,OUTPUT);
  pinMode(SINETA,OUTPUT);
  pinMode(SINETA_TESTE,OUTPUT);

  digitalWrite(LED_AZUL_STATUS, LOW);
  digitalWrite(SINETA_TESTE, HIGH); //sempre ligada
  digitalWrite(SINETA, LOW); // relé que controla a sineta / sirene / campainha escolar
  sntp_set_time_sync_notification_cb(timeSyncCallback);
    
  Serial.begin(115200);
  WiFi.disconnect(true); // delete old config
  delay(2000);
  
  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
 
  WiFi.begin(ssid, password);
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
    
  Serial.println("Iniciado");
  Serial.println("Wait for WiFi... ");

  delay(4000);
  server.begin();

  doc["projeto"]; 

  doc["projeto"]["nome"]       = "IF-Toque";
  doc["projeto"]["versao"]     = "v2.5";
  doc["projeto"]["inicio"]     = "Nov/2017";
  doc["projeto"]["compilacao"] = "27/09/2022";
  
  doc["sistema"];
  doc["sistema"]["mac"]  = WiFi.macAddress();
  doc["sistema"]["ip"]   = WiFi.localIP();

  doc["ntp"]["ntp_server"] = ntpServer;
  doc["ntp"]["ultima_sincr_unixtime"] = 0;
  
  // Init and get the time
  configTime(gmtOffset*3600, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop(){
  Uptime();
  Imprime_Info_DataHora_Serial();
  ServidorWeb();
  
  if (getTime()){
    digitalWrite(LED_AZUL_STATUS, HIGH);
    if (timeToStudy(timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec)){
      digitalWrite(SINETA, HIGH); // relé que controla a sineta / sirene / campainha escolar
      Serial.println("Toque... ");
    }
    else
      digitalWrite(SINETA, LOW); // relé que controla a sineta / sirene / campainha escolar
  }
  else{
    Serial.println("Falha ao tentar obter a hora");
    digitalWrite(LED_AZUL_STATUS, LOW);
    Serial.println("Tentando sincronizar");
    configTime(gmtOffset*3600, daylightOffset_sec, ntpServer);
  }
    
}
