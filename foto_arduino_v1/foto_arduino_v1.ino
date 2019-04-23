/*
  Espectrofotometro baseado em Arduino | v1.1.5 - 23/04/2019
  Escrito por: Samuel Heimbach Campos | IFMS - campus Corumbá
  Descrição : O código é referente a um espectrofotometro que realiza leituras no espectro vísivel.
  Hardware recomendado: Placa Arduino UNO, sensor de luz TEMT6000, 4 LEDs (ou 1 LED RGB e 1 LED amarelo), um relé para a lâmpada, uma lâmpada.
  Observação: Para gravação dos dados foi programado um aplicativo, que está no mesmo repositório.
*/

/* IMPORTAÇÃO DE BIBLIOTECAS */
#include <Stepper.h> // Motor de passo
Stepper motor(500, 8, 10, 9, 11);  //Inicializa o motor com os passos por volta e os pinos a serem utilizados

#include <SimpleKalmanFilter.h> // Biblioteca do Filtro de Kalman
                                // Mais informações sobre a biblioteca: https://github.com/denyssene/SimpleKalmanFilter
double e_mea = 4.386655229; // Desvio padrão dos dados, calculado previamente mediante medições de teste
SimpleKalmanFilter kalman(e_mea, e_mea, 0.001); // Inicialização do objeto com os valores

/* VARIAVEIS GLOBAIS */
#define ledR 7                  // Define o pino digital 7 como led vere
#define ledG 6                  // Define o pino digital 6 como led verde
#define ledB 5                  // Define o pino digital 5 como led azul
#define rele 3                  // Define o pino do relé
const byte BTpin = 2;           // Define o pino state do bluetooth HC-05
boolean BTconnected = false;    // Instancia e inicializa uma variavel controle
                                // que monitorará se o Bluetooth está ligado
                                
/* SETUP */
void setup() {
  Serial.begin(9600);             // Inicializa o display
  motor.setSpeed(20);             // Define a velocidade do motor

  //Serial.println("Ligou");      // Avisa pelo console que o programa inicializou

  // Define os pinos digitais dos LEDs como saída
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  // Define o pino digital do Bluetooth como INPUT
  pinMode(BTpin, INPUT);
  // Define o pino digital do relé como OUTPUT
  pinMode(rele, OUTPUT);

  // Enquanto a variavel controle BTconnected estiver falsa o BTpin será verificado
  while (!BTconnected) {
    // Se o BTpin estiver com ligado, ou seja, se o Bluetooth estiver conectado
    // A variavel BTconnected é preenchida com TRUE
    // e o led azul (ledB) é ligado
    if (digitalRead(BTpin) == HIGH) {
      BTconnected = true;
      digitalWrite(ledB, HIGH);
    }
  }
}

/* LOOP */
void loop() {
  if (Serial.available() > 0) {
    // lê do buffer o dado recebido:
    char sinal = (char) Serial.read();
    Serial.println("entrou");
    // responde com o dado recebido:
    // Serial.print("rec ");
    // Serial.println(sinal);
    if (sinal == '1') {
      //sinal = "";
      boolean flagVoltar = false;

      digitalWrite(ledB, LOW);
      digitalWrite(ledG, HIGH);
      digitalWrite(rele, HIGH);
      for (int i = 0; i < 1530; i++) {
        sinal = (char) Serial.read();
        if (sinal == '0' || digitalRead(BTpin) == LOW) {
          digitalWrite(rele, LOW);
          for (int j = 0; j < i; j++) {
            delay(300);
            motor.step(8);
            digitalWrite(ledG, LOW);
            digitalWrite(ledR, HIGH);
          }
          i = 1530;
          flagVoltar = true;
        }

        // Realiza a leitura do valor analógico e aplica o Filtro de Kalman para retirada de ruído do sensor
        int Van = analogRead(A0);
        int sinal = (int) kalman.updateEstimate(a);
        
        // Armazena o valor da leitura numa variavel tipo String
        // IMPORTANTE: não mude o bloco de comando abaixo. O aplicativo recebe
        // os dados do dispositivo em formato JSON.
        String valor = "{\"t\":\"";
        valor += sinal;
        valor += "\"";
        valor += "}";
        // Imprime o valor em formato JSON
        Serial.println(valor);

        // IMPORTANTE: não mude o valor do delay. Isso afetará no recebimento dos dados
        // pois está relacionado com a sincronização do aplicativo com o módulo Bluetooth
        delay(300);
        motor.step(-8);

        // Como último valor é enviado um aviso ao aplicativo que o pacote dos valores está completo
        if (i == 1529) {
          String valor = "{\"t\":\"";
          valor += "a";
          valor += "\"";
          valor += "}";
          Serial.print(valor);
        }
      }

      // Retorna para a posição inicial
      if (!flagVoltar) {
        for (int i = 0; i < 1450; i++) {
          digitalWrite(ledG, LOW);
          digitalWrite(ledR, HIGH);
          digitalWrite(rele, LOW);
          delay(10);
          motor.step(8);
        }
      }
      digitalWrite(ledR, LOW);
      digitalWrite(ledB, HIGH);
      digitalWrite(rele, LOW);
    }
  }
}
