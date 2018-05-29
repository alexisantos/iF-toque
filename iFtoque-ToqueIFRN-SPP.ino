/*
ACIONADOR TOQUE ESCOLAR ELETRÔNICO AUTOMÁTICO IFRN-SPP
- Baseado no sample TimeNTP da biblioteca Time.h
- Função timeToStudy desenvolvida por Diego Cirilo (https://github.com/dvcirilo/alarme_ifrn)

Autor: Alex Santos - CTI/SPP (Outubro/2017)
Último Update: 06/02/2018
*/

#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

# define VERSAO 2018020614

/* Protótipos das funções: */
/*------------------------- */
   /*   Acionamento to toque escolar  */
boolean DiadeAula ();                                           // Retorna true caso não seja sabado, domingo e feriados
boolean timeToStudy(byte hora, byte minuto, byte segundo);      // Retorna true se estiver na hora de acionar o toque escolar

   /*   Formatação e exibicao de data/hora: */
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

  /*    Funcoes com rede */
void    ServidorWeb  (byte timestatus);                         // Monta um servidor web, exibe as informações de hora caso a hora tenha sido obtida
void    sendNTPpacket(IPAddress &address);                      // Envia solicitações NTP para o servidor NTP do endereço IP fornecido
/*------------------------- */

/* PINOS UTILIZADOS */
#define SINETA 2

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xB0, 0x5D }; 
static byte ip[] = { 192, 168, 1, 110};  
static byte gateway[] = { 192, 168, 1, 1 };  
static byte subnet[] = { 255, 255, 255, 0 };  
static byte dns_server[] = { 192, 168, 1, 1 };


// NTP Servers:
IPAddress timeServer(10, 0, 0, 1);         // IP NTP IFRN (ntp.ifrn.local)
//IPAddress timeServer(200, 160, 0, 8);       // IP NTP.br   (ntp na internet ntp.br)


// Variáveis Globais::
                                    // Para Controle hora hora UTC
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
EthernetServer server(80);

void setup() 
{

  pinMode(SINETA, OUTPUT);
  digitalWrite(SINETA, LOW);
  Serial.begin(9600);
  delay(250);
  Serial.println(F("iF toque IFRN-SPP"));

  Ethernet.begin(mac,ip,dns_server,gateway,subnet);  
  Serial.print(F("IP do dispositivo: "));
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  Serial.println(F("Tentando sincronizar... "));
  setSyncProvider(getNtpTime);
    
}


void loop(){   
  /* timeStatus() retorna: 0 timeNotSet, 1 timeNeedsSync ou 2 timeSet  */
  
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      
      Serial.print(digitalClockDisplayStringData(weekday(),day(),month(),year()));
      Serial.print(F(" | "));
      Serial.println(digitalClockDisplayStringHora(hour(),minute(),second()));
      Serial.print(F("Aula atual: "));     
      Serial.print(AulaAtualNum());
      Serial.print(F(" | "));
      Serial.print(F("Turno atual: "));
      Serial.print(Turno());
      Serial.print(F(" | "));
      Serial.println(TurnoStr());      
      Serial.print(F("Proxima Aula: "));     
      Serial.print(ProximaAula(atualAula)); 
      Serial.print(F(" as  "));     
      Serial.print(HorarioAulaStr(proxAula,1));
      Serial.print(F(" em ")); 
      Serial.print(TempoProxAula());
      Serial.print(F(" faltam "));
      Serial.print(Printzero(tmRestante.Hour));
      Serial.print(F(":"));
      Serial.print(Printzero(tmRestante.Minute));
      Serial.print(F(":"));
      Serial.println(Printzero(tmRestante.Second));
      Serial.println (F("------------------------"));      
      
    }
  }
  if (timeStatus() == timeNotSet) {
     Serial.println(F("Obtencao de hora falhou. Verificar NTP/rede "));
     Serial.println(F("Tentando novamente... "));
     delay(1000);
     Udp.begin(localPort);
     Serial.println(F("Tentando sincronizar..."));
     setSyncProvider(getNtpTime);
  }

  ServidorWeb(timeStatus());

  if (timeToStudy(hour(), minute(), second())){
    digitalWrite(SINETA, HIGH); // relé que controla a sineta / sirene / campainha escolar
    /*Serial.println(F("TRIMMM..."));*/
  }
  else {
    digitalWrite(SINETA, LOW); // relé que controla a sineta / sirene / campainha escolar
  }

  //Calculo SystemUptime
  if ( millis() - prevMillis > 1000 ) {
      // increment previous milliseconds
     prevMillis += 1000;
     //
     // increment up-time counter
     UpTime += 1;
  }
}

