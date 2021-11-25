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
 
==> TELA
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
#define M1_AVANCO              13
#define M1_RETORNO             12
#define M1_PASSO               27
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

#define TAMANHO_MESA           350

enum Estados_enum {S0, S1, S2, S3, S4};
Estados_enum estado = S0;

void maquina_estados();

void ESTADO_INICIAL();
void cria_plano_corte();
void print_plano_corte();
void calcula_perda();

void EMPURRA_MADEIRA(float distancia);
void calcula_sobra();

void CORTE_AVANCO();

void CORTE_RETORNO();

void RETORNO();



float comando[10]={300,5,5,30,5,20,5,10,0,0};
float plano_corte[50]={0};
float tamanho_madeira = 0;
float espessura_serra = 0;
int indice_plano_corte = 0;
float sobra = 0;
float perda = 0;



void setup() {
  Serial.begin(115200); // (USB)
  Serial1.begin(115200); // (TTL)
  Serial2.begin(115200); //(485)
  Serial.println("Initialized");
  
  pinMode(LED_BUILTIN,           OUTPUT);
  pinMode(M1_AVANCO,             OUTPUT);
  pinMode(M1_RETORNO,            OUTPUT);
  pinMode(M1_PASSO,              OUTPUT);
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

  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);  
}

bool recebe_comando = true;// TESTE

void loop() {
  maquina_estados();
  delay(10);
}

void maquina_estados(){
  switch(estado){
    case S0:
      ESTADO_INICIAL();
      if (recebe_comando){
        cria_plano_corte();
        print_plano_corte();
        calcula_perda();
        estado = S1;
      }
      break;

     case S1:
      calcula_sobra(plano_corte[indice_plano_corte]);
      Serial.print("Corte:  ");
      Serial.println(indice_plano_corte);
      if((sobra > plano_corte[indice_plano_corte]) && (plano_corte[indice_plano_corte] != 0)){
        EMPURRA_MADEIRA(plano_corte[indice_plano_corte]);
        estado = S2;
      }
      else{
        Serial.println("Não é possível realizar mais cortes");
        estado = S0;
      }
      break;

     case S2:
      if(A2_AVANCO_SERRA || true){// TRUE SO PARA TESTE
        CORTE_AVANCO();
        estado = S3;
      }
      break;

     case S3:
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
      RETORNO();
      estado = S0;
      break;
  }
}

void ESTADO_INICIAL(){ 
  Serial.println("ESTADO_INICIAL");
        
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);
}

void EMPURRA_MADEIRA(float distancia){// TODO adicionar e calibrar avanço do motor de passo em cm
  Serial.print("EMPURRA_MADEIRA ");
  Serial.println(distancia);
      
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);
  Serial.print("AVANÇO ");
  Serial.println(distancia);
}

void CORTE_AVANCO(){ 
  Serial.println("CORTE_AVANCO ");
        
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, HIGH);
  digitalWrite(P1_PRENSOR_ENTRADA, HIGH);
  digitalWrite(P2_CARRINHO_SERRA, HIGH);
  digitalWrite(P3_PRENSOR_SAIDA, HIGH);
  digitalWrite(P4_PUXADOR, LOW);     
}

void CORTE_RETORNO(){// TODO serra ligada na volta?
  Serial.println("CORTE_RETORNO ");
        
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, HIGH);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, HIGH);
  digitalWrite(P4_PUXADOR, HIGH);     
}

void RETORNO(){// TODO adicionar motor de passo
  Serial.println("RETORNO ");
        
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);     
}

void cria_plano_corte(){
  tamanho_madeira = comando[0];
  espessura_serra = comando[1];
  int index_plano_corte = 1;
  sobra = tamanho_madeira;
  plano_corte[0] =  TAMANHO_MESA-tamanho_madeira;
  for(int tipo_corte=2 ; tipo_corte<=9; tipo_corte=tipo_corte+2){
    for(int num_corte=0; num_corte<comando[tipo_corte]; num_corte++,index_plano_corte++){
      plano_corte[index_plano_corte] = comando[tipo_corte+1];
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

void calcula_sobra(float distancia){
  sobra = sobra - distancia - perda;
}



