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
        M1_STEP                Y1
        M1_DIRECTION           Y2
        M1_ENABLE              Y3
        M2_SERRA               Y4
        P1_PRENSOR_ENTRADA     Y5 
        P2_CARRINHO_SERRA      Y6
        P3_PRENSOR_SAIDA       Y7
        P4_PUXADOR             Y8

==> ENTRADAS - (4 ENTRADAS)
        A1_RETORNO_CARRINHO    X1
        A2_AVANCO_SERRA        X2
        A3_RETORNO_SERRA       X3
        A4_PUXADOR             X4
        A5_BOTAO_CANCELA       X5

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
   
==> DISPLAY
   1      TAMANHO MADEIRA
   2 - - -ESPESSURA SERRA
   3      TAMANHO CORTE 1
   4 - - - NUMERO CORTE 1
   5      TAMANHO CORTE 2
   6 - - - NUMERO CORTE 2
   7      TAMANHO CORTE 3
   8 - - - NUMERO CORTE 3
   9      TAMANHO CORTE 4
   10 - - -NUMERO CORTE 4
   11                PLAY
   12 - - - - - - - PAUSE
   13               RESET
   14 - - - - - - SOBRA 1
   15             SOBRA 2
   16 - - - - - - SOBRA 3
   17             SOBRA 4
 */

#include <AccelStepper.h>
#include "DWIN_COMM.h"

#define M1_STEP                13
#define M1_DIRECTION           12
#define M1_ENABLE              27
#define M2_SERRA               26
#define P1_PRENSOR_ENTRADA     25 
#define P2_CARRINHO_SERRA      33
#define P3_PRENSOR_SAIDA       32
#define P4_PUXADOR             04

#define A1_RETORNO_CARRINHO    15
#define A2_AVANCO_SERRA        34
#define A3_RETORNO_SERRA       35
#define A4_PUXADOR             05
#define A5_BOTAO_CANCELA       18

#define TAMANHO_MESA           300
#define STEPS_PER_ROTATION     800
#define DISTANCE_PER_ROTATION   1 // TESTAR DISTANCIA EM CM POR ROTACAO
#define VELOCIDADE_MAX_CARRINHO 12000 // steps/s /800 3cm/s
#define ACELERACAO_MAX_CARRINHO 1000 // steps/s/s

DWIN Comm3(115200, &Serial2, true, 2); // 485

AccelStepper stepper(AccelStepper::DRIVER, M1_STEP, M1_DIRECTION);

enum Estados_enum {S0, S1, S2, S3, S4};
Estados_enum estado = S4;

int16_t comando[17]= {0};
float plano_corte[50]={0};
float tamanho_perfil = 0;
float espessura_serra = 0;
int botao_play = 0;
int botao_pause = 0;
int botao_reset = 0;
float posicao_atual = 0;
int indice_plano_corte = 0;
float sobra = 0;
float perda = 0;

uint16_t IHM_STA_Pagina = 0;
void maquina_estados();

void ESTADO_INICIAL();
void zera_comando();
void read_buttons();
void reset();
void read_comando();
void print_comando();
void cria_plano_corte();
void print_plano_corte();
void calcula_perda();
void calcula_sobra();

void EMPURRA_MADEIRA(float distancia);
void calcula_posicao_atual(float deslocamento);
void moveDistancia();
void calcula_resto();

void CORTE_AVANCO();

void CORTE_RETORNO();

void RETORNO();
void retorna_carrinho();
void zera_posicao_atual();
void para_carrinho();

void setup() {
  Serial.begin(115200); // (USB)
  Serial2.begin(115200); //(485)
  pinMode(2, OUTPUT);
  Serial.println("Initialized");

  stepper.setEnablePin(M1_ENABLE);
  stepper.setPinsInverted(false, false, false); // direction, steps, enable
  stepper.setMaxSpeed(VELOCIDADE_MAX_CARRINHO); // steps per second 2cm/s
  stepper.setAcceleration(ACELERACAO_MAX_CARRINHO);// steps per second per second
  stepper.disableOutputs(); // avoid to heat
    
  pinMode(LED_BUILTIN,           OUTPUT);
  pinMode(M1_STEP,               OUTPUT);
  pinMode(M1_DIRECTION,          OUTPUT);
  pinMode(M1_ENABLE,             OUTPUT);
  pinMode(M2_SERRA,              OUTPUT);
  pinMode(P1_PRENSOR_ENTRADA,    OUTPUT);
  pinMode(P2_CARRINHO_SERRA,     OUTPUT);
  pinMode(P3_PRENSOR_SAIDA,      OUTPUT);
  pinMode(P4_PUXADOR,            OUTPUT);

  pinMode(A1_RETORNO_CARRINHO,    INPUT);
  pinMode(A2_AVANCO_SERRA,        INPUT);
  pinMode(A3_RETORNO_SERRA,       INPUT);
  pinMode(A4_PUXADOR,             INPUT);
  pinMode(A5_BOTAO_CANCELA,       INPUT);

  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, LOW);
  digitalWrite(M1_ENABLE, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);  
}

bool recebe_comando = true;// TESTE

void loop() {
  //calcula_posicao_atual();
  Serial.print("Posição atual:  ");
  Serial.println(posicao_atual);
  Serial.print("Sobra:  ");
  Serial.println(sobra);
  print_comando();
  read_buttons();
  if(botao_pause == 1){
    reset(); 
  }
  if(botao_reset == 1){
    reset();
    botao_reset = 0;  
  }
  maquina_estados();
  delay(1000);
}

