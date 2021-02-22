boolean DiadeAula () {
  byte mes = month();
  byte dia = day();
  byte diaAula = true;

  if ((weekday() == 1 || weekday() == 7)) // Sem aulas nos domingos (1) ou sabados (7)
    diaAula = false;

  if (diaAula) {                 // Sem aulas nos feriados
    if (mes == 1 && (dia == 1 || dia == 25))   // Feriado confrat. universal e feriado municipal em Sâo Paulo do Potengi
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

void Imprime_Info_DataHora_Serial(){
  if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      Serial.print(digitalClockDisplayStringData(weekday(), day(), month(), year()));
      Serial.print(F(" | "));
      Serial.println(digitalClockDisplayStringHora(hour(), minute(), second()));
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

time_t HoraEpoch(byte hh, byte mm) {
  tmToque.Year = year() - 1970;
  tmToque.Month = month();
  tmToque.Day = day();
  tmToque.Hour = hh;
  tmToque.Minute = mm;
  tmToque.Second = 0;
  return makeTime(tmToque);         //converte a hora da estrutura tmElements_t para o tipo time_t (epoch time)
}

byte AulaAtualNum () {
  // retorna o horario atual da aula.
  // Retorna 0 caso fora de horario de aula e 10 ou 20 para indicar primeiro ou segundo intervalo
  unsigned long hora = now();
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
    if ((turno == 3) && (opcao == 1) && (now() > HoraEpoch(21, 25))) {
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
  unsigned long hora = now();
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
        if ((now() < HoraEpoch(11, 15)))
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
        if ((now() < HoraEpoch(17, 15)))
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
        if ((now() < HoraEpoch(21, 25))) {
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
          if (now() >= HoraEpoch(22, 15))
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
  if (isPM()) {
    if (hour() >= 18)
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

  if (turno == 1) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((now() < HoraEpoch(7, 0) ))) {
          tmToque.Hour = 7; tmToque.Minute = 0;
        } else {
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
  } else if (turno == 2) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((now() < HoraEpoch(13, 0) ))) {
          tmToque.Hour = 13; tmToque.Minute = 0;
        } else {
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
  } else if (turno == 3) {
    switch (proxAula) {
      case 1:
        if ( (atualAula == 0) && ((now() < HoraEpoch(19, 0) ))) {
          tmToque.Hour = 19; tmToque.Minute = 0;
        } else {
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
  if (amanha) {
    timeStampDif = ( ( HoraEpoch(tmToque.Hour, tmToque.Minute) - now()) + nextMidnight(now() ) );
  }
  else {
    timeStampDif = ( ( HoraEpoch(tmToque.Hour, tmToque.Minute) - now()) + previousMidnight(now() ) );
  }

  breakTime(timeStampDif, tmRestante); // Desmembra a hora do tipo t_time para uma estrutura tmElements_t (tmRestante) contendo: hh/mm/ss...
  return (timeStampDif);               //  retorna o timestamp (epoch time) para o proximo toque

}


String digitalClockDisplayStringHora(byte hh, byte mm, byte ss) {
  return Printzero(hh) + ":" + Printzero(mm) + ":" + Printzero(ss);
}

String digitalClockDisplayStringData(byte wday, byte DD, byte MM, int YYYY) {
  return diaSemana(wday) + ", " + Printzero(DD) + "/" + Printzero(MM) + "/" + (YYYY);
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

String NTP_Status_Desc (byte timestatus){
  //timeStatus() retorna: 0 timeNotSet, 1 timeNeedsSync ou 2 timeSet
  switch (timestatus) {
    case 0:
      return "TimeNotSet";
      break;
    case 1:
      return "timeNeedsSync";
      break;
    default:
      return "timeSet";
      break;
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
  Serial.println(timeServer);
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

void sendNTPpacket(IPAddress &address) // send an NTP request to the time server at the given address
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

void Uptime_Calc(){ 
  //Calculo SystemUptime
  if ( millis() - prevMillis > 1000 ) {
    // increment previous milliseconds
    prevMillis += 1000;
    // increment up-time counter
    UpTime += 1;
  }
}


void ServidorWeb () {
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
          client.println(F("Content-Type: application/json; charset=utf-8"));
          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
          //client.println(F("Refresh: 15"));        // refresh the page automatically every 15 sec
          client.println(F(""));           
          client.print(F("{\"info\":{\"dispositivo\":{\"nome\":\""));
          client.print(dispositivo_nome);
          client.print(F("\",\"local\":\""));            
          client.print(dispositivo_local);
          client.print(F("\",\"versao\":\""));
          client.print(dispositivo_versao);
          client.print(F("\",\"uptime\":"));
          client.print(UpTime);
          client.print(F("},\"ntp\": {\"server\": \""));
          client.print(timeServer);
          client.print(F("\",\"status\":"));
          client.print(timeStatus());
          client.print(F(",\"status_desc\":\""));
          client.print(NTP_Status_Desc(timeStatus()));
          client.print(F("\",\"ultima_sincr\":"));
          client.print(ultima_sincronizacao + 3600 * (abs(timeZone)));    //Devolve a hora em UTC
          client.print(F(",\"datahora\":\""));
          client.print(digitalClockDisplayStringData(weekday(), day(), month(), year()) + " | " + digitalClockDisplayStringHora(hour(), minute(), second()));
          client.print(F("\",\"unixtime\":"));
          client.print(now() + 3600 * (abs(timeZone)));                   //Devolve a hora em UTC
          client.print(F("},\"horarios_aula\":{\"dia_de_aula\":"));
          client.print(DiadeAula());
          client.print(F(",\"aula_atual\":{\"horario\":\""));
          client.print(AulaStr(atualAula, 0));
          client.print(F("\",\"duracao\":\""));
          client.print(HorarioAulaStr(atualAula, 0));          
          client.print(F("\"},\"proxima_aula\":{\"horario\":\""));
          client.print(AulaStr(proxAula, 1));
          client.print(F("\",\"duracao\":\""));
          client.print(HorarioAulaStr(proxAula, 1));
          client.print(F("\",\"tempo_prox_aula\":\""));
          client.print(Printzero(tmRestante.Hour) + ":" + Printzero(tmRestante.Minute) + ":" + Printzero(tmRestante.Second));
          client.print(F("\"}}}}"));
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
