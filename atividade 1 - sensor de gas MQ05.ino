/* Atividade 1 de 2 - Curso Utilização do Aplicativo Wokwi (CPS)
   DANIEL RODRIGUES DE SOUSA 17/05/2025
   PROJETO: https://wokwi.com/projects/429606843613330433 */

/*
  Montar um circuito com Arduino, um sensor de gás, um display LCD I2C e um buzzer.
  Programar para quando o sensor de gás detectar uma alta concentração de gás, exibir
  a mensagem de vazamento de gás e programar para disparar o buzzer e ativar um led. 
  Quando a concentração cessar, exibir o status de não vazamento, desativar o buzzer 
  e o pino do led. */

#include <LiquidCrystal_I2C.h>

// define o número de colunas e linhas do LCD
#define COLUNAS 16
#define LINHAS  2

// Definição dos pinos de entrada
#define PIN_MQ05    A0    // Pino do sensor de gás MQ05

// Definição dos pinos de saída
#define PIN_BUZZER  3     // Pino de controle do buzzer
#define PIN_LED     4     // Pino de controle do buzzer

#define VALOR_GAS   3000  // Limite de detecção de gás para acionamento do alarme

// Declaração de variáveis
unsigned long int tempo_atual;  // Variável para armazenar o tempo atual
unsigned long int tempo_1ms;    // Temporizador para intervalos de 100ms
unsigned long int tempo_100ms;  // Temporizador para intervalos de 100ms
unsigned long int tempo_500ms;  // Temporizador para intervalos de 500ms
int nivel_gas;                  // Armazena o nível de gás detectado (adc)
float ppm;                      // Armazena o nível de gás detectado (ppm)
bool f_pisca;     // Flag para o pisca do LCD
bool f_buzzer;    // Flag para controle do buzzer

// Define o endereço do LCD, número de colunas e linhas
LiquidCrystal_I2C lcd(0x27, COLUNAS, LINHAS);

// Tabela de valores ADC e ppm
const int adcTable[] = {
  210, 240, 280, 312, 333, 355, 368, 381, 395, 404,
  451, 504, 543, 567, 591, 605, 619, 632, 641, 655,
  664, 668, 677, 685, 690, 694, 702, 707, 711, 715,
  719, 723, 727, 731, 735, 739, 742, 746, 750, 754,
  758, 761, 765, 769, 772, 776, 779, 783, 786, 790,
  793, 796, 800, 803, 806, 809, 813, 816, 819, 822,
  825, 828, 831, 834, 837, 840, 843, 845, 848, 851,
  853, 856, 859, 861, 864, 866, 869, 871, 874, 876,
  879, 881, 883, 886, 888, 890, 892, 894, 896, 898,
  901, 903, 905, 907, 908, 910, 912, 914, 916, 918,
  920, 921, 923, 925, 926, 928, 930, 931, 933, 934,
  936, 937, 939, 940, 942, 943, 945, 946, 947, 949,
  950, 951, 952, 954, 955, 956, 957, 958, 960, 961,
  962, 963, 964, 965, 966, 967, 968, 969, 970, 971,
  972, 973, 974, 975, 976, 977, 978, 979, 980, 981,
  982, 983, 984, 985, 986, 987, 988, 989, 990, 991,
  992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001,
  1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010
};

const float ppmTable[] = {
  0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2,
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26, 28, 29, 30, 32,
  33, 35, 36, 38, 40, 42, 44, 46, 48, 50, 52, 55, 85,
  60, 63, 66, 69, 72, 76, 79, 83, 87, 91, 95, 100, 105,
  110, 115, 120, 126, 132, 138, 145, 151, 158, 166, 174,
  182, 191, 200, 209, 219, 229, 240, 251, 263, 275, 288,
  302, 316, 331, 347, 363, 380, 398, 417, 437, 457, 479,
  501, 525, 550, 575, 603, 631, 661, 692, 724, 759, 794,
  832, 871, 912, 955, 1000, 1047, 1096, 1148, 1202, 1259,
  1318, 1380, 1445, 1514, 1585, 1660, 1738, 1820, 1905,
  1995, 2089, 2188, 2291, 2399, 2512, 2630, 2754, 2884,
  3020, 3162, 3311, 3467, 3631, 3802, 3981, 4365, 4571,
  4786, 5012, 5248, 5754, 6026, 6310, 6918, 7244, 7586,
  8318, 8710, 9550, 10471, 10965, 12023, 13183, 13804,
  15136, 16596, 18197, 19953, 21878, 25119, 27542, 30200,
  34674, 38019, 43652, 50119, 57544, 66069, 79433, 95499
};

// Função de interpolação linear
float interpolatePPM(int adcValue)
{
  int tableSize = sizeof(adcTable) / sizeof(adcTable[0]);

  if (adcValue <= adcTable[0]) return ppmTable[0];
  if (adcValue >= adcTable[tableSize - 1]) return ppmTable[tableSize - 1];

  for(int i = 0; i < tableSize - 1; i++)
  {
    if(adcValue >= adcTable[i] && adcValue <= adcTable[i + 1])
    {
      float slope = (ppmTable[i + 1] - ppmTable[i]) / (adcTable[i + 1] - adcTable[i]);
      return ppmTable[i] + slope * (adcValue - adcTable[i]);
    }
  }

  return -1; // Erro
}

void setup()
{
  // Configura os pinos como saída
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Configura os pinos como entrada
  pinMode(PIN_MQ05, INPUT);

  // Inicializa o LCD
  lcd.init();

  // Liga a luz de fundo do LCD
  lcd.backlight();
}

void loop()
{
  tempo_atual = millis();             // Atualiza o tempo atual

  if((tempo_atual - tempo_1ms) >= 1)  // Temporização a cada 1ms
  {
    tempo_1ms = millis(); // Recarrega o temporizador de 1ms
    if (f_buzzer) digitalWrite(PIN_BUZZER, !digitalRead(PIN_BUZZER));
    else digitalWrite(PIN_BUZZER, LOW);
  }

  if((tempo_atual - tempo_100ms) >= 100)  // Temporização a cada 100ms
  {
    tempo_100ms = millis(); // Recarrega o temporizador de 100ms
    nivel_gas = analogRead(PIN_MQ05);
    ppm = interpolatePPM(nivel_gas);
  }

  if((tempo_atual - tempo_500ms) >= 500)  // Temporização a cada 500ms
  {
    tempo_500ms = millis(); // Recarrega o temporizador de 500ms

    lcd.setCursor(0, 0);
    lcd.print("ppm =         ");
    lcd.setCursor(6, 0);
    lcd.print(ppm);

    lcd.setCursor(0, 1);
    if(ppm > VALOR_GAS)   // Verifica se o nível de gás excede o limite
    {
      if (f_pisca)
      {
        lcd.print("VAZAMENTO DE GAS");
      }
      else
      {
        lcd.print("                ");
      }

      digitalWrite(PIN_LED, HIGH);
      f_pisca = !f_pisca;
      f_buzzer = 1;
    }
    else
    {
      lcd.print(" SEM VAZAMENTO  ");
      digitalWrite(PIN_LED, LOW);
      f_pisca = 0;
      f_buzzer = 0;
    }
  }
}
