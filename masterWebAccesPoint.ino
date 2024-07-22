//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32 Lora HelTec Master Web Server AP MODE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  --------------------------------------                                                                                                            //
//  Uma breve descrição de como este projeto funciona.                                                                                                //
//                                                                                                                                                    //
//  Existem 2 tipos/modos de mensagens enviadas pelo Master:                                                                                          //
//  1. "get_Data_Mode".                                                                                                                               //
//    > Neste modo o Mestre envia uma mensagem contendo o comando para os Escravos enviarem o estado de leitura do sensor DHT11, valor de umidade,    //
//      valor da temperatura, estado do LED 1 e LED 2.                                                                                                //
//    > Neste modo o Mestre envia mensagens para o Escravo 1 e Escravo 2 alternadamente a cada 1 segundo.                                             //
//      Isso é feito para que não haja colisão entre as mensagens recebidas do Escravo 1 e do Escravo 2.                                              //
//    > A mensagem enviada contém:                                                                                                                    //
//      "destination_address | sender_address | message_length | mode | message_content".                                                             //
//      destination_address     = endereço do Slave.                                                                                                  //
//      sender_address          = Endereço do Master(GatWay).                                                                                         //
//      message_content_length  = número total de"characteres" na mensagem enviada (destination_address, sender_address e modo não incluído).         //
//                                Neste modo esta seção é empty/NULL.                                                                                 //
//      mode                    = contém valores 1 e 2. Valor 1 para "get_Data_Mode".                                                                 //
//      message_content         = conteúdo da mensagem na forma de caracteres (String). Neste modo esta seção é empty/NULL.                           //
//                                                                                                                                                    //
//      Por exemplo, envie uma mensagem para Slave 1:                                                                                                 //
//      "0x02 | 0x01 | NULL | 1 | NULL"                                                                                                               //
//      0x02  = endereço do Slave 1.                                                                                                                  //
//      0x01  = endereço Master.                                                                                                                      //
//      1     = "get_Data_Mode". Notifica o Slave pretendido (por exemplo Slave 1) para enviar uma mensagem de resposta contendo o valor de umidade,  //
//              valor de temperatura, estado do Sensor 1 e Sensor 2.                                                                                  //
//    > Após a mensagem ser recebida pelo Escravo pretendido (Escravo 1 ou Escravo 2). Então o escravo enviará uma mensagem de resposta contendo:     //
//      "destination_address | sender_address | message_content | message_content".                                                                   //
//      destination_address     = endereço Master(GatWay).                                                                                            //
//      sender_address          = enderço dos Slave 1 ou Slave 2.                                                                                     //
//      message_content_length  = número total de "caracteres" na mensagem enviada (destination_address , sender_address e modo não incluído).        //
//      message_content         = conteúdo da mensagem na forma de caracteres (String).                                                               //
//                                                                                                                                                    //
//      Por examplo, Slave 1 envia uma mensagem de resposta para Master(GatWay):                                                                      //
//      "0x01 | 0x02 | 14 | s,80,30,50,0,1"                                                                                                           //
//      0x01            = endereço Master.                                                                                                            //
//      0x02            = enderço do Slave 1.                                                                                                         //
//      14              = número total de "caracteres" na mensagem enviada. O conteúdo da mensagem enviada é: "s,80,30.50,0,1" ,                      //
//                        o número total de caracteres é de 14 caracteres.                                                                            //
//      s,80,30.50,0,1  = data.                                                                                                                       //
//                        s     = DHT11 status de leitura do sensor, "s" se for bem-sucedido e "f" se falhar.                                         //
//                        80    = valor de umidade.                                                                                                   //
//                        30.50 = valor da temperatura.                                                                                               //
//                        0     = Sensor 1 estado (0 é off, 1 é on).                                                                                  //
//                        1     = Sensor 2 estado (0 é off, 1 é on).                                                                                  //
//                                                                                                                                                    //
//  2. "led_Control_Mode".                                                                                                                            //
//    > Neste modo o Master envia uma mensagem contendo o comando para controlar o Sensor 1 e Sensor 2 nos Slaves.                                    //
//    > Neste modo o Mestre envia uma mensagem para o Escravo 1 ou Escravo 2 cada vez que o Botão para controlar o LED na página do servidor web      //
//      é pressionado/clicado.                                                                                                                        //
//    > A mensagem enviada contém:                                                                                                                    //
//    "destination_address | sender_address | message_length | mode | message_content".                                                               //
//    destination_address     = endereço do Slave 1 ou Slave 2.                                                                                       //
//    sender_address          = endereço Master(GateWay).                                                                                             //
//    message_content_length  = número total de "caracteres" na mensagem enviada (destination_address , sender_address e modo não incluídos).         //
//    mode                    = contém valores 1 e 2. Valor 2 para "led_Control_Mode".                                                                //
//    message_content         = conteúdo da mensagem na forma de caracteres (String).                                                                 //
//                                                                                                                                                    //
//    Por exemplo, envie uma mensagem para Slave 1:                                                                                                   //
//    "0x02 | 0x01 | 3 | 2 | 1,t"                                                                                                                     //
//    0x02  = endereço do Slave 1.                                                                                                                    //
//    0x01  = endereço do Master(GateWay).                                                                                                            //
//    3     = número total de "caracteres" na mensagem enviada. O conteúdo da mensagem enviada é: "1,1" ,                                             //
//            o número total de caracteres é de 3 caracteres.                                                                                         //
//    2     = "led_Control_Mode". Notifica o Slave pretendido (por exemplo Slave 1) que uma mensagem foi enviada para controlar o Sensor.             //
//    1,t   = data.                                                                                                                                   //
//            1 = Número de Sensor controlado (1 para "Sensor 1" e 2 para "Sensor 2").                                                                //
//            t = Define o estado do Sensor ("t" para ligar e "f" para desligar).                                                                     //
//  --------------------------------------                                                                                                            //
//                                                                                                                                                    //
//  --------------------------------------                                                                                                            //
//  Informações adicionais :                                                                                                                          //
//                                                                                                                                                    //
//  Se você tem certeza de que conectou o Lora HelTec corretamente ao ESP32,                                                                          //
// mas você recebe a mensagem de erro "Falha na inicialização do LoRa! Verifique suas conexões" e tente usar uma fonte de alimentação externa.        //
// Neste projeto utilizei uma fonte de 3,3V de uma placa ESP32 para alimentar o Lora HelTec.                                                          //
// Porque talvez a qualidade do módulo da placa ESP32 seja diferente do mercado e a fonte de alimentação não seja forte o suficiente para alimentação //
// Então você recebe a mensagem de erro "LoRa init failed ! Verifique suas conexões".                                                                 //
//  --------------------------------------                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------- Include Library.

