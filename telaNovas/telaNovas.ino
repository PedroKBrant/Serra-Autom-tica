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
#define STEPS_PER_ROTATION     800.0
#define DISTANCE_PER_ROTATION   1.0 // TESTAR DISTANCIA EM CM POR ROTACAO
#define VELOCIDADE_MAX_CARRINHO 4000 // steps/s  10cm/s
#define ACELERACAO_MAX_CARRINHO 2000 // steps/s/s
#define TEMPO_DEBOUNCE 10 //ms

DWIN Comm3(115200, &Serial2, true, 2); // 485

AccelStepper stepper(AccelStepper::DRIVER, M1_STEP, M1_DIRECTION);

enum Estados_enum {S0, S1, S2, S3, HALT};
Estados_enum estado = S3;

uint16_t IHM_STA_Pagina = 0;

int16_t comando[17]= {0};
int16_t comando_manual[6] = {0};
int16_t saidas[8] = {0};
int16_t entradas[8] = {0};

bool play_button = false;

float plano_corte[35]={0};
int16_t indice_plano_corte = 0;

unsigned long timestamp_ultimo_acionamento = 0;

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
      // TODO add check pause here
      maquina_estados();
      MANUAL_zera_comando();
      break;
      
     case 5:
      Serial.println("Operador Manual");
      
      MANUAL_read_comando();
      MANUAL_empurra_retorno();
      MANUAL_empurra_avanco();     
      MANUAL_corte();
      MANUAL_reset();  
      MANUAL_zera_botoes();
       
      TESTE_zera_saidas(); 
      break;
      
     case 7:
      Serial.println("Modo Testes");
      
      TESTE_read_saidas();
      TESTE_write_saidas();
      TESTE_read_entradas();
      break;
  }
}

//------------------------------------------
// OPERADOR AUTOMATICO

void maquina_estados(){
  switch(estado){
    case S0:
      ESTADO_INICIAL();
      //AUTOMATICO_retorna_carrinho(); // TODO Checar necessidade  
      AUTOMATICO_read_comando();
      UTILS_calculate_sobra();
      play_button = AUTOMATICO_check_buttons();
      if(play_button){
        AUTOMATICO_print_comando();
        Comm3.write(15, 0);
        estado = S1;
      }
      break;

     case S1:
      if(true){//(posicao_atual - plano_corte[indice_plano_corte] > 0)){estado != HALT && (sobra > plano_corte[indice_plano_corte]) && 
        Serial.println(plano_corte[indice_plano_corte]);
        EMPURRA_MADEIRA(plano_corte[indice_plano_corte]);
        //calcula_posicao_atual(plano_corte[indice_plano_corte]); //TESTE
        estado = S2;
      }
      else{
        //Serial.print(posicao_atual - plano_corte[indice_plano_corte]);
        Serial.println("Não é possível realizar mais cortes");
        estado = S0;
      }
      break;

     case S2:
      if(digitalRead(A2_AVANCO_SERRA)|| true){// TRUE SO PARA TESTE
        AUTOMATICO_corte();
        indice_plano_corte++;
        if(plano_corte[indice_plano_corte] != 0){
          estado = S1;//  NOVO CORTE
        }
        else{
         estado = S3;// RETORNA
        }
      }
      break;
      
     case S3:
      RETORNO();
      AUTOMATICO_retorna_carrinho();
      estado = S0;
      break;
     
     case HALT:
      break;
  }
}

void AUTOMATICO_read_comando(){
  for(int i=1;i<18;i++){
      Comm3.read(i, &comando[i]);
  }
}

void AUTOMATICO_print_comando(){
  for(int i=1;i<18;i++){
    Serial.print("posicao ");
    Serial.print(i);
    Serial.print(":  ");
    Serial.println(comando[i]);
  }
  Serial.println("");

  Serial.println("");
}

void AUTOMATICO_cria_plano_corte(){
  //tamanho_perfil = comando[1];
  //espessura_serra = comando[2];
  int index_plano_corte = 1;
  //sobra = tamanho_perfil;
  plano_corte[0] =  TAMANHO_MESA-comando[1];
  for(int tipo_corte=7 ; tipo_corte<=10; tipo_corte++){//TODO inverter tipo de corte com tamanho corte
    for(int num_corte=0; num_corte<comando[tipo_corte-4]; num_corte++,index_plano_corte++){
      plano_corte[index_plano_corte] = comando[tipo_corte];
      }
   }
  indice_plano_corte = 0; 
} 