boolean DiadeAula (){
  byte mes = month();
  byte dia = day();
  byte diaAula = true;
  
  if((weekday() == 1 || weekday() == 7)) // Sem aulas nos domingos (1) ou sabados (7)
    diaAula = false;

  if(diaAula){                   // Sem aulas nos feriados
      if (mes == 1 && (dia == 1 || dia == 25))   // Feriado confrat. universal e feriado municipal
        diaAula = false;
      if (mes == 4 && dia == 21)                 // Feriado tiradentes
        diaAula = false; 
      if (mes == 5 && dia == 1)                  // Feriado dia do trabalhador
        diaAula = false; 
      if (mes == 7 && dia == 7)                  // Feriado de independencia
        diaAula = false;            
      if (mes == 10 && (dia == 3 || dia == 12))  // Feriados martires, NS Aparecida
        diaAula = false;
      if (mes == 10 && dia == 28)                // Feriado dia servidor publico
        diaAula = false;    
      if (mes == 11 && (dia == 2 || dia == 15))  // Feriados finados e republica
        diaAula = false;    
      if (mes == 12 && (dia == 25 || dia == 30)) // Feriado de Natal e feriado municipal
        diaAula = false;
  }
  return diaAula;
}

boolean timeToStudy(byte hora, byte minuto, byte segundo){
  byte tempoLongo = 7;
  byte tempoCurto = 3;
  boolean diaAula = true;
  
  if(DiadeAula()){
    // toque de tempoLongo s para 07h00m, 12h00m, 13h00m, 18h00m, 19h00m
    if((hora==7 || hora==12 || hora==13 || hora==18 || hora==19) && minuto==0 && (segundo>=0 && segundo<=tempoLongo))
      return true;

    // toque de tempoLongo s para 8h30m, 10h30m, 14h30m, 16h30m, 20h30m
    if((hora==8 || hora==10 || hora==14 || hora==16 || hora==20) && minuto==30 && (segundo>=0 && segundo<=tempoLongo))
      return true;

    // toque de tempoLongo s para 08h50m, 14h50m
    if((hora==8 || hora==14) && minuto==50 && (segundo>=0 && segundo<=tempoLongo))
      return true;

    // toque de tempoLongo s para 10h20m, 16h20m
    if((hora==10 || hora==16) && minuto==20 && (segundo>=0 && segundo<=tempoLongo))
      return true;

    // toque de tempoLongo s para 20h40m, 22h10m
    if(((hora==20 && minuto==40) || (hora==22 && minuto==10)) && (segundo>=0 && segundo<=tempoLongo))
      return true;

    // toque de tempoCurto s para 07h45m, 13h45m, 19h45
    if((hora==7 || hora==13 || hora==19) && minuto==45 && (segundo>=0 && segundo<=tempoCurto))
      return true;

    // toque de tempoCurto s para 09h35, 15h35m
    if((hora==9 || hora==15) && minuto==35 && (segundo>=0 && segundo<=tempoCurto))
      return true;

    // toque de tempoCurto s para 11h15m, 17h15m
    if((hora==11 || hora==17) && minuto==15 && (segundo>=0 && segundo<=tempoCurto))
      return true;

    // toque de tempoCurto s para 21h25m
    if(hora==21 && minuto==25 && (segundo>=0 && segundo<=tempoCurto))
      return true;
  } else{
    return false; 
  }
  return false;
}

