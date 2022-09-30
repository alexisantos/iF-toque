void PreencheJson(){
  char datacompleta[100];

  strftime(datacompleta,80, "%A, %B %d %Y %H:%M:%S", &timeinfo);
  doc["projeto"]["nome"]       = "IF-Toque";
  doc["projeto"]["versao"]     = "v2.5.1";
  doc["projeto"]["inicio"]     = "Nov/2017";
  doc["projeto"]["compilacao"] = "30/09/2022";
  doc["sistema"]["ip"]   = WiFi.localIP();
  doc["sistema"]["mac"]  = WiFi.macAddress();
  doc["sistema"]["rssi"] = WiFi.RSSI();
  doc["sistema"]["uptime"] = locUpTime;
  doc["iftoque"]["datahora"] = datacompleta;  
  doc["iftoque"]["unixtime"] = getTime();
  doc["ntp"]["ntp_server"] = ntpServer;
  doc["ntp"]["sincronizacao"] = last_ntp_update;
  doc["horarios_aula"]["dia_aula"] = DiadeAula ();
  doc["horarios_aula"]["aula_atual"]["horario"]["num"] = AulaAtualNum();
  doc["horarios_aula"]["aula_atual"]["horario"]["horario"] = HorarioAulaStr(atualAula, 0);
  doc["horarios_aula"]["aula_atual"]["turno"]["num"] = Turno();
  doc["horarios_aula"]["aula_atual"]["turno"]["turno"] = TurnoStr();
  doc["horarios_aula"]["prox_aula"]["horario"]["num"] = ProximaAula(atualAula);
  doc["horarios_aula"]["prox_aula"]["horario"]["horario"] = HorarioAulaStr(proxAula, 0);
  doc["horarios_aula"]["prox_aula"]["tempo_restante"]["tempo"] = String(tmRestante.Hour) + "h " + String(tmRestante.Minute) + "m " + String(tmRestante.Second) + "s";
  doc["horarios_aula"]["prox_aula"]["tempo_restante"]["unixtime"] = TempoProxAula();
  doc["data_hora"]["dia_semana_data"] = diaSemana(timeinfo.tm_wday+1) + ", " + Printzero(timeinfo.tm_mday) + "/" + Printzero(timeinfo.tm_mon+1) + "/" + String(1900 + timeinfo.tm_year);
  doc["data_hora"]["dia_semana_str"] = diaSemana(timeinfo.tm_wday+1);
  doc["data_hora"]["dia_semana_int"] = timeinfo.tm_wday+1;
  doc["data_hora"]["data"] = Printzero(timeinfo.tm_mday) + "/" + Printzero(timeinfo.tm_mon+1) + "/" + String(1900 + timeinfo.tm_year);
  doc["data_hora"]["hora"] = Printzero(timeinfo.tm_hour) + ":" + Printzero(timeinfo.tm_min) + ":" + Printzero(timeinfo.tm_sec);
}

unsigned long getTime() { // Function that gets current epoch time
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&agora);
  return agora;
}

void timeSyncCallback(struct timeval *tv)
{
  Serial.println("\n----Time Sync-----");
  Serial.println(tv->tv_sec);
  Serial.println(ctime(&tv->tv_sec));
  last_ntp_update  = tv->tv_sec;
}

void Uptime(){
  if ( millis() - prevMillis > 1000 ) {
     prevMillis += 1000;
     locUpTime += 100;
     Serial.println("");
     Serial.print(WiFi.localIP());
     Serial.print(" | ");
     Serial.println(WiFi.macAddress());

  }
}

boolean timeToStudy(byte hora, byte minuto, byte segundo) {
  byte tempoLongo = 7;
  byte tempoCurto = 3;
  boolean diaAula = true;

  if (DiadeAula()) {
    // toque de tempoLongo s para 07h00m, 12h00m, 13h00m, 18h00m, 19h00m
    if ((hora == 7 || hora == 12 || hora == 13 || hora == 18 || hora == 19) && minuto == 0 && (segundo >= 0 && segundo <= tempoLongo))
      return true;

    // toque de tempoLongo s para 8h30m, 10h30m, 14h30m, 16h30m, 20h30m
    if ((hora == 8 || hora == 10 || hora == 14 || hora == 16 || hora == 20) && minuto == 30 && (segundo >= 0 && segundo <= tempoLongo))
      return true;

    // toque de tempoLongo s para 08h50m, 14h50m
    if ((hora == 8 || hora == 14) && minuto == 50 && (segundo >= 0 && segundo <= tempoLongo))
      return true;

    // toque de tempoLongo s para 10h20m, 16h20m
    if ((hora == 10 || hora == 16) && minuto == 20 && (segundo >= 0 && segundo <= tempoLongo))
      return true;

    // toque de tempoLongo s para 20h40m, 22h10m
    if (((hora == 20 && minuto == 40) || (hora == 22 && minuto == 10)) && (segundo >= 0 && segundo <= tempoLongo))
      return true;

    // toque de tempoCurto s para 07h45m, 13h45m, 19h45
    if ((hora == 7 || hora == 13 || hora == 19) && minuto == 45 && (segundo >= 0 && segundo <= tempoCurto))
      return true;

    // toque de tempoCurto s para 09h35, 15h35m
    if ((hora == 9 || hora == 15) && minuto == 35 && (segundo >= 0 && segundo <= tempoCurto))
      return true;

    // toque de tempoCurto s para 11h15m, 17h15m
    if ((hora == 11 || hora == 17) && minuto == 15 && (segundo >= 0 && segundo <= tempoCurto))
      return true;

    // toque de tempoCurto s para 21h25m
    if (hora == 21 && minuto == 25 && (segundo >= 0 && segundo <= tempoCurto))
      return true;
  } else {
    return false;
  }
  return false;
}

