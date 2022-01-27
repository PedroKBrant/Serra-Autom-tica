//Chamada das bibliotecas
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
#define A2_AVANCO_SERRA        14
#define A3_RETORNO_SERRA       05
#define A4_PUXADOR             18
#define A5_BOTAO_CANCELA       19
#define A6_RESERVA             36
#define A7_RESERVA             39
#define A8_RESERVA             34

#define TAMANHO_MESA           300
#define STEPS_PER_ROTATION     800
#define DISTANCE_PER_ROTATION   1 // TESTAR DISTANCIA EM CM POR ROTACAO
#define VELOCIDADE_MAX_CARRINHO 800 //12000 // steps/s /800 3cm/s
#define ACELERACAO_MAX_CARRINHO 200 //1000 // steps/s/s

DWIN Comm3(115200, &Serial2, true, 2); // 485

AccelStepper stepper(AccelStepper::DRIVER, M1_STEP, M1_DIRECTION);

uint16_t IHM_STA_Pagina = 0;
int16_t IHM_BTN_Luz = 0;
int16_t saidas[8] = {0};
int16_t entradas[8] = {0};

void setup() {
  Serial.begin(115200); // (USB)
  Serial2.begin(115200); //(485)
  pinMode(2, OUTPUT);

  stepper.setEnablePin(M1_ENABLE);
  stepper.setPinsInverted(false, false, true); // direction, steps, enable
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
  pinMode(A6_RESERVA,             INPUT);
  pinMode(A7_RESERVA,             INPUT);
  pinMode(A8_RESERVA,             INPUT);

}

void loop() {

//Leitura da página atual
  Comm3.page(&IHM_STA_Pagina);
  select_screen(IHM_STA_Pagina);
  
}
void select_screen(uint16_t IHM_STA_Pagina){
  switch(IHM_STA_Pagina){
    case 0:
      Serial.println("Tela Inicial");
      break;
      
     case 1:
      Serial.println("Operador Automático");
      zera_saidas();
      break;
      
     case 5:
      Serial.println("Operador Manual");
      zera_saidas();
      break;
      
     case 7:
      Serial.println("Modo Testes");
      read_saidas();
      write_saidas();
      read_entradas();
      break;
  }
}
void zera_saidas(){
  for(int i=24;i<32;i++){
    Comm3.write(i, 0);
  }
}

void read_saidas(){
  for(uint16_t i=24;i<32;i++){
      Comm3.read(i, &saidas[i-24]);
  }
}

void write_saidas(){
  Serial.println(10*(1-2*saidas[1]));
  if(saidas[0]==1) TESTE_moveDistancia(100);
  if(saidas[1]==1)
  if(saidas[2]==1)       stepper.enableOutputs();
  else if(saidas[2]==0) stepper.disableOutputs();
  //TODO adicionar para o motor de passo
  digitalWrite(M2_SERRA,           saidas[3]);
  digitalWrite(P1_PRENSOR_ENTRADA, saidas[4]);
  digitalWrite(P2_CARRINHO_SERRA,  saidas[5]);
  digitalWrite(P3_PRENSOR_SAIDA,   saidas[6]);
  digitalWrite(P4_PUXADOR,         saidas[7]);
}

void read_entradas(){
  entradas[0] = digitalRead(A1_RETORNO_CARRINHO);
  entradas[1] =     digitalRead(A2_AVANCO_SERRA);
  entradas[2] =    digitalRead(A3_RETORNO_SERRA);
  entradas[3] =          digitalRead(A4_PUXADOR);
  entradas[4] =    digitalRead(A5_BOTAO_CANCELA);
  entradas[5] =          digitalRead(A6_RESERVA);
  entradas[6] =          digitalRead(A7_RESERVA);
  entradas[7] =          digitalRead(A8_RESERVA);
  
  for(int i=32;i<40;i++){
    Serial.print(entradas[i-32]);
    Comm3.write(i, entradas[i-32]);
  }
}

void TESTE_moveDistancia(float distancia) {
  stepper.enableOutputs();
  stepper.move(saidas[1]?distancia:-distancia ); //moveTo for absolute position
  while (stepper.distanceToGo() != 0 && saidas[0] && saidas[2])// && !A1_RETORNO_CARRINHO && !A3_RETORNO_SERRA) 
  { 
    Serial.println(stepper.distanceToGo());
    Comm3.read(24, &saidas[0]);
    Comm3.read(25, &saidas[1]);
    Comm3.read(26, &saidas[2]);
    stepper.run();}
  stepper.disableOutputs();
}