time_t HoraEpoch(byte hh, byte mm){
  tmToque.Year = year() - 1970;
  tmToque.Month = month();
  tmToque.Day = day();
  tmToque.Hour = hh;
  tmToque.Minute = mm;
  tmToque.Second = 0;
  return makeTime(tmToque);         //converte a hora da estrutura tmElements_t para o tipo time_t (epoch time)
}

byte AulaAtualNum (){
  // retorna o horario atual da aula.
  // Retorna 0 caso fora de horario de aula e 10 ou 20 para indicar primeiro ou segundo intervalo
  unsigned long hora = now();
  byte aula = 0;
  if (turno == 1){         //Manhã: hora entre (00:00:00 e 12:00:00)
       if (hora >= HoraEpoch(7,0)){ //Turno matutino (7:00 ~ 11:59)
          if ((hora >= HoraEpoch(7,0)) && (hora < HoraEpoch(7,45)))
            aula = 1;
          if ((hora >= HoraEpoch(7,45)) && (hora < HoraEpoch(8,30)))
            aula = 2;
          if ((hora >= HoraEpoch(8,30)) && (hora < HoraEpoch(8,50)))
            aula = 10;
          if ((hora >= HoraEpoch(8,50)) && (hora < HoraEpoch(9,35)))
            aula = 3;
          if ((hora >= HoraEpoch(9,35)) && (hora < HoraEpoch(10,20)))
            aula = 4;
          if ((hora >= HoraEpoch(10,20)) && (hora < HoraEpoch(10,30)))
            aula = 20;  
          if ((hora >= HoraEpoch(10,30)) && (hora < HoraEpoch(11,15)))
            aula = 5;
          if ((hora >= HoraEpoch(11,15)) && (hora < HoraEpoch(12,0)))
            aula = 6;  
      }
      else{
        aula = 0; //fora de horário de aula (0~6:59)
      }
  }
  if (turno == 2){  //hora entre (12:00:00 e 23:59:59)
      if ((hora >= HoraEpoch(13,0)) && (hora < HoraEpoch(18,0))){                       
          if ((hora >= HoraEpoch(13,0)) && (hora < HoraEpoch(13,45)))
            aula = 1; 
          if ((hora >= HoraEpoch(13,45)) && (hora < HoraEpoch(14,30)))
            aula = 2;
          if ((hora >= HoraEpoch(14,30)) && (hora < HoraEpoch(14,50)))
            aula = 10;
          if ((hora >= HoraEpoch(14,50)) && (hora < HoraEpoch(15,35)))
            aula = 3;
          if ((hora >= HoraEpoch(15,35)) && (hora < HoraEpoch(16,20)))
            aula = 4;
          if ((hora >= HoraEpoch(16,20)) && (hora < HoraEpoch(16,30)))
            aula = 20;  
          if ((hora >= HoraEpoch(16,30)) && (hora < HoraEpoch(17,15)))
            aula = 5;    
          if ((hora >= HoraEpoch(17,15)) && (hora < HoraEpoch(18,0)))
            aula = 6;  
      }
      else {
          aula = 0; //fora de horário de aula (12~12:59)
      } 
  } 
  if (turno == 3){
      if ((hora >= HoraEpoch(19,0)) && (hora < HoraEpoch(22,0))){         
          if ((hora >= HoraEpoch(19,0)) && (hora < HoraEpoch(19,45)))
            aula = 1; 
          if ((hora >= HoraEpoch(19,45)) && (hora < HoraEpoch(20,30)))
            aula = 2;
          if ((hora >= HoraEpoch(20,30)) && (hora < HoraEpoch(20,40)))
            aula = 30;
          if ((hora >= HoraEpoch(20,40)) && (hora < HoraEpoch(21,25)))
            aula = 3;
          if ((hora >= HoraEpoch(21,25)) && (hora < HoraEpoch(22,10)))
            aula = 4;
            
      }
      else {
          aula = 0; //fora de horário de aula (18~18:59)
      } 
  }
  atualAula = aula;
  return atualAula;   
}

