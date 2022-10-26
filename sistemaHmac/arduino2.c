#define PAYLOAD_SIZE1 5
#define PAYLOAD_SIZE2 6

uint8_t uiBuffer[12] = {};
uint8_t uiCount = 0; //conta as posicoes do buffer 
uint8_t uiPayloadSize = PAYLOAD_SIZE1; 
uint8_t uiPackageSize = 11;
bool bIntegrity = true; //variavel de sinalizacao (flag)

void setup(){
	Serial.begin(9600); 
}
  
void loop(){
  
  if(Serial.available() > 0){
  	char cChar; //variavel auxiliar 
   	cChar = Serial.read();
    uiBuffer[uiCount] = (uint8_t)cChar; //casting da variavel cChar 
    Serial.println(uiBuffer[uiCount], HEX);
    analyzePackageId(uiBuffer); //função que analisa se os pacotes são os de 6B de payload ou os de 5B
    uiCount++;
  }
  
  if(uiCount >= uiPackageSize){
  	Serial.println("Fim do pacote");
    uiCount = 0;
    analyzePackage(uiBuffer);
  }
}

void analyzePackageId(uint8_t* uiBuffer){ 
  if(uiBuffer[2] == 0xB1 || uiBuffer[2] == 0xB2){//caso os pacotes tenham um desses ids
      uiPayloadSize = PAYLOAD_SIZE2; // muda-se o tamanho do payload e do pacote, já que o tamanho dos
      uiPackageSize = 12; // pacotes são diferentes
  }
  else {
    uiPayloadSize = PAYLOAD_SIZE1; //caso sejam outros pacotes, os tamanhos
    uiPackageSize = 11; //voltam ao que estavam antes
  }
}

void analyzePackage(uint8_t* uiBuffer) {
  uint8_t uiPayload[uiPayloadSize]; //não inicializa, pois o payload pode ter 5 ou 6 bytes
  memcpy(&uiPayload, &uiBuffer[4], uiPayloadSize); // copia bytes de um buffer para outro
  									   //(buf destinatario, buf remetente, size)
  									   // vai pegar 5/6 bytes a partir do byte 4
  
  // Calcula CRC do payload + CRC recebido
  uint8_t uiCrcResult = crcCalc(uiPayload, uiPayloadSize); 
  
  if(uiPayloadSize == PAYLOAD_SIZE1){ // se o payload tiver 5 bytes
    if(uiCrcResult != uiBuffer[9]){ 
       bIntegrity = false;
    }
    else {
        bIntegrity = true;
    }
   
    if(uiBuffer[0] != 0xFE || uiBuffer[10] != 0xFF){ 
      bIntegrity = false;
      Serial.println("Erro start/end");
    }
 }
  else if(uiPayloadSize == PAYLOAD_SIZE2){ // se o payload tiver 6 bytes
    if(uiCrcResult != uiBuffer[10]){
       bIntegrity = false;
    }
    else {
        bIntegrity = true;
    }
    
    if(uiBuffer[0] != 0xFE || uiBuffer[11] != 0xFF){ 
      bIntegrity = false;
      Serial.println("Erro start/end");
    } 
  }
 
  if(uiBuffer[1] != uiPackageSize-3){ //tamanho desconta 3B: start, end e size
      bIntegrity = false;
      Serial.println("Erro size");
  }
  
  uint8_t uiAckNack[5] = {0xFE, 0x02, 0x00, 0x01, 0xFF};  

  uiAckNack[2] = uiBuffer[2]; //cópia do id do pacote
  if(bIntegrity == true){
    Serial.println("ACK");
    Serial.println();
  }  
  else {
    Serial.println("NACK");
    uiAckNack[3] = 0x00; //representa o nack
    Serial.println();
  }
    
  for(int k=0; k<5; k++){
    Serial.write(uiAckNack[k]);
    delay(50);
    Serial.println();
  }
}

// Funcao CRC: https://github.com/pavan-pan/CRC8/blob/master/CRC8_BYTE_BYTE.c
uint8_t crcCalc(uint8_t *arr, uint8_t size) //provisionar as variaveis/ponteiros
{
    uint8_t crc = 0;
    const uint8_t generator = 0x28;//Polynomial

    for (int j = 0; j < size; j++)
    {
        crc ^= arr[j];

        //For each bit of the byte check for the MSB bit, 
		//if it is 1 then left shift the CRC and XOR with the 
      	// polynomial otherwise just left shift the variable
        for (int i = 0; i < 8; i++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (uint8_t)((crc << 1 ^ generator));
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    Serial.println(crc);
    return crc;
}
