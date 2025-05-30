"""
   Atividade 2 de 2 - Curso Utilização do Aplicativo Wokwi (CPS)
   DANIEL RODRIGUES DE SOUSA 30/05/2025
   PROJETO: https://wokwi.com/projects/431587362514477057 */

Quando o Sensor PIR detectar movimento, um LED conectado a um pino digital e um Buzzer
conectado em outro pino digital (poderá usar digitalWrite, PWM ou Tone, escolha uma das
formas) deverão ser ativados e deverá também enviar uma mensagem a um broker MQTT que 
houve detecção de movimento no ambiente, e quando houver ausência de movimento, o LED 
e o Buzzer deverão ser desativados e deverá cessar o envio de mensagens ao broker MQTT. 
Poderá ser programado em linguagem C/C++ do Arduino ou linguagem MicroPhyton (escolha uma
das linguagens). Considere na mensagem enviada ao broker, uma frase contendo seu nome,
por exemplo: “João, detectei movimento!” 
Considere como plataforma de testes, o site do MQTT Cool: https://testclient-cloud.mqtt.cool

"""
import network
import time
import urandom

from machine import Pin, PWM
from umqtt.simple import MQTTClient

# Configuração dos pinos
PIR = Pin(4, Pin.IN)       # Sensor de movimento PIR no pino D4
LED = Pin(12, Pin.OUT)     # LED no pino D12
BUZZER = Pin(14)           # Buzzer no pino D14

# Configuração do servidor MQTT
SERVIDOR     = "test.mosquitto.org"     # Broker MQTT público
MQTT_TOPICO  = "esp/pir_sensor"         # Tópico onde as mensagens serão publicadas

# Função para gerar uma seed aleatória baseada no tempo de conexão
def seed_aleatoria(seed_val):
    urandom.seed(seed_val ^ urandom.getrandbits(16))

# Função para gerar um número aleatório entre min_val e max_val
def numero_aleatorio(min_val, max_val):
    return min_val + urandom.getrandbits(16) % (max_val - min_val + 1)

# Conexão com a rede Wi-Fi
print("Conectando-se ao WiFi ", end="")
sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
sta_if.connect('Wokwi-GUEST', '')  # Rede Wi-Fi simulada do Wokwi

# Medição do tempo de conexão
inicio = time.ticks_us()
while not sta_if.isconnected():
    pass
fim = time.ticks_us()
tempo_conexao = time.ticks_diff(fim, inicio)

print("Conectado!")
print("Tempo de conexão (us):", tempo_conexao)

# Geração de um nome de cliente MQTT aleatório
seed_aleatoria(tempo_conexao)
num_aleatorio = numero_aleatorio(1, 1000)

# Conexão com o broker MQTT
print("Conectando-se ao servidor MQTT... ", end="")
nome_cliente = "ClienteID-" + str(num_aleatorio)
client = MQTTClient(nome_cliente, SERVIDOR, 1883)
client.connect()

print("Conectado! ")
print("Nome do cliente: " + nome_cliente)

# Inicialização de variáveis
mensagem = ''
prev_mensagem = ''
flag_buzzer = 0

# Configuração do PWM para o buzzer
pwm_buzzer = PWM(BUZZER)
pwm_buzzer.freq(1000)

# Loop principal
while True:
    valor = PIR.value()  # Leitura do sensor PIR

    if valor == 0:
        # Sem movimento: desliga LED e buzzer
        LED.off()
        pwm_buzzer.duty(0)
        mensagem = ''
        prev_mensagem = ''
    else:
        # Movimento detectado: liga LED e alterna o buzzer
        LED.on()

        if flag_buzzer == 0:
            flag_buzzer = 1
            pwm_buzzer.duty(512)  # Liga o buzzer
        else:
            flag_buzzer = 0
            pwm_buzzer.duty(0)    # Desliga o buzzer

        # Define a mensagem a ser enviada
        if mensagem == '':
            mensagem = 'Daniel, detectei movimento!'

    # Envia a mensagem MQTT se for nova
    if mensagem != '' and mensagem != prev_mensagem:
        print("Atualizado!")
        print("Enviando ao tópico MQTT {}: {}".format(MQTT_TOPICO, mensagem))
        client.publish(MQTT_TOPICO, mensagem)
        prev_mensagem = mensagem
    else:
        print("Sem atualizações!")

    time.sleep(0.25)  # Pequeno atraso para evitar sobrecarga