void maquina_estados(){
  switch(estado){
    case S0://   check if it is in origin
      para_carrinho();
      zera_posicao_atual();
      read_comando();
      calcula_sobra();
      ESTADO_INICIAL();
      if (botao_play == 1){
        botao_play = 0;
        read_comando();
        print_comando();
        cria_plano_corte();
        print_plano_corte();
        calcula_perda();
        estado = S1;
      }
      break;

     case S1:
      para_carrinho();
      calcula_resto(plano_corte[indice_plano_corte]);
      Serial.print("Corte:  ");
      Serial.println(indice_plano_corte);
      if((sobra > plano_corte[indice_plano_corte]) && (posicao_atual - plano_corte[indice_plano_corte] > 0)){
        EMPURRA_MADEIRA(plano_corte[indice_plano_corte]);
        calcula_posicao_atual(plano_corte[indice_plano_corte]); //TESTE
        estado = S2;
      }
      else{
        Serial.print(posicao_atual - plano_corte[indice_plano_corte]);
        Serial.println("Não é possível realizar mais cortes");
        estado = S0;
      }
      break;

     case S2:
      para_carrinho();
      if(A2_AVANCO_SERRA || true){// TRUE SO PARA TESTE
        CORTE_AVANCO();
        estado = S3;
      }
      break;

     case S3:
     para_carrinho();
     if(A3_RETORNO_SERRA || true){// TRUE SO PARA TESTE
      CORTE_RETORNO();
      indice_plano_corte++;
      if(plano_corte[indice_plano_corte] != 0){
        estado = S1;
      }
      else{
       estado = S4;
      }

     }
      break;
      
     case S4:
      para_carrinho();
      RETORNO();
      zera_posicao_atual();
      estado = S0;
      break;
  }
}

void ESTADO_INICIAL(){ 
  Serial.println("ESTADO_INICIAL");
        
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);
}

void EMPURRA_MADEIRA(float distancia){// TODO adicionar e calibrar avanço do motor de passo em cm
  Serial.print("EMPURRA_MADEIRA ");
  Serial.println(distancia);
      
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, HIGH);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);
  Serial.print("AVANÇO ");
  Serial.println(distancia);
  moveDistancia(distancia);
}

void CORTE_AVANCO(){ 
  Serial.println("CORTE_AVANCO ");
        
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, LOW);
  digitalWrite(M2_SERRA, HIGH);
  digitalWrite(P1_PRENSOR_ENTRADA, HIGH);
  digitalWrite(P2_CARRINHO_SERRA, HIGH);
  digitalWrite(P3_PRENSOR_SAIDA, HIGH);
  digitalWrite(P4_PUXADOR, LOW);     
}

void CORTE_RETORNO(){// TODO serra ligada na volta?
  Serial.println("CORTE_RETORNO ");
        
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, HIGH);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, HIGH);
  digitalWrite(P4_PUXADOR, HIGH);     
}

void RETORNO(){// TODO adicionar motor de passo
  Serial.println("RETORNO ");
        
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M1_DIRECTION, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW); 
  retorna_carrinho();
}

void zera_comando(){
  for(int i=1;i<18;i++){
    comando[i] = 0;
  }
}

void reset(){
  for(int i=1;i<18;i++){
      Comm3.write(i, 0);
  }
}

void calcula_sobra(){
  float sobra = comando[1];
  for(int i=3;i<7;i++){
    sobra = sobra - (comando[i]*comando[i+4]);
    Comm3.write(i+8, sobra);
  }
}

void read_buttons(){
  botao_play =  comando[15];
  botao_pause = comando[16];
  botao_reset = comando[17];
}
void read_comando(){
  for(int i=1;i<18;i++){
      Comm3.read(i, &comando[i]);
  }
}

void print_comando(){
  for(int i=1;i<18;i++){
    Serial.print("posicao ");
    Serial.print(i);
    Serial.print(":  ");
    Serial.println(comando[i]);
  }
  Serial.println("");
  Serial.println("");
  Serial.println("");
}

void cria_plano_corte(){
  tamanho_perfil = comando[1];
  espessura_serra = comando[2];
  int index_plano_corte = 1;
  sobra = tamanho_perfil;
  plano_corte[0] =  TAMANHO_MESA-tamanho_perfil;
  for(int tipo_corte=7 ; tipo_corte<=10; tipo_corte++){//TODO inverter tipo de corte com tamanho corte
    for(int num_corte=0; num_corte<comando[tipo_corte-4]; num_corte++,index_plano_corte++){
      plano_corte[index_plano_corte] = comando[tipo_corte];
      }
   }
  indice_plano_corte = 0; 
} 

void print_plano_corte(){
  for(int i=0;plano_corte[i] !=0;i++){
    if(i>0 && plano_corte[i-1] == plano_corte[i]){
      Serial.print(" - ");
      Serial.print(plano_corte[i]);
    }else{
      Serial.println("");
      Serial.print(plano_corte[i]);
    }
  }
}

void calcula_perda(){// TODO CALCULAR A PERDA
  perda = espessura_serra;
}

void calcula_resto(float distancia){
  sobra = sobra - distancia;// - perda;
}

void para_carrinho(){
  stepper.stop();
  stepper.disableOutputs();
}

void zera_posicao_atual(){
  posicao_atual = tamanho_perfil;
}

void calcula_posicao_atual(float deslocamento){
  //posicao_atual = TAMANHO_MESA - (stepper.currentPosition()* STEPS_PER_ROTATION / DISTANCE_PER_ROTATION);
  posicao_atual = posicao_atual - deslocamento;
}

void moveDistancia(float distancia) {//TODO solve speed limit
  stepper.move(distancia * STEPS_PER_ROTATION / DISTANCE_PER_ROTATION); //moveTo for absolute position
}

void retorna_carrinho(){
  while(!A1_RETORNO_CARRINHO || false){//TESTE
    stepper.setSpeed(-4000);
    stepper.runSpeed();
    }//teste
  Serial.print("Retornei para o início");
}