String AulaStr (byte aula, byte opcao){
  String s; byte i;
  if ((aula==0) || (aula > 6)){
      s = "Intervalo";
      if (aula == 0)
        s = "Troca de turno";
  }else{
      if ((turno == 3) && (opcao==1) && (now() > HoraEpoch(21,25))){  
          s = "Amanhã";
      }else{
          for (i=1; i <= aula; i++){
              s = (String)i + "º horário";
          }  
      }  
   }
  return s;
}

byte ProximaAula (byte aula_atual){
  unsigned long hora = now();
  byte proxaula = 0;
  if (turno != 3){
      if (aula_atual >= 6){     //verifica horario de aula atual (0-6) ou (10, 20,30) caso intervalo
          if (aula_atual == 6)
            proxaula = 1;       //se a aula atual for a 6ª, a proxima aula será a 1ª do proximo turno
          if (aula_atual == 10) //intervalo entre 2º e 3º horario
            proxaula = 3;
          if (aula_atual == 20) //intervalo entre 4º e 5º horario
            proxaula = 5;     
      }else {
          proxaula = aula_atual + 1; 
          if (aula_atual == 0)
            proxaula = 1;
      }  
  } else {
      if (aula_atual >= 4){
        if (aula_atual ==  4) proxaula = 1;        
        if (aula_atual == 30) proxaula = 3;
      }else {
          proxaula = aula_atual + 1;
          if (aula_atual == 0)
            proxaula = 1;
      }
  }
         
  proxAula = proxaula;     
    
  return proxAula;      
}

String HorarioAulaStr (byte aula, byte opcao){
  //opcao: 0 para horario de aula atual; 1 para horario de proxima aula
  String manha[] = {"-", "07:00 até 07:45", "07:45 até 08:30", "08:50 até 09:35", "09:35 até 10:20", "10:30 até 11:15", "11:15 até 12:00"};
  String tarde[] = {"-", "13:00 até 13:45", "13:45 até 14:30", "14:50 até 15:35", "15:35 até 16:20", "16:30 até 17:15", "17:15 até 18:00"};
  String noite[] = {"-", "19:00 até 19:45", "19:45 até 20:30", "20:40 até 21:25", "21:25 até 22:10"};

  String s = ""; 

  if (turno == 1){ 
    if ((aula > 0) && (aula <= 6)){
      if (opcao == 1){
          if ((now() < HoraEpoch(11,15)))
            s = manha[aula];  
          else
            s = tarde[1];    // se a consulta eh para o próximo horario (amanha)
      }
      if (opcao == 0)
          s = manha[aula];
    }    
    else{
        if (aula == 0){
          if (opcao == 0)
            s = "";
        }else{
          if (aula == 10){
            s = "08:30 até 08:50";
          }else
            if (aula == 20)
               s =  "10:20 até 10:30";
        }
    }
  }  
  if (turno == 2){
    if ((aula > 0) && (aula <= 6)){
        if (opcao == 1){
            if ((now() < HoraEpoch(17,15)))
              s = tarde[aula];
            else
              s = noite[1];    // se a consulta eh para o próximo horario (amanha)
        }
        if (opcao == 0) 
            s = tarde[aula];
    }else{
      if (aula == 0){
          if (opcao == 0)
            s = "";
        }else{
          if (aula == 10){
            s =  "14:30 até 14:50";
          }else
            if (aula == 20)
               s =  "16:20 até 16:30";
        }
    }
  }
  if (turno == 3) {              
    if ((aula > 0) && (aula <= 4)){
        if (opcao == 1){
            if ((now() < HoraEpoch(21,25))){
              s = noite[aula];  
            }
            else{
              s = manha[1];    // se a consulta eh para o próximo horario (amanha)
            }
        }
        if (opcao == 0){
            s = noite[aula];
        }
    } else{
      if (aula == 0) {
          if (opcao == 0)
            s = "";
          else{
            s = noite[1];
            if (now() >= HoraEpoch(22,15))
              manha[1];
          }  
        }else{
          if (aula == 30)
            s =  "20:30 até 20:40";
        }
    }
 }
      
  return s;
 
}