#include <SPI.h>  //responsável pela comunicação serial
#include <LoRa.h> //responsável pela comunicação com o WIFI Lora
#include <Wire.h> //responsável pela comunicação i2c
//#include "images.h" //contém o logo para usarmos ao iniciar o display
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
//---------------------------------------- 

#include "PageIndex.h" 

//---------------------------------------- LoRa Pin / GPIO configuração.
#define ss 18
#define rst 14
#define dio0 26
//----------------------------------------

//---------------------------------------- Access Point Declaração e Configuração.
const char* ssid = "PLMKRS_LR";  //--> nome do access point
const char* password = "LoRaesp32HT"; //--> senha access point

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
//----------------------------------------

//---------------------------------------- Declaração de variável para armazenar dados de entrada e saída.
String Incoming = "";
String Message = ""; 
//----------------------------------------

//---------------------------------------- Configuração de transmissão de dados LoRa.
byte LocalAddress = 0x01;               //--> endereço deste dispositivo (endereço Master(GateWay).
byte Destination_ESP32_Slave_1 = 0x02;  //--> destino para enviar ao Slave 1 (ESP32).
byte Destination_ESP32_Slave_2 = 0x03;  //--> destino para enviar ao Slave 2 (ESP32).
byte Destination_ESP32_Slave_3 = 0x04;  //--> destino para enviar ao Slave 3 (ESP32).
byte Destination_ESP32_Slave_4 = 0x05;  //--> destino para enviar ao Slave 4 (ESP32).
byte Destination_ESP32_Slave_5 = 0x06;  //--> destino para enviar ao Slave 5 (ESP32).
const byte get_Data_Mode = 1;           //--> Modo para obter o status de leitura do sensor DHT11, valor de umidade, valor de temperatura, estado do Sensor 1 e Sensor 2.
const byte led_Control_Mode = 2;        //--> Modo para controlar o Sensor 1 e o Sensor 2 no Slave.
//---------------------------------------- 