void AUTOMATICO_print_plano_corte(){
  Serial.print(plano_corte[0]);
  for(int i=1;plano_corte[i] !=0;i++){
    if(i>0 && plano_corte[i-1] == plano_corte[i]){
      Serial.print(" - ");
      Serial.print(plano_corte[i]);
    }else{
      Serial.println("");
      Serial.print(plano_corte[i]);
    }
  }
}

bool AUTOMATICO_check_buttons(){
  if(comando[17] == 1){ // RESET
    AUTOMATICO_zera_comando();
    return false;
  }
  if(comando[16] == 1){ // PAUSE
    Comm3.write(16, 0);
    AUTOMATICO_zera_comando();
    return false;
  }
  if(comando[15] == 1){ //PLAY
    Comm3.write(15, 0);
    AUTOMATICO_read_comando();
    AUTOMATICO_print_comando();
    AUTOMATICO_cria_plano_corte();
    AUTOMATICO_print_plano_corte();
    return true;
  }
}

void AUTOMATICO_zera_comando(){
  for(int i=1;i<18;i++){
      Comm3.write(i, 0);
  }
}

void AUTOMATICO_corte(){
  if(!digitalRead(A2_AVANCO_SERRA)){
      while(digitalRead(A3_RETORNO_SERRA)){
        CORTE_AVANCO();
      }
      CORTE_RETORNO();
      delay(5000);
      ESTADO_INICIAL();
      Comm3.write(22, 0);
    }
}

void AUTOMATICO_moveDistance(float distance, float serra, bool frente) {
  int count1 = 0;
  stepper.enableOutputs();
  distance = UTILS_calculate_distance(distance, serra);// TODO PRECISA DISSO NA VOLTA?
  stepper.move(distance * STEPS_PER_ROTATION);
  while(stepper.distanceToGo() != 0){
    count1++;
    if(count1%800 == 0 && 
      (((digitalRead(A1_RETORNO_CARRINHO) == 0) && (frente == false)) || 
      ((digitalRead(A4_PUXADOR) == 0) && (frente == true))) ){
      stepper.setCurrentPosition(0);
      break;
    }
    stepper.run();
  }
  stepper.disableOutputs();
}

