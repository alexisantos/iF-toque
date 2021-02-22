# iF-toque
Sistema para controle de toque escolar (sineta/sirene) baseado em arduino com uso de rele com atualização de hora pela rede através de protocolo NTP.

As horas dos toques ficam no código fonte na função timeToStudy() e são separadas por toque longo ou toque curto. O toque curto é utilizado quando não há intervalos entre as aulas e dura por padrão 3s. O toque longo (horário do inicio ou fim do intervalo, inicio de turno, etc) dura por padrão 7s. Feriados fixos estão definos na função DiadeAula () que retorna para timeToStudy() se o toque deve ser acionado ou não.

O servidor web é utilizado como gerenciamento passivo do dispositivo para acompanhar se a hora do dispositivo está correta assim como para acompanhar: horário de aula atual, próxima aula, tempo para próxima aula, data/hora atual, uptime e data/hora atual no formato unix epoch time. Uma requisição http no dispositivo irá retornar um documento em formato aberto json.

**Utilizado:**
* Arduino Uno
* Módulo de Rele 5v para arduino
* Shield ethernet w5110
* Servidor NTP (local ou da Internet)

![Diagrama de funcionamento](iFtoque-ToqueIFRN-SPP-DiagramaFuncionamento2.png)

**Gerenciamento opcional com Zabbix e Grafana:**
![Zabbix-Toque com Zabbix/Grafana](iFToque-Grafana-Integracao.png)

**Retorno de requisição http**
Uma requisição http irá retornar um documento json
```
{
  "info":{
    "dispositivo":{
      "nome":"if-toque",
      "local":"IFRN-SPP",
      "versao":"v2 b2021021011",
      "uptime":188318
    },
    "ntp":{
      "server":"10.22.0.213",
      "status":2,
      "status_desc":"timeSet",
      "ultima_sincr":1613810251,
      "datahora":"Segunda, 22/02/2021 | 09:56:12",
      "unixtime":1613998572
    },
    "horarios_aula":{
      "dia_de_aula":1,
      "aula_atual":{
        "horario":"4º horário",
        "duracao":"09:35 até 10:20"
      },
      "proxima_aula":{
        "horario":"5º horário",
        "duracao":"10:30 até 11:15",
        "tempo_prox_aula":"00:33:49"
      }
    }
  }
}
```