//---------------------------------------- Declaração de variável para Millis/Timer.
unsigned long previousMillis_SendMSG_to_GetData = 0;
const long interval_SendMSG_to_GetData = 1000;

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//---------------------------------------- 

//---------------------------------------- Variáveis ​​para acomodar o status de leitura do sensor DHT11, valor de umidade, valor de temperatura, estado do Sensor 1 e Sensor 2.
int Humd[2];
float Temp[2];
String LED_1_State_str = "";
String LED_2_State_str = "";
String receive_Status_Read_DHT11 = "";
bool LED_1_State_bool = false;
bool LED_2_State_bool = false;
//---------------------------------------- 

//---------------------------------------- As variáveis ​​usadas para verificar os parâmetros passados ​​na URL.
// Look in the "PageIndex.h" file.
// xhr.open("GET", "set_LED?Slave_Num="+slave+"&LED_Num="+led_num+"&LED_Val="+value, true);
// For example :
// set_LED?Slave_Num=S1&LED_Num=1&LED_Val=1
// PARAM_INPUT_1 = S1
// PARAM_INPUT_2 = 1
// PARAM_INPUT_3 = 1
const char* PARAM_INPUT_1 = "Slave_Num";
const char* PARAM_INPUT_2 = "LED_Num";
const char* PARAM_INPUT_3 = "LED_Val";
//---------------------------------------- 

//---------------------------------------- Declaração de variável para armazenar dados do servidor web para controlar o Sensor.
String Slave_Number = "";
String LED_Number = "";
String LED_Value = "";
//---------------------------------------- 

// Declaração de variável para contar slaves.
byte Slv = 0;

// Declaração de variável para obter o endereço dos Slaves.
byte slave_Address;

// Declaração de variável como contador para reiniciar Lora HelTec.
byte count_to_Rst_LORA = 0;

// Declaração de variável para avisar que o processo de recebimento da mensagem foi concluído.
bool finished_Receiving_Message = false;

// Declaração de variável para avisar que o processo de envio da mensagem foi finalizado.
bool finished_Sending_Message = false;

// Declaração de variável para iniciar o envio de mensagens aos Slaves para controlar os Sensores.
bool send_Control_LED = false;

// Inicializar JSONVar
JSONVar JSON_All_Data_Received;

// Crie o objeto AsyncWebServer na porta 80
AsyncWebServer server(80);

// Crie uma fonte de eventos em /events
AsyncEventSource events("/events");