boolean DiadeAula () {
  byte mes = timeinfo.tm_mon + 1;
  byte dia = timeinfo.tm_mday;
  byte diasemana = timeinfo.tm_wday + 1;
  byte diaAula = true;

  if ((diasemana == 1 || diasemana == 7)) // Sem aulas nos domingos (1) ou sabados (7)
    diaAula = false;

  if (diaAula) {                 // Sem aulas nos feriados
    if (mes == 1 && (dia == 1 || dia == 25) || dia == 16)   // Feriado confrat. universal e feriados municipais em Sâo Paulo do Potengi
      diaAula = false;
    if (mes == 4 && dia == 21)                 // Feriado tiradentes
      diaAula = false;
    if (mes == 5 && dia == 1)                  // Feriado dia do trabalhador
      diaAula = false;
    if (mes == 7 && dia == 7)                  // Feriado de independencia
      diaAula = false;
    if (mes == 10 && (dia == 3 || dia == 12))  // Feriados martires RN, NS Aparecida
      diaAula = false;
    if (mes == 11 && (dia == 2 || dia == 15))  // Feriados finados e republica
      diaAula = false;
    if (mes == 12 && (dia == 25 || dia == 30)) // Feriado de Natal e feriado municipal de São Paulo do Potengi
      diaAula = false;
  }
  return diaAula;
}

time_t HoraEpoch(byte hh, byte mm) {
  //https://cplusplus.com/reference/ctime/mktime/
  time_t rawtime;
  struct tm *infodatahora;
  
  time(&rawtime);
  infodatahora = localtime(&rawtime);
  infodatahora -> tm_hour = hh;
  infodatahora -> tm_min = mm;
  infodatahora -> tm_sec = 0;
  return mktime(infodatahora);         //converte a hora da estrutura tm para o tipo time_t (epoch time)
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  delay(1000);
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid, password);
}