byte Turno(){
  if (isPM()){
      if (hour()>=18)
        turno = 3;
      else   
        turno = 2;
  }
  else{
      turno = 1;
  }
  return turno;
}

String TurnoStr(){
  if (turno == 1)
    return "Manhã";
  else if (turno == 2)
    return "Tarde";
      else
        return "Noite";
}
String TurnoStrRange(){
  if (turno == 1)
    return "(07h as 12h)";
  else if (turno == 2)
    return "(13h as 18h)";
      else
        return "(19h as 22h10min)";
}

time_t TempoProxAula(){
  time_t timeStampDif;
  boolean amanha = false;
  
  if (turno == 1){
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((now() < HoraEpoch(7,0) ))){
          tmToque.Hour = 7; tmToque.Minute = 0;
        } else{
          tmToque.Hour = 13; tmToque.Minute = 0;
        }
        break;
      case 2:
        tmToque.Hour = 7; tmToque.Minute = 45; break;
      case 3:
        tmToque.Hour = 8; tmToque.Minute = 50; break;
      case 4:
        tmToque.Hour = 9; tmToque.Minute = 35; break;
      case 5:
        tmToque.Hour = 10; tmToque.Minute = 30; break;
      case 6:
        tmToque.Hour = 11; tmToque.Minute = 15; break;        
    }
  }else
  if (turno == 2){
      switch (proxAula) {
        case 1:
          if ( (atualAula == 0) && ((now() < HoraEpoch(13,0) ))){
            tmToque.Hour = 13; tmToque.Minute = 0;
          } else{
            tmToque.Hour = 19; tmToque.Minute = 0;
          }
          break;
        case 2:
          tmToque.Hour = 13; tmToque.Minute = 45; break;
        case 3:
          tmToque.Hour = 14; tmToque.Minute = 50; break;
        case 4:
          tmToque.Hour = 15; tmToque.Minute = 35; break;
        case 5:
          tmToque.Hour = 16; tmToque.Minute = 30; break;
        case 6:
          tmToque.Hour = 17; tmToque.Minute = 15; break;        
      }
  } else
  if (turno == 3){
       switch (proxAula) {
        case 1:
          if ( (atualAula == 0) && ((now() < HoraEpoch(19,0) ))){
            tmToque.Hour = 19; tmToque.Minute = 0;
          } else{
            tmToque.Hour = 7;  tmToque.Minute = 0;
            amanha = true;
          }
          break;
        case 2:
          tmToque.Hour = 19; tmToque.Minute = 15; break;
        case 3:
          tmToque.Hour = 20; tmToque.Minute = 40; break;
        case 4:
          tmToque.Hour = 21; tmToque.Minute = 25; break;       
      }
  }
  if (amanha){
    timeStampDif = ( ( HoraEpoch(tmToque.Hour,tmToque.Minute) - now()) + nextMidnight(now() ) );
  }
  else{
    timeStampDif = ( ( HoraEpoch(tmToque.Hour,tmToque.Minute) - now()) + previousMidnight(now() ) );
  }
     
  breakTime(timeStampDif, tmRestante); // Desmembra a hora do tipo t_time para uma estrutura tmElements_t (tmRestante) contendo: hh/mm/ss... 
  return (timeStampDif);               //  retorna o timestamp (epoch time) para o proximo toque

}
      

String digitalClockDisplayStringHora(byte hh, byte mm, byte ss){
  return Printzero(hh) + ":" + Printzero(mm) + ":" + Printzero(ss);
}

String digitalClockDisplayStringData(byte wday, byte DD, byte MM, int YYYY){
  return diaSemana(wday) + ", " + Printzero(DD) + "/" + Printzero(MM) + "/" + (YYYY);
}