//________________________________________________________________________________ Subrotinas para envio de dados (LoRa HelTec).
void sendMessage(String Outgoing, byte Destination, byte SendMode) { 
  finished_Sending_Message = false;

  Serial.println();
  Serial.println("Tr to  : 0x" + String(Destination, HEX));
  Serial.print("Mode   : ");
  if (SendMode == 1) Serial.println("Getting Data");
  if (SendMode == 2) Serial.println("Controlling LED.");
  Serial.println("Message: " + Outgoing);
  
  LoRa.beginPacket();             //--> iniciar pacote
  LoRa.write(Destination);        //--> adicionar endereço de destino
  LoRa.write(LocalAddress);       //--> adicionar endereço do remetente
  LoRa.write(Outgoing.length());  //--> adicionar comprimento de carga útil
  LoRa.write(SendMode);           //--> 
  LoRa.print(Outgoing);           //--> adicionar carga útil
  LoRa.endPacket();               //--> termine o pacote e envie-o
  
  finished_Sending_Message = true;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subrotinas para recebimento de dados (LoRa Ra-02).
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // se não houver pacote, retorne

  finished_Receiving_Message = false;

  //---------------------------------------- ler bytes do cabeçalho do pacote:
  int recipient = LoRa.read();        //--> Endereço do destinatário
  byte sender = LoRa.read();          //--> Endereço do remetente
  byte incomingLength = LoRa.read();  //--> comprimento da mensagem recebida
  //---------------------------------------- 

  // Limpa dados variáveis ​​de entrada.
  Incoming = "";

  //---------------------------------------- Obtenha todos os dados/mensagens recebidas.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  // Redefine o valor da variável count_to_Rst_LORA se uma mensagem for recebida.
  count_to_Rst_LORA = 0;

  //---------------------------------------- Verifique o comprimento quanto a erros.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("er"); //--> "er" = error: o comprimento da mensagem não corresponde ao comprimento.
    //Serial.println("error: o comprimento da mensagem não corresponde ao comprimento");
    finished_Receiving_Message = true;
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //---------------------------------------- Verifica se os dados ou mensagens recebidos para este dispositivo.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("!");  //--> "!" = Esta mensagem não é para mim.
    //Serial.println("Esta mensagem não é para mim.");
    finished_Receiving_Message = true;
    return; //--> pular o resto da função
  }
  //---------------------------------------- 

  //----------------------------------------  se a mensagem for para este dispositivo ou transmissão, imprima os detalhes:
  Serial.println();
  Serial.println("Rc from: 0x" + String(sender, HEX));
  Serial.println("Message: " + Incoming);
  //---------------------------------------- 

  // Obtenha o endereço dos remetentes ou escravos.
  slave_Address = sender;

  // Chama a sub-rotina Processing_incoming_data().
  Processing_incoming_data();

  finished_Receiving_Message = true;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subrotinas para processar dados de mensagens recebidas.
