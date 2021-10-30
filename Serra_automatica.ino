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
                 
==> ENTRADAS - (4 ENTRADAS)
    - A1 - RETORNO ALIMENTAÇÃO -> 1 ENTRADA DIGITAL
    - A2 - AVANÇO SERRA        -> 1 ENTRADA DIGITAL
    - A3 - RETORNO SERRA       -> 1 ENTRADA DIGITAL
    - A4 - PUXADOR             -> 1 ENTRADA DIGITAL
                     
==> SAÍDAS - (8 SAÍDAS)
    - M1 - MOTOR DE PASSO      -> 3 SAÍDAS DIGITAIS
    - M2 - MOTORES SERRA       -> 1 SAÍDA DIGITAL
    - P1 - PRENSOR ENTRADA     -> 1 SAÍDA DIGITAL
    - P2 - CARRINHO SERRA      -> 1 SAÍDA DIGITAL
    - P3 - PRENSOR SAÍDA       -> 1 SAÍDA DIGITAL
    - P4 - PUXADOR             -> 1 SAÍDA DIGITAL

==> FUNÇÕES //TODO adicionar funções
- = - = - = - = - = - = - OPS010  - = - = - = - = - = - = -

==> PORTAS
    - 08 ENTRADAS DIGITAIS 24VDC OPTOACOPLADAS
    - 08 SAÍDAS DIGITAIS 24VDC TRANSISTORIZADAS
    - 01 ENTRADA ANALÓGICA 0-10V
    - 01 SAÍDA ANALÓGICA 0-10V
    
==> MAPEAMENTO PINOS  // TODO cooncluir tabela
ESP32 - - - - - - OPS010 
 13
 12
 27
 26  

                
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