String diaSemana(byte dia){
  String str[] = {"-","Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
  return str[dia];
}

String Printzero(byte a){
  if(a>=10){
    return (String)a+"";
  }
    else {
      return "0"+(String)a;
    }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(F("Transmit NTP Request"));
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println(F("Receive NTP Response"));
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println(F("No NTP Response :-("));
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();

}


void ServidorWeb (byte timestatus){  
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("new client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
          client.println(F("Refresh: 15"));        // refresh the page automatically every 15 sec
          client.println(F(""));
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<html><head><meta charset='UTF-8'><title>iF toque :: IFRN-SPP</title></head><body bgcolor='#243224' text='white'><center>"));
          client.print(F("<font face='Tahoma' size=10>"));
          client.print(F("<p><b>iF toque • IFRN-SPP</b></p>"));      
          client.print(F("</font>"));
          client.println(F("<font face='Arial' size='5'>")); 
          if (timestatus!= timeNotSet){                 
              client.println(F("<table style='width: 500px; margin-left: auto; margin-right: auto; border:dashed; border-color:#426F42' cellspacing='3'>")); 
              client.println(F("<tbody>"));
              client.println(F("<tr>"));              
              client.println(F("<td height='30' colspan='2' align='center' bgcolor='#426F42'>Horário atual</td>"));              
              client.println(F("</tr>"));
              client.println(F("<tr>"));  
              client.println(F("<td height='55'>&nbsp;"));
              client.print(AulaStr(atualAula,0));  
              client.println(F("</td>"));                   
              client.println(F("<td><div align='right'>&nbsp;"));
              client.print(HorarioAulaStr(atualAula,0)); 
              client.print(F("&nbsp;&nbsp;</div></td>"));  
              client.println(F("</tr>"));
              client.println(F("<tr>"));  
              client.println(F("<td height='30' colspan='2' align='center' bgcolor='#426F42'>Próxima Aula</td>"));              
              client.println(F("</tr>"));
              client.println(F("<tr>"));  
              client.println(F("<td height='36'>&nbsp;"));
              client.print(AulaStr(proxAula,1));
              client.println(F("&nbsp;&nbsp;</td>"));
              client.println(F("<td><div align='right'>com início as&nbsp;&nbsp;<br/>"));
              client.print(HorarioAulaStr(proxAula,1));
              client.print(F("&nbsp;&nbsp;</div></td>"));
              client.println(F("</tr>"));
              client.println(F("<tr>")); 
              client.print(F("<td colspan='2'><p style='text-align: center;'>Próxima aula inicia em&nbsp;"));
              client.print(F("<span style='background-color: #426F42;color: #fff; display: inline-block; padding: 2px 8px; font-weight: bold; border-radius: 5px;'>"));            
              client.print(Printzero(tmRestante.Hour)); client.print(F(":"));
              client.print(Printzero(tmRestante.Minute)); client.print(F(":"));
              client.print(Printzero(tmRestante.Second));
              client.print(F("</span></p></td>"));
              client.println(F("</tr>"));
              client.println(F("</tbody>"));
              client.println(F("</table>"));  
          }
          else{
              client.println(F("Verificar NTP server. <br/>Falha em obter hora"));
          }
          client.println(F("</font>"));
          client.println(F("<p>&nbsp;</p>"));
          client.print(F("<font face='Lucida Sans Unicode' size='-1'>"));
          client.print(F("Agora: "));
          client.print(digitalClockDisplayStringData(weekday(),day(),month(),year()));
          client.println(F(" | "));
          client.print(digitalClockDisplayStringHora(hour(),minute(),second()));          
          client.print(F("<br/>Time info: ")); 
          client.print(now()); client.print(F(" | ")); client.print(UpTime);          
          client.println(F("<br/></font><font face='Lucida Sans Unicode' size='-2'>"));
          client.println(F("<p><img src='https://ead.ifrn.edu.br/cdn/images/ifrn-logo.png'><br/>© IFRN-SPP | Nov/2017"));
          client.print(F("<br/>v. "));
          client.print(VERSAO);
          client.print(F("</font></p></center></body></html>"));
          break;
        }
        
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  } 

 
}



