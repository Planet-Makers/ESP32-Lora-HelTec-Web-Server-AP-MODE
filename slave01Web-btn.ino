//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32 Lora HelTec Slave (1)
// Informações mais detalhadas sobre este projeto, consulte o código do programa Master.

//---------------------------------------- Incluir biblioteca.
#include <SPI.h>
#include <LoRa.h>
//#include "images.h" //contém o logo para usarmos ao iniciar o display
#include "DHT.h"
//---------------------------------------- 

//---------------------------------------- Define o pino DHT11 e o tipo DHT.
#define DHTPIN      17
#define DHTTYPE     DHT11
//---------------------------------------- 

//---------------------------------------- Define pinos de LED.
#define LED_1_Pin   27
#define LED_2_Pin   25
//---------------------------------------- 

//---------------------------------------- Define pinos de botão.
int btn1 = 36;
int btn2 = 37;
//---------------------------------------- 

//---------------------------------------- Configuração LoRa Pin/GPIO.
#define ss 18
#define rst 14
#define dio0 26
//----------------------------------------

// Inicializa o sensor DHT (DHT11).
DHT dht11(DHTPIN, DHTTYPE);

//---------------------------------------- Variável de string para LoRa
String Incoming = "";
String Message = "";
String LED_Num = "";
String LED_State = "";
//----------------------------------------

//---------------------------------------- Configuração de transmissão de dados LoRa.
////////////////////////////////////////////////////////////////////////////////
// POR FAVOR, DESCOMENTE E SELECIONE UMA DAS VARIÁVEIS "LocalAddress" ABAIXO. //
////////////////////////////////////////////////////////////////////////////////

byte LocalAddress = 0x02;       //--> endereço deste dispositivo (Slave 1).
//byte LocalAddress = 0x03;       //--> endereço deste dispositivo (Slave 2).
//byte LocalAddress = 0x04;       //--> endereço deste dispositivo (Slave 3).
byte Destination_Master = 0x01; //--> destination to send to Master (ESP32).
//----------------------------------------

//---------------------------------------- Declarações de variáveis ​​para status de leitura do sensor DHT11, valores de temperatura e umidade.
int Humd = 0;
float Temp = 0.00;
String send_Status_Read_DHT11 = "";
//---------------------------------------- 

//---------------------------------------- Declaração de variável para Millis/Timer.
unsigned long previousMillis_UpdateDHT11 = 0;
const long interval_UpdateDHT11 = 2000;

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//---------------------------------------- 

// Declaração de variável como contador para reiniciar Lora HelTec.
byte Count_to_Rst_LORA = 0;