void Processing_incoming_data() {
  
//  Exemplos do conteúdo das mensagens recebidas de escravos são os seguintes: "s,80,30.50,1,0" , 
//  para separá-los com base no caractere vírgula, utiliza-se a sub-rotina "GetValue" e a ordem é a seguinte:
//  GetValue(Incoming, ',', 0) = s
//  GetValue(Incoming, ',', 1) = 80
//  GetValue(Incoming, ',', 2) = 30.50
//  GetValue(Incoming, ',', 3) = 1
//  GetValue(Incoming, ',', 4) = 0

  //---------------------------------------- Condições para processamento de dados ou mensagens de Slave 1 (ESP32 Slave 1).
  if (slave_Address == Destination_ESP32_Slave_1) {
    receive_Status_Read_DHT11 = GetValue(Incoming, ',', 0);
    if (receive_Status_Read_DHT11 == "f") receive_Status_Read_DHT11 = "FAILED";
    if (receive_Status_Read_DHT11 == "s") receive_Status_Read_DHT11 = "SUCCEED";
    Humd[0] = GetValue(Incoming, ',', 1).toInt();
    Temp[0] = GetValue(Incoming, ',', 2).toFloat();
    LED_1_State_str = GetValue(Incoming, ',', 3);
    LED_2_State_str = GetValue(Incoming, ',', 4);
    if (LED_1_State_str == "1" || LED_1_State_str == "0") {
      LED_1_State_bool = LED_1_State_str.toInt();
    }
    if (LED_2_State_str == "1" || LED_2_State_str == "0") {
      LED_2_State_bool = LED_2_State_str.toInt();
    }

    // Chama a sub-rotina Send_Data_to_WS().
    Send_Data_to_WS("S1", 1);
  }
  //---------------------------------------- 

  //---------------------------------------- Condições para processamento de dados ou mensagens do (ESP32 Slave 2).
  if (slave_Address == Destination_ESP32_Slave_2) {
    receive_Status_Read_DHT11 = GetValue(Incoming, ',', 0);
    if (receive_Status_Read_DHT11 == "f") receive_Status_Read_DHT11 = "FAILED";
    if (receive_Status_Read_DHT11 == "s") receive_Status_Read_DHT11 = "SUCCEED";
    Humd[1] = GetValue(Incoming, ',', 1).toInt();
    Temp[1] = GetValue(Incoming, ',', 2).toFloat();
    LED_1_State_str = GetValue(Incoming, ',', 3);
    LED_2_State_str = GetValue(Incoming, ',', 4);
    if (LED_1_State_str == "1" || LED_1_State_str == "0") {
      LED_1_State_bool = LED_1_State_str.toInt();
    }
    if (LED_2_State_str == "1" || LED_2_State_str == "0") {
      LED_2_State_bool = LED_2_State_str.toInt();
    }

    // Chama a sub-rotina Send_Data_to_WS().
    Send_Data_to_WS("S2", 2);
  }
  //---------------------------------------- 

  //---------------------------------------- Condições para processamento de dados ou mensagens de Slave 3 (ESP32 Slave 3).
  if (slave_Address == Destination_ESP32_Slave_3) {
    receive_Status_Read_DHT11 = GetValue(Incoming, ',', 0);
    if (receive_Status_Read_DHT11 == "f") receive_Status_Read_DHT11 = "FAILED";
    if (receive_Status_Read_DHT11 == "s") receive_Status_Read_DHT11 = "SUCCEED";
    Humd[1] = GetValue(Incoming, ',', 1).toInt();
    Temp[1] = GetValue(Incoming, ',', 2).toFloat();
    LED_1_State_str = GetValue(Incoming, ',', 3);
    LED_2_State_str = GetValue(Incoming, ',', 4);
    if (LED_1_State_str == "1" || LED_1_State_str == "0") {
      LED_1_State_bool = LED_1_State_str.toInt();
    }
    if (LED_2_State_str == "1" || LED_2_State_str == "0") {
      LED_2_State_bool = LED_2_State_str.toInt();
    }

    // Chama a sub-rotina Send_Data_to_WS().
    Send_Data_to_WS("S3", 3);
  }
  //----------------------------------------

  //---------------------------------------- Condições para processamento de dados ou mensagens de Slave 4 (ESP32 Slave 4).
  if (slave_Address == Destination_ESP32_Slave_4) {
    receive_Status_Read_DHT11 = GetValue(Incoming, ',', 0);
    if (receive_Status_Read_DHT11 == "f") receive_Status_Read_DHT11 = "FAILED";
    if (receive_Status_Read_DHT11 == "s") receive_Status_Read_DHT11 = "SUCCEED";
    Humd[0] = GetValue(Incoming, ',', 1).toInt();
    Temp[0] = GetValue(Incoming, ',', 2).toFloat();
    LED_1_State_str = GetValue(Incoming, ',', 3);
    LED_2_State_str = GetValue(Incoming, ',', 4);
    if (LED_1_State_str == "1" || LED_1_State_str == "0") {
      LED_1_State_bool = LED_1_State_str.toInt();
    }
    if (LED_2_State_str == "1" || LED_2_State_str == "0") {
      LED_2_State_bool = LED_2_State_str.toInt();
    }

    // Chama a sub-rotina Send_Data_to_WS().
    Send_Data_to_WS("S4", 4);
  }
  //---------------------------------------- 

  //---------------------------------------- Condições para processamento de dados ou mensagens de Slave 5 (ESP32 Slave 5).
  if (slave_Address == Destination_ESP32_Slave_5) {
    receive_Status_Read_DHT11 = GetValue(Incoming, ',', 0);
    if (receive_Status_Read_DHT11 == "f") receive_Status_Read_DHT11 = "FAILED";
    if (receive_Status_Read_DHT11 == "s") receive_Status_Read_DHT11 = "SUCCEED";
    Humd[1] = GetValue(Incoming, ',', 1).toInt();
    Temp[1] = GetValue(Incoming, ',', 2).toFloat();
    LED_1_State_str = GetValue(Incoming, ',', 3);
    LED_2_State_str = GetValue(Incoming, ',', 4);
    if (LED_1_State_str == "1" || LED_1_State_str == "0") {
      LED_1_State_bool = LED_1_State_str.toInt();
    }
    if (LED_2_State_str == "1" || LED_2_State_str == "0") {
      LED_2_State_bool = LED_2_State_str.toInt();
    }

    // Chama a sub-rotina Send_Data_to_WS().
    Send_Data_to_WS("S5", 5);
  }
  //---------------------------------------- 
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Sub-rotina para enviar dados recebidos dos Slaves ao servidor web para serem exibidos.
void Send_Data_to_WS(char ID_Slave[5], byte Slave) {
  //:::::::::::::::::: Insira os dados recebidos em JSONVar(JSON_All_Data_Received).
  JSON_All_Data_Received["ID_Slave"] = ID_Slave;
  JSON_All_Data_Received["StatusReadDHT11"] = receive_Status_Read_DHT11;
  JSON_All_Data_Received["Humd"] = Humd[Slave-1];
  JSON_All_Data_Received["Temp"] = Temp[Slave-1];
  JSON_All_Data_Received["LED1"] = LED_1_State_bool;
  JSON_All_Data_Received["LED2"] = LED_2_State_bool; 
  //:::::::::::::::::: 
  
  //:::::::::::::::::: Crie uma string JSON para armazenar todos os dados recebidos do remetente.
  String jsonString_Send_All_Data_received = JSON.stringify(JSON_All_Data_Received);
  //:::::::::::::::::: 
  
  //:::::::::::::::::: Envia todos os dados recebidos do remetente para o navegador como um evento ('allDataJSON').
  events.send(jsonString_Send_All_Data_received.c_str(), "allDataJSON", millis());
  //::::::::::::::::::  
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
  Serial.println("Reinicia o LoRa...");
  Serial.println("Inicializa LoRa...");
  if (!LoRa.begin(915E6)) {             // taxa de inicialização em 915 ou 433 MHz
    Serial.println("Falha na inicialização do LoRa. Cheque suas conexões.");
    while (true);                       // se falhar, não faça nada
  }
  Serial.println("LoRa inicializado com sucesso.");

  // Redefina o valor da variável count_to_Rst_LORA.
  count_to_Rst_LORA = 0;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP
void setup() {
  // coloque seu código de configuração aqui, para executar uma vez:

  Serial.begin(115200);

  //---------------------------------------- Limpa os valores das variáveis ​​de matriz Temp e Humd pela primeira vez.
  for (byte i = 0; i < 2; i++) {
    Humd[i] = 0;
    Temp[i] = 0.00;
  }
  //---------------------------------------- 

 //---------------------------------------- Definir Wi-Fi para AP mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : AP");
  WiFi.mode(WIFI_AP);
  Serial.println("-------------");
  //---------------------------------------- 

  delay(100);

  //---------------------------------------- Configurando o ESP32 para ser um Access Point.
  Serial.println();
  Serial.println("-------------");
  Serial.println("Configurando o ESP32 para ser um Access Point.");
  WiFi.softAP(ssid, password); //--> Criando pontos de acesso
  delay(1000);
  Serial.println("Configurando ESP32 softAPConfig.");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("-------------");
  //----------------------------------------

  delay(500);

  //---------------------------------------- Lidar com servidor web
  Serial.println();
  Serial.println("Configurando a página principal no Servidor.");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", MAIN_page);
  });
  //---------------------------------------- 

  //---------------------------------------- Lidar com eventos do servidor Web
  Serial.println();
  Serial.println("Configurando fontes de eventos no servidor.");
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Cliente reconectado! O último ID da mensagem obtido é: %u\n", client->lastId());
    }
    // enviar evento com mensagem "hello!", id current millis
    // e definindo o atraso de reconexão para 10 segundos
    client->send("hello!", NULL, millis(), 10000);
  });
  //---------------------------------------- 

  //---------------------------------------- Envie uma solicitação GET para <ESP_IP>/set_LED?Slave_Num=<inputMessage1>&LED_Num=<inputMessage2>&LED_Val=<inputMessage3>
  server.on("/set_LED", HTTP_GET, [] (AsyncWebServerRequest *request) {
    //:::::::::::::::::: 
    // OBTER valor de entrada ativado <ESP_IP>/set_LED?Slave_Num=<inputMessage1>&LED_Num=<inputMessage2>&LED_Val=<inputMessage3>
    // PARAM_INPUT_1 = inputMessage1
    // PARAM_INPUT_2 = inputMessage2
    // PARAM_INPUT_3 = inputMessage3
    // Slave_Number = PARAM_INPUT_1
    // LED_Number = PARAM_INPUT_2
    // LED_Value = PARAM_INPUT_3
    //:::::::::::::::::: 
    
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2) && request->hasParam(PARAM_INPUT_3)) {
      Slave_Number = request->getParam(PARAM_INPUT_1)->value();
      LED_Number = request->getParam(PARAM_INPUT_2)->value();
      LED_Value = request->getParam(PARAM_INPUT_3)->value();

      String Rslt = "Slave : " + Slave_Number + " || LED : " + LED_Number + " || Set to : " + LED_Value;
      Serial.println();
      Serial.println(Rslt);
      send_Control_LED = true;
    }
    else {
      send_Control_LED = false;
      Slave_Number = "Nenhuma mensagem enviada";
      LED_Number = "Nenhuma mensagem enviada";
      LED_Value = "Nenhuma mensagem enviada";
      String Rslt = "Slave : " + Slave_Number + " || LED : " + LED_Number + " || Set to : " + LED_Value;
      Serial.println();
      Serial.println(Rslt);
    }
    request->send(200, "text/plain", "OK");
  });
  //---------------------------------------- 

  //---------------------------------------- Adicionando fontes de eventos no servidor.
  Serial.println();
  Serial.println("Adicionando fontes de eventos no servidor.");
  server.addHandler(&events);
  //---------------------------------------- 

  //---------------------------------------- Iniciando o servidor.
  Serial.println();
  Serial.println("Iniciando o Server.");
  server.begin();
  //---------------------------------------- 

  // Chama a sub-rotina Rst_LORA().
  Rst_LORA();

  Serial.println();
  Serial.println("------------");
  Serial.print("SSID name : ");
  Serial.println(ssid);
  Serial.print("IP address : ");
  Serial.println(WiFi.softAPIP());
  Serial.println();
  Serial.println("Conecte seu computador ou Wifi móvel ao SSID acima.");
  Serial.println("Insira o endereço IP acima em seu navegador para abrir a página principal.");
  Serial.println("------------");
  Serial.println();

}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
void loop() {

  //---------------------------------------- Millis/Timer para enviar mensagens aos escravos a cada 1 segundo (ver variável interval_SendMSG_to_GetData).
  //  As mensagens são enviadas a cada segundo alternadamente.
  //  > Mestre envia mensagem para Escravo 1, atraso de 1 segundo.
  //  > Mestre envia mensagem para Escravo 2, atraso de 1 segundo.
  
  unsigned long currentMillis_SendMSG_to_GetData = millis();
  
  if (currentMillis_SendMSG_to_GetData - previousMillis_SendMSG_to_GetData >= interval_SendMSG_to_GetData) {
    previousMillis_SendMSG_to_GetData = currentMillis_SendMSG_to_GetData;

    Slv++;
    if (Slv > 5) Slv = 1;
    
    //:::::::::::::::::: Condição para envio de dados de mensagem/comando para Slave 1 (ESP32 Slave 1).
    if (Slv == 1) {
      Humd[0] = 0;
      Temp[0] = 0.00;
      sendMessage("", Destination_ESP32_Slave_1, get_Data_Mode);
    }
    //:::::::::::::::::: 
    
    //:::::::::::::::::: Condição para envio de mensagem/comando para Slave 2 (ESP32 Slave 2).
    if (Slv == 2) {
      Humd[1] = 0;
      Temp[1] = 0.00;
      sendMessage("", Destination_ESP32_Slave_2, get_Data_Mode);
    }
    //:::::::::::::::::: 
        //:::::::::::::::::: Condição para envio de dados de mensagem/comando para Slave 3 (ESP32 Slave 3).
    if (Slv == 3) {
      Humd[0] = 0;
      Temp[0] = 0.00;
      sendMessage("", Destination_ESP32_Slave_3, get_Data_Mode);
    }
    //::::::::::::::::::

    //:::::::::::::::::: Condição para envio de dados de mensagem/comando para Slave 4 (ESP32 Slave 4).
    if (Slv == 4) {
      Humd[0] = 0;
      Temp[0] = 0.00;
      sendMessage("", Destination_ESP32_Slave_4, get_Data_Mode);
    }
    //:::::::::::::::::: 
    
    //:::::::::::::::::: Condição para envio de dados de mensagem/comando ao Escravo 5 (ESP32 Escravo 5).
    if (Slv == 5) {
      Humd[1] = 0;
      Temp[1] = 0.00;
      sendMessage("", Destination_ESP32_Slave_5, get_Data_Mode);
    }
    //::::::::::::::::::
  }
  //---------------------------------------- 

  //---------------------------------------- 
  if (finished_Sending_Message == true && finished_Receiving_Message == true) {
    if (send_Control_LED == true) {
      delay(250);
      send_Control_LED = false;
      if (Slave_Number == "S1") {
        Message = "";
        Message = LED_Number + "," + LED_Value;
        sendMessage(Message, Destination_ESP32_Slave_1, led_Control_Mode);
      }
      if (Slave_Number == "S2") {
        Message = "";
        Message = LED_Number + "," + LED_Value;
        sendMessage(Message, Destination_ESP32_Slave_2, led_Control_Mode);
      }
      if (Slave_Number == "S3") {
        Message = "";
        Message = LED_Number + "," + LED_Value;
        sendMessage(Message, Destination_ESP32_Slave_3, led_Control_Mode);
      }
      if (Slave_Number == "S4") {
        Message = "";
        Message = LED_Number + "," + LED_Value;
        sendMessage(Message, Destination_ESP32_Slave_4, led_Control_Mode);
      }
      if (Slave_Number == "S5") {
        Message = "";
        Message = LED_Number + "," + LED_Value;
        sendMessage(Message, Destination_ESP32_Slave_5, led_Control_Mode);
      }
      delay(250);
    }
  }
  //---------------------------------------- 

  //---------------------------------------- Millis/Timer reiniciar Lora HelTec.
  //  - A redefinição do Lora HelTec é necessária para uso a longo prazo.
  //  - Isso significa que o Lora HelTec está ligado e funcionando há muito tempo.
  //  - Pela minha experiência ao usar Lora Ra-02 por muito tempo, há momentos em que Lora HelTec parece “congelar” ou ocorrer um erro, 
  //    portanto, não pode enviar e receber mensagens. Isso não acontece com frequência, mas acontece às vezes. 
  //    Então adicionei um método para redefinir Lora Ra-02 para resolver esse problema. Como resultado, o método teve sucesso na resolução do problema.
  //  - Este método de reinicialização do Lora HelTec funciona verificando se há mensagens recebidas, 
  //    se nenhuma mensagem for recebida por aproximadamente 30 segundos, considera-se que o Lora Ra-02 está apresentando "congelamento" ou erro, portanto, uma reinicialização é realizada.

  unsigned long currentMillis_RestartLORA = millis();
  
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    count_to_Rst_LORA++;
    if (count_to_Rst_LORA > 30) {
      LoRa.end();
      Rst_LORA();
    }
  }
  //----------------------------------------

  //----------------------------------------analise um pacote e chame onReceive com o resultado:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
