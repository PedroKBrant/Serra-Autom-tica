/*
          SERRA AUTOMATICA
            by Pedro Brant
             30/10/2021
             ____
            |  A2|
 ___________|    |_________
|           |    |       A1|
|  P4  P3   | M2 | P1      |M1
|____A4_____|    |_________|
            |    |
            |__A3|
              P2
                                     
==> SAÍDAS - (8 SAÍDAS)
    - M1 - MOTOR DE PASSO      -> 3 SAÍDAS DIGITAIS
    - M2 - MOTORES SERRA       -> 1 SAÍDA DIGITAL
    - P1 - PRENSOR ENTRADA     -> 1 SAÍDA DIGITAL
    - P2 - CARRINHO SERRA      -> 1 SAÍDA DIGITAL
    - P3 - PRENSOR SAÍDA       -> 1 SAÍDA DIGITAL
    - P4 - PUXADOR             -> 1 SAÍDA DIGITAL

==> ENTRADAS - (4 ENTRADAS)
    - A1 - RETORNO ALIMENTAÇÃO -> 1 ENTRADA DIGITAL
    - A2 - AVANÇO SERRA        -> 1 ENTRADA DIGITAL
    - A3 - RETORNO SERRA       -> 1 ENTRADA DIGITAL
    - A4 - PUXADOR             -> 1 ENTRADA DIGITAL
    - A5 - BOTÃO CANCELA       -> 1 ENTRADA DIGITAL

==> ESTADOS
    - S0 - ESTADO INICIAL  
    - S1 - EMPURRA MADEIRA 
    - S2 - CORTE AVANÇO
    - S3 - CORTE RETORNO
    - S4 - RETORNO
    
==> FUNÇÕES

- = - = - = - = - = - = - OPS010  - = - = - = - = - = - = -

==> PORTAS
    - 08 ENTRADAS DIGITAIS 24VDC OPTOACOPLADAS
    - 08 SAÍDAS DIGITAIS 24VDC TRANSISTORIZADAS
    - 01 ENTRADA ANALÓGICA 0-10V
    - 01 SAÍDA ANALÓGICA 0-10V
    
==> MAPEAMENTO PINOS  
ESP32 - - - - - - OPS010 
     SAÍDA DIGITAL
 13                 Y1
 12 - - - - - - - - Y2
 27                 Y3
 26 - - - - - - - - Y4
 25                 Y5
 33 - - - - - - - - Y6
 32                 Y7
 04 - - - - - - - - Y8
 
    ENTRADA DIGITAL 
 15                 X1
 34 - - - - - - - - X2
 35                 X3
 05 - - - - - - - - X4
 18                 X5
 19 - - - - - - - - X6
 14                 X7
 39 - - - - - - - - X8
 36                A|1
 23 - - - - - - - -A|2
 
 02           ENTX_pin
 22 - - - - - - TTL RX
 21             TTL TX
 S1 - - - - - - -  USB 
 S2              RS485
 S3 - - - - - - - -TTL
                
*/

#define M1_AVANCO              13
#define M1_RETORNO             12
#define M1_PASSO               27
#define M2_SERRA               26
#define P1_PRENSOR_ENTRADA     25 
#define P2_CARRINHO_SERRA      33
#define P3_PRENSOR_SAIDA       32
#define P4_PUXADOR             04

#define A1_RETORNO_ALIMENTACAO 15
#define A2_AVANCO_SERRA        34
#define A3_RETORNO_SERRA       35
#define A4_PUXADOR             05
#define A5_BOTAO_CANCELA       18


int S0(){ // ESTADO INICIAL
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);  
  GET_CORTES( cortes, cortes_size);   
  return NULL;
}
int S1(){ // EMPURRA MADEIRA
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);     
  return NULL;
}
int S2(){ // CORTE AVANÇO
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);     
  return NULL;
}
int S3(){ // CORTE RETORNO
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);     
  return NULL;
}
int S4(){ // RETORNO
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);     
  return NULL;
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN,           OUTPUT);
  pinMode(M1_AVANCO,             OUTPUT);
  pinMode(M1_RETORNO,            OUTPUT);
  pinMode(M1_PASSO,              OUTPUT);
  pinMode(M2_SERRA,              OUTPUT);
  pinMode(P1_PRENSOR_ENTRADA,    OUTPUT);
  pinMode(P2_CARRINHO_SERRA,     OUTPUT);
  pinMode(P3_PRENSOR_SAIDA,      OUTPUT);
  pinMode(P4_PUXADOR,            OUTPUT);

  pinMode(A1_RETORNO_ALIMENTACAO, INPUT);
  pinMode(A2_AVANCO_SERRA,        INPUT);
  pinMode(A3_RETORNO_SERRA,       INPUT);
  pinMode(A4_PUXADOR,             INPUT);
  pinMode(A5_BOTAO_CANCELA,       INPUT);

  Serial.begin(115200);
 
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}