String diaSemana(byte dia) {
  String str[] = {"-", "Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
  return str[dia];
}


String Printzero(byte a) {
  if (a >= 10) {
    return (String)a + "";
  }
  else {
    return "0" + (String)a;
  }
}
void Imprime_Info_DataHora_Serial(){
  if (getTime() != prevDisplay) { //update the display only if time has changed
      prevDisplay = getTime();
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); 
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
      Serial.print(HorarioAulaStr(proxAula, 1));
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

byte AulaAtualNum () {
  // retorna o horario atual da aula.
  // Retorna 0 caso fora de horario de aula e 10 ou 20 para indicar primeiro ou segundo intervalo
  unsigned long hora = getTime();

  byte aula = 0;
  if (turno == 1) {        //Manhã: hora entre (00:00:00 e 12:00:00)
    if (hora >= HoraEpoch(7, 0)) { //Turno matutino (7:00 ~ 11:59)
      if ((hora >= HoraEpoch(7, 0)) && (hora < HoraEpoch(7, 45)))
        aula = 1;
      if ((hora >= HoraEpoch(7, 45)) && (hora < HoraEpoch(8, 30)))
        aula = 2;
      if ((hora >= HoraEpoch(8, 30)) && (hora < HoraEpoch(8, 50)))
        aula = 10;
      if ((hora >= HoraEpoch(8, 50)) && (hora < HoraEpoch(9, 35)))
        aula = 3;
      if ((hora >= HoraEpoch(9, 35)) && (hora < HoraEpoch(10, 20)))
        aula = 4;
      if ((hora >= HoraEpoch(10, 20)) && (hora < HoraEpoch(10, 30)))
        aula = 20;
      if ((hora >= HoraEpoch(10, 30)) && (hora < HoraEpoch(11, 15)))
        aula = 5;
      if ((hora >= HoraEpoch(11, 15)) && (hora < HoraEpoch(12, 0)))
        aula = 6;
    }
    else {
      aula = 0; //fora de horário de aula (0~6:59)
    }
  }
  if (turno == 2) { //hora entre (12:00:00 e 23:59:59)
    if ((hora >= HoraEpoch(13, 0)) && (hora < HoraEpoch(18, 0))) {
      if ((hora >= HoraEpoch(13, 0)) && (hora < HoraEpoch(13, 45)))
        aula = 1;
      if ((hora >= HoraEpoch(13, 45)) && (hora < HoraEpoch(14, 30)))
        aula = 2;
      if ((hora >= HoraEpoch(14, 30)) && (hora < HoraEpoch(14, 50)))
        aula = 10;
      if ((hora >= HoraEpoch(14, 50)) && (hora < HoraEpoch(15, 35)))
        aula = 3;
      if ((hora >= HoraEpoch(15, 35)) && (hora < HoraEpoch(16, 20)))
        aula = 4;
      if ((hora >= HoraEpoch(16, 20)) && (hora < HoraEpoch(16, 30)))
        aula = 20;
      if ((hora >= HoraEpoch(16, 30)) && (hora < HoraEpoch(17, 15)))
        aula = 5;
      if ((hora >= HoraEpoch(17, 15)) && (hora < HoraEpoch(18, 0)))
        aula = 6;
    }
    else {
      aula = 0; //fora de horário de aula (12~12:59)
    }
  }
  if (turno == 3) {
    if ((hora >= HoraEpoch(19, 0)) && (hora < HoraEpoch(22, 0))) {
      if ((hora >= HoraEpoch(19, 0)) && (hora < HoraEpoch(19, 45)))
        aula = 1;
      if ((hora >= HoraEpoch(19, 45)) && (hora < HoraEpoch(20, 30)))
        aula = 2;
      if ((hora >= HoraEpoch(20, 30)) && (hora < HoraEpoch(20, 40)))
        aula = 30;
      if ((hora >= HoraEpoch(20, 40)) && (hora < HoraEpoch(21, 25)))
        aula = 3;
      if ((hora >= HoraEpoch(21, 25)) && (hora < HoraEpoch(22, 10)))
        aula = 4;

    }
    else {
      aula = 0; //fora de horário de aula (18~18:59)
    }
  }
  atualAula = aula;
  return atualAula;
}

String AulaStr (byte aula, byte opcao) {
  String s; byte i;
  if ((aula == 0) || (aula > 6)) {
    s = "Intervalo";
    if (aula == 0)
      s = "Troca de turno";
  } else {
    if ((turno == 3) && (opcao == 1) && (getTime() > HoraEpoch(21, 25))) {
      s = "Amanhã";
    } else {
      for (i = 1; i <= aula; i++) {
        s = (String)i + "º horário";
      }
    }
  }
  return s;
}

byte ProximaAula (byte aula_atual) {
  unsigned long hora = getTime();
  byte proxaula = 0;
  if (turno != 3) {
    if (aula_atual >= 6) {    //verifica horario de aula atual (0-6) ou (10, 20,30) caso intervalo
      if (aula_atual == 6)
        proxaula = 1;       //se a aula atual for a 6ª, a proxima aula será a 1ª do proximo turno
      if (aula_atual == 10) //intervalo entre 2º e 3º horario
        proxaula = 3;
      if (aula_atual == 20) //intervalo entre 4º e 5º horario
        proxaula = 5;
    } else {
      proxaula = aula_atual + 1;
      if (aula_atual == 0)
        proxaula = 1;
    }
  } else {
    if (aula_atual >= 4) {
      if (aula_atual ==  4) proxaula = 1;
      if (aula_atual == 30) proxaula = 3;
    } else {
      proxaula = aula_atual + 1;
      if (aula_atual == 0)
        proxaula = 1;
    }
  }

  proxAula = proxaula;
  return proxAula;
}

String HorarioAulaStr (byte aula, byte opcao) {
  //opcao: 0 para horario de aula atual; 1 para horario de proxima aula
  String manha[] = {"-", "07:00 até 07:45", "07:45 até 08:30", "08:50 até 09:35", "09:35 até 10:20", "10:30 até 11:15", "11:15 até 12:00"};
  String tarde[] = {"-", "13:00 até 13:45", "13:45 até 14:30", "14:50 até 15:35", "15:35 até 16:20", "16:30 até 17:15", "17:15 até 18:00"};
  String noite[] = {"-", "19:00 até 19:45", "19:45 até 20:30", "20:40 até 21:25", "21:25 até 22:10"};

  String s = "";

  if (turno == 1) {
    if ((aula > 0) && (aula <= 6)) {
      if (opcao == 1) {
        if ((getTime() < HoraEpoch(11, 15)))
          s = manha[aula];
        else
          s = tarde[1];    // se a consulta eh para o próximo horario (amanha)
      }
      if (opcao == 0)
        s = manha[aula];
    }
    else {
      if (aula == 0) {
        if (opcao == 0)
          s = "";
      } else {
        if (aula == 10) {
          s = "08:30 até 08:50";
        } else if (aula == 20)
          s =  "10:20 até 10:30";
      }
    }
  }
  if (turno == 2) {
    if ((aula > 0) && (aula <= 6)) {
      if (opcao == 1) {
        if ((getTime() < HoraEpoch(17, 15)))
          s = tarde[aula];
        else
          s = noite[1];    // se a consulta eh para o próximo horario (amanha)
      }
      if (opcao == 0)
        s = tarde[aula];
    } else {
      if (aula == 0) {
        if (opcao == 0)
          s = "";
      } else {
        if (aula == 10) {
          s =  "14:30 até 14:50";
        } else if (aula == 20)
          s =  "16:20 até 16:30";
      }
    }
  }
  if (turno == 3) {
    if ((aula > 0) && (aula <= 4)) {
      if (opcao == 1) {
        if ((getTime() < HoraEpoch(21, 25))) {
          s = noite[aula];
        }
        else {
          s = manha[1];    // se a consulta eh para o próximo horario (amanha)
        }
      }
      if (opcao == 0) {
        s = noite[aula];
      }
    } else {
      if (aula == 0) {
        if (opcao == 0)
          s = "";
        else {
          s = noite[1];
          if (getTime() >= HoraEpoch(22, 15))
            manha[1];
        }
      } else {
        if (aula == 30)
          s =  "20:30 até 20:40";
      }
    }
  }
  return s;
}

byte Turno() {
  if (timeinfo.tm_hour >= 12) {
    if (timeinfo.tm_hour >= 18)
      turno = 3;
    else
      turno = 2;
  }
  else {
    turno = 1;
  }
  return turno;
}

String TurnoStr() {
  if (turno == 1)
    return "Manhã";
  else if (turno == 2)
    return "Tarde";
  else
    return "Noite";
}
String TurnoStrRange() {
  if (turno == 1)
    return "(07h as 12h)";
  else if (turno == 2)
    return "(13h as 18h)";
  else
    return "(19h as 22h10min)";
}

time_t TempoProxAula() {
  time_t timeStampDif;
  boolean amanha = false;
  int hora;
  int minuto;

  if (turno == 1) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && (getTime() < HoraEpoch(7, 0) )) {
          hora = 7; minuto = 0;
        } else {
          hora = 13; minuto = 0;
        }
        break;
      case 2:
        hora = 7; minuto = 45; break;
      case 3:
        hora = 8; minuto = 50; break;
      case 4:
        hora = 9; minuto = 35; break;
      case 5:
        hora = 10; minuto = 30; break;
      case 6:
        hora = 11; minuto = 15; break;
    }
  } else if (turno == 2) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((getTime() < HoraEpoch(13, 0) ))) {
          hora = 13; minuto = 0;
        } else {
          hora = 19; minuto = 0;
        }
        break;
      case 2:
        hora = 13; minuto = 45; break;
      case 3:
        hora = 14; minuto = 50; break;
      case 4:
        hora = 15; minuto = 35; break;
      case 5:
        hora = 16; minuto = 30; break;
      case 6:
        hora = 17; minuto = 15; break;
    }
  } else if (turno == 3) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((getTime() < HoraEpoch(19, 0) ))) {
          hora = 19; minuto = 0;
        } else {
          hora = 7;  minuto = 0;
          amanha = true;
        }
        break;
      case 2:
        hora = 19; minuto = 45; break;
      case 3:
        hora = 20; minuto = 40; break;
      case 4:
        hora = 21; minuto = 25; break;
    }
  }
  if (amanha) {
    timeStampDif = ( ( HoraEpoch(hora, minuto) - getTime()) + nextMidnight(getTime() ) );
  }
  else {
    timeStampDif = ( ( HoraEpoch(hora, minuto) - getTime()) + previousMidnight(getTime() ) );
  }

  breakTime(timeStampDif, tmRestante); // Desmembra a hora do tipo t_time para uma estrutura tmElements_t (tmRestante) contendo: hh/mm/ss...
  return (timeStampDif);               //  retorna o timestamp (epoch time) para o proximo toque

}

void ServidorWeb(){   
  String saidajson = "";
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {  
    PreencheJson();
    delay(5);  
    serializeJson(doc, saidajson); //Envia os dados recebidos para um string json
  
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json; charset=utf-8");
            client.println();
            // the content of the HTTP response follows the header:
            client.print(saidajson);
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }      
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:

    client.stop();
    Serial.println("Client Disconnected.");
    doc.clear();
  }
}
