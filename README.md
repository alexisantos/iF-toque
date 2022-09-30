# iF-toque
Sistema para controle de toque escolar (sineta/sirene) baseado em esp32 com uso de rele com atualização de hora pela rede através de protocolo NTP.

As horas dos toques ficam no código fonte na função timeToStudy() e são separadas por toque longo ou toque curto. O toque curto é utilizado quando não há intervalos entre as aulas e dura por padrão 3s. O toque longo (horário do inicio ou fim do intervalo, inicio de turno, etc) dura por padrão 7s. Feriados fixos estão definos na função DiadeAula () que retorna para timeToStudy() se o toque deve ser acionado ou não.

O servidor web é utilizado como gerenciamento passivo do dispositivo para acompanhar se a hora do dispositivo está correta assim como para acompanhar: horário de aula atual, próxima aula, tempo para próxima aula, data/hora atual, uptime e data/hora atual no formato unix epoch time. Uma requisição http no dispositivo irá retornar um documento em formato aberto json. Outras funções desenvolvidas são puramente visuais para controle e monitoramento via json.

**Utilizado:**
* ESP32 com 30 pinos
* Módulo de Rele 5v
* Fonte Ajustável para Protoboard-MB102
* Servidor NTP (pode ser via Internet ou local)

![Diagrama de funcionamento](iFtoque-ToqueIFRN-SPP-DiagramaFuncionamento2.png)

### Retorno de requisição http

Uma requisição http irá retornar um documento json
```
{
  "projeto": {
    "nome": "IF-Toque",
    "versao": "v2.5.1",
    "inicio": "Nov/2017",
    "compilacao": "30/09/2022"
  },
  "sistema": {
    "ip": "10.76.1.202",
    "mac": "7C:9E:BD:48:E6:AC",
    "rssi": -66,
    "uptime": 229300
  },
  "iftoque": {
    "datahora": "Friday, September 30 2022 19:47:07",
    "unixtime": 1664578027
  },
  "ntp": {
    "ntp_server": "pool.ntp.org",
    "sincronizacao": 1664575747
  },
  "horarios_aula": {
    "dia_aula": true,
    "aula_atual": {
      "horario": {
        "num": 2,
        "horario": "19:45 até 20:30"
      },
      "turno": {
        "num": 3,
        "turno": "Noite"
      }
    },
    "prox_aula": {
      "horario": {
        "num": 3,
        "horario": "20:40 até 21:25"
      },
      "tempo_restante": {
        "tempo": "0h 52m 53s",
        "unixtime": 1664499173
      }
    }
  },
  "data_hora": {
    "dia_semana_data": "Sexta, 30/09/2022",
    "dia_semana_str": "Sexta",
    "dia_semana_int": 6,
    "data": "30/09/2022",
    "hora": "19:47:07"
  }
}
```


### Sugestão de prototipagem
![Sugestão de prototipagem](Toque-IFRN-NTP_draw_bb.png)

### Exibição dos dados
Os dados json podem ser utilizados para exibição em outras plataformas.
![Zabbix-Toque com Zabbix/Grafana](iFToque-Grafana-Integracao.png)
