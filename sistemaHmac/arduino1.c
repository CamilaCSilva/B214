#define PACKAGE_SIZE1 11
#define PACKAGE_SIZE2 12
#define PIN_BUTTON 7 

//start, size, id, ack/nack, payload(5B), checksum/crc, end
uint8_t uiPackage1[PACKAGE_SIZE1] = {0xFE, 0x08, 0xAB, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x48, 0xFF}; // integro
uint8_t uiPackage2[PACKAGE_SIZE1] = {0xFE, 0x08, 0xB0, 0x01, 0x00, 0x01, 0x02, 0x05, 0x04, 0x48, 0xFF}; // nao integro
//start, size, id, ack/nack, payload(6B), checksum/crc, end
uint8_t uiPackage3[PACKAGE_SIZE2] = {0xFE, 0x09, 0xB1, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xD8, 0xFF}; // integro
uint8_t uiPackage4[PACKAGE_SIZE2] = {0xFE, 0x09, 0xB2, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x08, 0xD8, 0xFF}; // nao integro

uint8_t uiCount = 0; // conta quantos pacotes foram enviados


void setup(){
 pinMode(PIN_BUTTON,INPUT); 
 Serial.begin(9600);
 
}

void loop(){
  
  if(digitalRead(PIN_BUTTON) == LOW){
    //se nenhum pacote tiver sido enviado
    if(uiCount == 0){
      for(int k=0; k<PACKAGE_SIZE1; k++){
        Serial.write(uiPackage1[k]); // enviar o primeiro pacote
        delay(50);
        uiCount = 1; // uiCount indica que um pacote foi enviado
      }
    }
    //se um pacote tiver sido enviado
    else if(uiCount == 1) { 
      for(int k=0; k<PACKAGE_SIZE1; k++){
        Serial.write(uiPackage2[k]); // enviar o segundo pacote
        delay(50);
        uiCount = 2; // uiCount indica que dois pacote foram enviados
      }
    }
    //se dois pacotes tiverem sido enviados
    else if(uiCount == 2) { 
      for(int k=0; k<PACKAGE_SIZE2; k++){
        Serial.write(uiPackage3[k]);  // enviar o terceiro pacote
        delay(50);
        uiCount = 3; // uiCount indica que três pacote foram enviados
      }
    }
    //se 3 ou outra quantidade de pacotes tiver sido enviado
    else{
      for(int k=0; k<PACKAGE_SIZE2; k++){
        Serial.write(uiPackage4[k]); // enviar o quarto pacote
        delay(50);
        uiCount = 0; // uiCount é zerado
      }
    }
  }   
}