void AUTOMATICO_retorna_carrinho(){
  Serial.print("Voltando");
  stepper.enableOutputs();
  stepper.move(-240000);// TODO CALCULAR DISTANCIA
  while(digitalRead(A1_RETORNO_CARRINHO) == 1){
    //digitalRead(A1_RETORNO_CARRINHO);
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  stepper.disableOutputs();
  Serial.print("Retornei para o início");
}

//------------------------------------------
// OPERADOR MANUAL

void MANUAL_read_comando(){
  for(uint16_t i=18;i<24;i++){
      Comm3.read(i, &comando_manual[i-18]);
      Serial.print(comando_manual[i-18]);
  } 
  Serial.println("");
}

void MANUAL_zera_botoes(){
  for(uint16_t i=20;i<24;i++){
      Comm3.write(i, 0);
  } 
}

void MANUAL_zera_comando(){
  for(uint16_t i=18;i<24;i++){
      Comm3.write(i, 0);
  } 
}

void MANUAL_empurra_retorno(){
  if(comando_manual[2] == 1){
    AUTOMATICO_moveDistance(-comando_manual[0], comando_manual[1], false);
  }
}

void MANUAL_empurra_avanco(){
  if(comando_manual[3] == 1){
    AUTOMATICO_moveDistance(comando_manual[0], comando_manual[1], true);
  }
}

void MANUAL_corte(){
  if(comando_manual[4] == 1){ 
    AUTOMATICO_corte();
  }
}

void MANUAL_reset(){// Volta para o início
  if(comando_manual[5] == 1){
    AUTOMATICO_retorna_carrinho();
  }
}

//------------------------------------------
// MODO DE TESTES

void TESTE_zera_saidas(){
  for(int i=24;i<32;i++){
    Comm3.write(i, 0);
  }
}

void TESTE_read_saidas(){
  for(uint16_t i=24;i<32;i++){
      Comm3.read(i, &saidas[i-24]);
  }
}

void TESTE_write_saidas(){
  if(saidas[2]==1)        stepper.enableOutputs();
  else if(saidas[2]==0)  stepper.disableOutputs();
  if(saidas[0]==1)      TESTE_moveDistancia(4000);
  digitalWrite(M2_SERRA,           saidas[3]);
  digitalWrite(P1_PRENSOR_ENTRADA, saidas[4]);
  digitalWrite(P2_CARRINHO_SERRA,  saidas[5]);
  digitalWrite(P3_PRENSOR_SAIDA,   saidas[6]);
  digitalWrite(P4_PUXADOR,         saidas[7]);
}

void TESTE_read_entradas(){
  entradas[0] = digitalRead(A1_RETORNO_CARRINHO);
  entradas[1] =     digitalRead(A2_AVANCO_SERRA);
  entradas[2] =    digitalRead(A3_RETORNO_SERRA);
  entradas[3] =          digitalRead(A4_PUXADOR);
  entradas[4] =    digitalRead(A5_BOTAO_CANCELA);
  entradas[5] =          digitalRead(A6_RESERVA);
  entradas[6] =          digitalRead(A7_RESERVA);
  entradas[7] =          digitalRead(A8_RESERVA);
  
  for(int i=32;i<40;i++){
    Comm3.write(i, entradas[i-32]);
  }
}

void TESTE_moveDistancia(float distancia) {
  int count1=0;
  //stepper.enableOutputs();
  stepper.move(saidas[1]?distancia:-distancia); //moveTo for absolute position
  while (stepper.distanceToGo() != 0)// && saidas[0] && saidas[2] && !A1_RETORNO_CARRINHO && !A4_PUXADOR) 
  { 
    /*
    count1++;
    if(count1%1000==0){
      Comm3.read(24, &saidas[0]);
      Comm3.read(25, &saidas[1]);
      Comm3.read(26, &saidas[2]);
    }
    */
    stepper.run();
  }
  stepper.disableOutputs();
}

//------------------------------------------
// UTILS

float UTILS_calculate_distance(float distance, float serra){// TODO Conferir com tio Celo
  return distance>0?distance+(serra/10):distance-(serra/10);
}

void UTILS_calculate_sobra(){// TODO adicionar o calculo de distãncia~?
  float sobra = comando[1]-2;
  for(int i=3;i<7;i++){
    sobra = sobra - (comando[i] * UTILS_calculate_distance(comando[i+4], comando[2]));
    Comm3.write(i+8, sobra);
  }
}

//------------------------------------------
// MAQUINA DE ESTADOS

void ESTADO_INICIAL(){ 
  Serial.println("ESTADO_INICIAL");
        
  stepper.disableOutputs();
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
  AUTOMATICO_moveDistance(distancia, comando[2], true);
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

void CORTE_RETORNO(){
  Serial.println("CORTE_RETORNO ");
        
  stepper.disableOutputs();
  digitalWrite(M2_SERRA, HIGH);
  digitalWrite(P1_PRENSOR_ENTRADA, HIGH);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, HIGH);     
}

void RETORNO(){// TODO adicionar motor de passo
  Serial.println("RETORNO ");
  
  digitalWrite(M1_DIRECTION, LOW);  
  digitalWrite(M1_STEP, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW); 
}

void HALT_STOP(){// TODO adicionar motor de passo
  Serial.println("HALT ");
}

//------------------------------------------
// Função ISR (chamada quando há interrupção)

void IRAM_ATTR funcao_ISR()
{
  /* Conta acionamentos do botão considerando debounce */
  if ( (millis() - timestamp_ultimo_acionamento) >= TEMPO_DEBOUNCE ){
    Serial.print("PERDI MEU BRAÇO");
    digitalWrite(M1_STEP, LOW);
    digitalWrite(M1_DIRECTION, LOW);
    digitalWrite(M2_SERRA, LOW);
    digitalWrite(P1_PRENSOR_ENTRADA, LOW);
    digitalWrite(P2_CARRINHO_SERRA, LOW);
    digitalWrite(P3_PRENSOR_SAIDA, LOW);
    digitalWrite(P4_PUXADOR, LOW); 
    stepper.disableOutputs();
       
    estado = HALT;
    //contador_acionamentos++;
    timestamp_ultimo_acionamento = millis();
  }
}