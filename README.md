# iF-toque
Sistema para controle de toque escolar (sineta/sirene) baseado em esp32 com uso de rele com atualização de hora pela rede através de protocolo NTP.

As horas dos toques ficam no código fonte na função timeToStudy() e são separadas por toque longo ou toque curto. O toque curto é utilizado quando não há intervalos entre as aulas e dura por padrão 3s. O toque longo (horário do inicio ou fim do intervalo, inicio de turno, etc) dura por padrão 7s. Feriados fixos estão definos na função DiadeAula () que retorna para timeToStudy() se o toque deve ser acionado ou não.

O servidor web é utilizado como gerenciamento passivo do dispositivo para acompanhar se a hora do dispositivo está correta assim como para acompanhar: horário de aula atual, próxima aula, tempo para próxima aula, data/hora atual, uptime e data/hora atual no formato unix epoch time. Uma requisição http no dispositivo irá retornar um documento em formato aberto json. Outras funções desenvolvidas são puramente visuais para controle e monitoramento via json.

**Utilizado:**
* ESP32 com 30 pinos
* Módulo de Rele 5v
* Servidor NTP (pode ser via Internet ou local)

![Diagrama de funcionamento](iFtoque-ToqueIFRN-SPP-DiagramaFuncionamento2.png)

**Retorno de requisição http**

Uma requisição http irá retornar um documento json
```
{
  "projeto": {
    "nome": "IF-Toque",
    "versao": "v2.5",
    "inicio": "Nov/2017",
    "compilacao": "27/09/2022"
  },
  "sistema": {
    "mac": "7C:9E:BD:48:E6:AC",
    "ip": "10.76.1.202",
    "rssi": -69,
    "uptime": 666000
  },
  "ntp": {
    "ntp_server": "pool.ntp.org",
    "ultima_sincr_unixtime": 1664379432,
    "datahora": null,
    "unixtime": 1664382480
  },
  "horarios_aula": {
    "dia_aula": true,
    "aula_atual": {
      "num": 1,
      "horario": "13:00 até 13:45",
      "turno": 2,
      "turno_str": "Tarde"
    },
    "prox_aula": {
      "num": 2,
      "horario": "13:45 até 14:30",
      "tempo_restante": {
        "str": "0h 17m 0s",
        "unixtime": 1664324220
      }
    }
  },
  "data_hora": {
    "dia_semana_data": "Quarta, 28/09/2022",
    "dia_semana_str": "Quarta",
    "dia_semana_int": 4,
    "data": "28/09/2022",
    "hora": "13:28:00"
  }
}
```
**Gerenciamento opcional com Zabbix e Grafana:**

Os dados json podem ser utilizados para exibição em outras plataformas.

![Zabbix-Toque com Zabbix/Grafana](iFToque-Grafana-Integracao.png)