//________________________________________________________________________________ Subrotinas para envio de dados (LoRa HelTec).
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> iniciar pacote
  LoRa.write(Destination);        //--> adicionar endereço de destino
  LoRa.write(LocalAddress);       //--> adicionar endereço do remetente
  LoRa.write(Outgoing.length());  //--> adicionar comprimento de carga útil
  LoRa.print(Outgoing);           //--> adicionar carga útil
  LoRa.endPacket();               //--> termine o pacote e envie-o
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subrotinas para recebimento de dados (LoRa HelTec).
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // se não houver pacote, retorne

  //---------------------------------------- ler bytes do cabeçalho do pacote:
  int recipient = LoRa.read();        //--> Endereço do destinatário
  byte sender = LoRa.read();          //--> Endereço do remetente
  byte incomingLength = LoRa.read();  //--> comprimento da mensagem recebida
  byte master_Send_Mode = LoRa.read();
  //---------------------------------------- 

  //---------------------------------------- Condição que é executada se a mensagem não for de Master.
  if (sender != Destination_Master) {
    Serial.println();
    Serial.println("i"); //--> "i" = Não é do Master, Ignore.
    //Serial.println("Não é do Master, Ignore");

    // Redefine o valor da variável Count_to_Rst_LORA.
    Count_to_Rst_LORA = 0;
    return; //--> pular o resto da função
  }
  //---------------------------------------- 

  // Limpa dados variáveis ​​de entrada.
  Incoming = "";

  //---------------------------------------- Obtenha todos os dados recebidos.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  // Redefine o valor da variável Count_to_Rst_LORA.
  Count_to_Rst_LORA = 0;

  //---------------------------------------- Verifique o comprimento quanto a erros.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("er"); //--> "er" = error: o comprimento da mensagem não corresponde ao comprimento.
    //Serial.println("error: o comprimento da mensagem não corresponde ao comprimento");
    return; //--> pular o resto da função
  }
  //---------------------------------------- 

  //---------------------------------------- Verifica se os dados ou mensagens recebidos para este dispositivo.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("!");  //--> "!" = Esta mensagem não é para mim.
    //Serial.println("Ta mensagem dele não é para mim.");
    return; //--> pular o resto da função
  } else {
    // se a mensagem for para este dispositivo ou transmissão, imprima os detalhes:
    Serial.println();
    Serial.println("Rc from: 0x" + String(sender, HEX));
    Serial.println("Message: " + Incoming);

    // Chama a sub-rotina Processing_incoming_data().
    if (master_Send_Mode == 1) Processing_incoming_data();
    
    // Chama a sub-rotina Processing_incoming_data_for_Ctrl_LEDs().
    if (master_Send_Mode == 2) Processing_incoming_data_for_Ctrl_LEDs();
  }
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subrotina para processar os dados a serem enviados, após isso envia uma mensagem para oMaster.
void Processing_incoming_data() {
  // Obtenha o último estado dos LEDs.
  byte LED_1_State = digitalRead(LED_1_Pin);
  byte LED_2_State = digitalRead(LED_2_Pin);

  // Preencha a variável “Mensagem” com o estado de leitura do sensor DHT11, valor de umidade, valor de temperatura, estado do LED 1 e LED 2.
  Message = "";
  Message = send_Status_Read_DHT11 + "," + String(Humd) + "," + String(Temp) + "," + String(LED_1_State) + "," + String(LED_2_State);
  
  Serial.println();
  Serial.println("Tr to  : 0x" + String(Destination_Master, HEX));
  Serial.println("Message: " + Message);

  // Envie uma mensagem para Master.
  sendMessage(Message, Destination_Master);
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subrotinas para processamento de dados de mensagens recebidas para controle dos LEDs.
void Processing_incoming_data_for_Ctrl_LEDs() {

//  - Os dados recebidos do Mestre quando o modo de controle de LED é: "LED_number,LED_state".
//  - Por exemplo, o Mestre deseja acender o LED 1. Então o conteúdo da mensagem recebida é: "1,t"
//    > "1" = Número de LED controlado.
//    > "t" = Estado do LED. "t" para ligar e "f" para desligar.

  LED_Num = GetValue(Incoming, ',', 0);
  LED_State = GetValue(Incoming, ',', 1);

  // Se o Button for Low
  if (LED_Num == "1") {
    if (LED_State == "t") digitalWrite(LED_1_Pin, HIGH);
    if (LED_State == "f") digitalWrite(LED_1_Pin, LOW);
  }

  if (LED_Num == "2") {
    if (LED_State == "t") digitalWrite(LED_2_Pin, HIGH);
    if (LED_State == "f") digitalWrite(LED_2_Pin, LOW);
  }
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Função string para processar os dados recebidos
String GetValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Sub-rotina para redefinir Lora HelTec.
void Rst_LORA() {
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println(F("Reiniciando LoRa..."));
  Serial.println(F("Iniciando LoRa ..."));
  if (!LoRa.begin(915E6)) {             // taxa de inicialização em 915 or 433 MHz
    Serial.println(F("Falha na inicialização do LoRa. Cheque sua conexão."));
    while (true);                       // se falhar, não faça nada
  }
  Serial.println(F("LoRa iniciado com sucesso."));

  // Redefine o valor da variável Count_to_Rst_LORA.
  Count_to_Rst_LORA = 0;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP
void setup() {
  // coloque seu código de configuração aqui, para executar uma vez:

  Serial.begin(115200);

  pinMode(LED_1_Pin, OUTPUT);
  pinMode(LED_2_Pin, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);

  // Chama a sub-rotina Rst_LORA().
  Rst_LORA();
  
  dht11.begin();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
void loop() {
  // coloque seu código principal aqui, para rodar repetidamente:

  //---------------------------------------- Acionamento do Button
  if (digitalRead(btn1)==HIGH){
    digitalWrite(LED_1_Pin, HIGH);
  }else{
    digitalWrite(LED_1_Pin, LOW);
  }
  //----------------------------------------
  //---------------------------------------- Millis / Temporizador para atualização dos valores de temperatura e umidade do sensor DHT11 a cada 2 segundos (ver variável interval_UpdateDHT11).
  unsigned long currentMillis_UpdateDHT11 = millis();
  
  if (currentMillis_UpdateDHT11 - previousMillis_UpdateDHT11 >= interval_UpdateDHT11) {
    previousMillis_UpdateDHT11 = currentMillis_UpdateDHT11;

    // A leitura da temperatura ou umidade leva cerca de 250 milissegundos!
    // As leituras do sensor também podem ter até 2 segundos de idade (é um sensor muito lento)
    Humd = dht11.readHumidity();
    // Leia a temperatura como Celsius (o padrão)
    Temp = dht11.readTemperature();
    // Leia a temperatura como Fahrenheit (isFahrenheit = true)
    // float f = dht.readTemperature(true);
  
    // Verifique se alguma leitura falhou e saia mais cedo (para tentar novamente).
    if (isnan(Humd) || isnan(Temp)) {
      Humd = 0;
      Temp = 0.00;
      send_Status_Read_DHT11 = "f";
      Serial.println(F("Falha na leitura do sensor DHT!"));
      return;
    } else {
      send_Status_Read_DHT11 = "s";
    }
  }
  //---------------------------------------- 

  //---------------------------------------- Millis/Timer reiniciar Lora HelTec.
  // Por favor, consulte o código do programa Master para obter informações mais detalhadas sobre o método de redefinição Lora.
  
  unsigned long currentMillis_RestartLORA = millis();
  
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    Count_to_Rst_LORA++;
    if (Count_to_Rst_LORA > 30) {
      LoRa.end();
      Rst_LORA();
    }
  }
  //---------------------------------------- 

  //---------------------------------------- analise um pacote e chame onReceive com o resultado:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
