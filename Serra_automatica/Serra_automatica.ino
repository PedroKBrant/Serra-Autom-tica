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

/*
int cortes[10];
int cortes_size = 10;

void GET_CORTES(int * cortes, int cortes_size){ //TODO AJUSTAR PARA VARIOS CORTES 
  Serial.println("Insira o tamanho do perfil:");   // Prompt User for input
  while (Serial.available()==0){}                  // wait for user input
  cortes[0] = Serial.parseInt();                   // Read user input and hold it in a variable
  cortes[0] = 10;
  Serial.println(cortes[0]);
  cortes[1] = cortes[0];
  Serial.println("Insira o tamanho do corte 1:");   
  while (Serial.available()==0){}                 
  cortes[2] = Serial.parseInt();                  

  Serial.println("Insira o número de cortes do tipo 1:");
  while (Serial.available()==0){}                 
  cortes[3] = Serial.parseInt();

  if(CALCULATE_CORTES(cortes, cortes_size)){
    Serial.print("tamanho do perfil:");                 
    Serial.print(cortes[0]);
    Serial.print("Sobras:");                 
    Serial.print(cortes[1]);
  }
  else{
    Serial.print("Não é possível realizar plano de corte");
  }
}

bool CALCULATE_CORTES(int * cortes, int cortes_size){
  int perda = 5;
  cortes[1] = cortes[0] - (cortes[2]*cortes[3] + perda*cortes[3]);
  if(cortes[1] >= 0){
    return true;
  }
  return false;
}
*/
float tamanho_madeira = 300;
float largura_mesa = 350;
float perda_corte = 0.3;
//float tamanho_corte[5];
//float numero_cortes[5];
float tamanho_corte[] = {30,20,0,0,0};
float numero_cortes[] = {5,5,0,0,0};
float sobra = 0;
float posicao_carrinho = 350;

void calcula_posicao_carrinho(float deslocamento){
  posicao_carrinho = posicao_carrinho - deslocamento;
  Serial.println(deslocamento);
  Serial.println(posicao_carrinho);
}
float calcula_sobra(){
  float total_corte = 0;
  for(int i=0;i<5;i++){
    total_corte = total_corte+(tamanho_corte[i]*numero_cortes[i])+(numero_cortes[i]*perda_corte);
  }
  sobra = tamanho_madeira - total_corte;
  return sobra;
}
int calcula_tipos_cortes(){
  int tipos = 0;
  for(int i=0;i<5;i++){
    Serial.println(tamanho_corte[i]);
    if(tamanho_corte[i] != 0 && numero_cortes[i] != 0){
      tipos++;
    }
  }
  return tipos;
}
void zera_cortes(){
  for(int i=0;i<5;i++){
    tamanho_corte[i] = 0;
    numero_cortes[i] = 0;
  }
}

void motor_passo(int distancia){
  digitalWrite(M1_AVANCO, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  for(int i=0;i<distancia;i++){
    digitalWrite(M1_PASSO, HIGH);   
    delay(1);
    digitalWrite(M1_PASSO, LOW);
    delay(1);
  }
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  for(int i=0;i<distancia;i++){
    digitalWrite(M1_PASSO, HIGH);   
    delay(1);
    digitalWrite(M1_PASSO, LOW);
    delay(1);
  }
}

int ESTADO_INICIAL(){ 
  bool recebe_cortes = true;
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);  
  //GET_CORTES(cortes, cortes_size);
  while(recebe_cortes == false){}   
  return NULL;
}

int EMPURRA_MADEIRA(float distancia){ 
  digitalWrite(M1_AVANCO, LOW);
  digitalWrite(M1_RETORNO, LOW);
  digitalWrite(M1_PASSO, LOW);
  digitalWrite(M2_SERRA, LOW);
  digitalWrite(P1_PRENSOR_ENTRADA, LOW);
  digitalWrite(P2_CARRINHO_SERRA, LOW);
  digitalWrite(P3_PRENSOR_SAIDA, LOW);
  digitalWrite(P4_PUXADOR, LOW);
  
  //motor_passo(distancia);
  calcula_posicao_carrinho(distancia);
  
  return NULL;
}

int CORTE_AVANCO(){ 
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

int CORTE_RETORNO(){ 
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

int RETORNO(){ 
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
  Serial.println("Initialized");
}

// the loop function runs over and over again forever
void loop() {
  int tipos_cortes = calcula_tipos_cortes();
  Serial.println("NUM CORTES ");
    Serial.println(tipos_cortes);
  zera_cortes();
  
  ESTADO_INICIAL();
  Serial.println("ESTADO_INICIAL");
  EMPURRA_MADEIRA(largura_mesa-tamanho_madeira);
  Serial.println("EMPURRA_MADEIRA");
  for(int tipo=0; tipo<tipos_cortes;tipo++){
    Serial.println(tamanho_corte[tipo]);
    for(int numero;numero<numero_cortes[tipo];numero++){
      Serial.println("EMPURRA_MADEIRA");
      Serial.println(tamanho_corte[tipo]);
      EMPURRA_MADEIRA(tamanho_corte[tipo]);
      if(true){
        CORTE_AVANCO();
        if(true){
          CORTE_RETORNO();
        }
      }
      delay(500);
    }
    
    RETORNO();
  }
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}