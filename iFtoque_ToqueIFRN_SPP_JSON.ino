/*
  ACIONADOR TOQUE ESCOLAR ELETRÔNICO AUTOMÁTICO IFRN-SPP
  - Baseado no sample TimeNTP da biblioteca Time.h
  - Função timeToStudy desenvolvida por Diego Cirilo (https://github.com/dvcirilo/alarme_ifrn)

  Autor: Alex Santos - CTI/SPP (Outubro/2017)
  Último Update: Fev/2021
*/

#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

/* ---------Protótipos das funções: -------------------*/
/*---------------------------------------------------- */
/*   Acionamento do toque escolar - Funcões essenciais*/
//void    (* resetFunc) (void) = 0;                             // Resseta o arduino
boolean DiadeAula();                                            // Retorna true caso não seja sabado, domingo e feriados (Configurar feriados nesta função)
boolean timeToStudy(byte hora, byte minuto, byte segundo);      // Retorna true se estiver na hora de acionar o toque escolar

/*   Formatação e exibicao de data/hora: */
void    Imprime_Info_DataHora_Serial();
String  digitalClockDisplayStringHora(byte hh, byte mm, byte ss);                 // Retorna a hora em formato hh:mm:ss
String  digitalClockDisplayStringData(byte wday, byte DD, byte MM, int YYYY);     // Retorna a hora em formato hh:mm:ss
String  diaSemana(byte dia);                                    // Recebe um numero (1-7) e retorna dia da semana em string (-,Domingo, Segunda, (...), Sabado)
String  Printzero(byte a);                                      // Recebe um número

/*    Exibicao de informações da aula atual */
byte    AulaAtualNum ();                                        // Retorna um numero correspondente a aula atual: (1 a 6) ou (0, 10, 20 ou 30)
String  AulaStr (byte aula, byte opcao);                        // Retorna uma String com a aula atual formatada: '(1º-6º) horário', 'Intervalo', 'Troca de turno'
byte    ProximaAula (byte aula_atual);                          // Retorna um numero correspondente a próxima Aula: 1 a 6 (nao considera intervalo como aula)
String  HorarioAulaStr (byte aula, byte opcao);                 // Retorna uma String com o horário da hora atual ou da próxima Aula: '11:15 até 12:00'..
byte    Turno ();                                               // Retorna um número correspondente ao  turno atual (Manhã, Tarde ou Noite): 1, 2 ou 3
String  TurnoStr ();                                            // Retorna uma String correspondente ao turno atual (1, 2 ou 3)            : 'Manhã', 'Tarde" ou 'Noite'
String  TurnoStrRange();                                        // Retorna a duracao do turno atual: '07h as 12h', '13 as 18' ou '19h as 22h10min'

/*    Funcoes que retornam tempo no formato Unix epoch/Unix timestamp  */
time_t  getNtpTime ();                                          // Realiza a consulta de hora, retorna 0 caso não consiga obter a hora
time_t  TempoProxAula ();                                       // Calcula o tempo para a proxima aula, retorna um epoch time deste tempo. Quebra esta hora em tmRestante.hora, min e s
time_t  HoraEpoch (byte hh, byte mm);                           // Converte uma hora e minuto fornecida em um epoch time do dia atual. Útil para comparar com now()

/*    Funcoes com rede e monitoramento*/
void    ServidorWeb  ();                                        // Monta um servidor web, exibe as informações de hora caso a hora tenha sido obtida
void    sendNTPpacket(IPAddress &address);                      // Envia solicitações NTP para o servidor NTP do endereço IP fornecido
String  NTP_Status_Desc (byte timestatus);                       // Devolve status da hora em modo textual (retorna: 0 timeNotSet, 1 timeNeedsSync ou 2 timeSet)
void    Uptime_Calc();
/*------------------------- */

/* PINOS UTILIZADOS NO ARDUINO E PORTA DE MONITORAMENTO*/
#define SINETA 3
#define HTTP_PORT 80

/* VARIAVEIS GLOBAIS*/
static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xB0, 0x6D };
static byte ip[] = { 192, 168, 51, 110};
static byte gateway[] = { 192, 168, 51, 1 };
static byte subnet[] = { 255, 255, 255, 0 };
static byte dns_server[] = { 192, 168, 51, 1 };

static char dispositivo_nome[10]   = "if-toque";
static char dispositivo_local[10]  = "IFRN-SPP";
static char dispositivo_versao[15] = "v2 b2021021011";
time_t ultima_sincronizacao  = 0;

// NTP Servers:
IPAddress timeServer(10, 22, 0, 213);         // IP NTP LOCAL
//IPAddress timeServer(200, 160, 0, 8);       // IP NTP PUBLICO   (ntp na internet ntp.br)


// Variáveis Globais::
// Para Controle de hora UTC
const int timeZone = -3;                // Timezone do Brasil (nordeste)
unsigned int localPort = 8888;          // Porta de comunicação UDP local do cliente NTP
// Para exibicao em web: Controle de tempo para proximo toque
tmElements_t tmToque;                    // Estrutura que armazena (hh, mm, ss) para o proximo toque
tmElements_t tmRestante;                 // Estrutura que armazena (hh, mm, ss) restante para o proximo toque

// Para exibicao da hora atual em hh:mm:ss
time_t prevDisplay = 0;                 // Controla para só exibir da data/hora no display/serial caso a hora atual seja diferente da atual

// Para controle de aula e turno
byte atualAula = 0;                     // Armazena a Aula atual  : 1 a 6 ou 0, 10, 20 ou 30
byte proxAula = 0;                      // Armazena a Proxima aula: 1 a 6 (nao considera intervalo como aula)
byte turno = 0;                         // Armazena o turno atual

// Para Calculo de uptime do dispositivo
static uint32_t UpTime = 0;             // Tempo em segundos que o dispotivo está ligado
uint32_t prevMillis = millis();         // Millis() retorna o tempo que o arduino está ligado em ms. PrevMillis é incrementado a cada 1000ms (1s)


EthernetUDP Udp;
EthernetServer server(HTTP_PORT);

void setup(){
  pinMode(SINETA, OUTPUT);
  digitalWrite(SINETA, LOW);
  Serial.begin(9600);
  delay(500);
  Ethernet.begin(mac, ip, dns_server, gateway, subnet);
  server.begin(); 
  Serial.print(F("IP do dispositivo: "));
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  Serial.print(F("Tentando sincronizar hora com NTP Server... "));
  setSyncProvider(getNtpTime);
  Serial.println(Ethernet.localIP());

  if (timeStatus() == timeSet) {
    ultima_sincronizacao = now();
  }

}


void loop() {
  if (timeToStudy(hour(), minute(), second())) {
    digitalWrite(SINETA, HIGH); // relé que controla a sineta / sirene / campainha escolar
  }
  else {
    digitalWrite(SINETA, LOW); // relé que controla a sineta / sirene / campainha escolar
  }
  
  if (timeStatus() == timeSet) { // timeStatus() retorna: 0 timeNotSet, 1 timeNeedsSync ou 2 timeSet
    Imprime_Info_DataHora_Serial();
  }
  else {
    Serial.println(F("Obtencao de hora falhou. Verificar NTP/rede "));
    Serial.println(F("Tentando sincronizar hora novamente... "));
    delay(1000);
    Udp.begin(localPort);
    setSyncProvider(getNtpTime);
    if (timeStatus() == timeSet) {
      ultima_sincronizacao = now();
    }
  }  
  ServidorWeb();
  Uptime_Calc();
}
