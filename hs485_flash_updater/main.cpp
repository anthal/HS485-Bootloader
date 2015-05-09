/*
 * Hinweise zur Programmierung und zum Protokoll finden sie in der beiliegenden Dokumentation
 * Beachten Sie auch die beiliegenden Nutzungsbedingungen und den Haftungsausschluss
 *
 * Copyright (c) 2005 by ELV Elektronik AG,
 * Maiburger Str. 32-36 D-26789 Leer, Germany
 * All rights reserved.
 */

#include <cstdlib>
#include <iostream>
#include "ComPort.h"
#include <stdio.h>
#include <conio.h>
#include "HS485Demo_private.h"    // Enthält Versionsnummer etc.

using namespace std;

#define FRAME_START_LONG  0xFD
#define FRAME_START_SHORT 0xFE
#define ESCAPE_CHAR     0xFC

#define IS_IFRAME(x)        (((x) & 0x01) == 0x00)
#define IS_ACK(x)           (((x) & 0x97) == 0x11)
#define IS_DISCOVERY(x)     (((x) & 0x07) == 0x03)
#define CONTAINS_SENDER(x)  (((x) & (1<<3)) !=0)

#define MAX_RX_FRAME_LENGTH      255
#define CRC16_POLYGON         0x1002

struct stData
{
  SYSTEMTIME    sysTime;
  unsigned char ucStartByte;
  unsigned char ucReceiverAddress[10];
  unsigned char ucControlByte;
  unsigned char ucSenderAddress[10];  
  unsigned char ucDataLength;
  unsigned char ucFrameData[MAX_RX_FRAME_LENGTH];
};

unsigned int crc16_register;        // Register mit CRC16 Code

void crc16_shift(unsigned char w)
{
  unsigned char q;
  unsigned char flag;
  for(q=0;q<8;q++)
  {
    flag=(crc16_register & 0x8000)!=0;
    crc16_register<<=1;
    if(w&0x80)
    {
      crc16_register|=1;
    }
    w<<=1;
    if(flag)crc16_register ^= CRC16_POLYGON;
  }
}

void crc16_init(void)
{
  crc16_register=0xffff;
}

bool GetModulType(unsigned char p_ucType, unsigned char *p_ucTypeName, int p_iMaxSize)
{
  char ucName[50];
  switch(p_ucType)
  {
    case 0:
      strcpy(ucName,"HS485 D - Hutschienendimmer");
      break;
    case 1:
      strcpy(ucName,"HS485 S - Hutschienenschalter");
      break;
    case 2:
      strcpy(ucName,"HS485 RS - Hutschienenrolladenschalter");
      break;
    case 3:
      strcpy(ucName,"HS485 ZKL - Zutrittskontrollleser");
      break;
    case 4:
      strcpy(ucName,"JCU10 TFS - Temperatur-Feuchte-Sensor");
      break;
    case 5:
      strcpy(ucName,"HS485 IO4UP - 4-fach I/O-Modul Unterputz");
      break;
    case 7:
      strcpy(ucName,"HS485 IO127 - I/O-Modul");
      break;
    case 8:
      strcpy(ucName,"HS485 LX1 - Lux-Sensor");
      break;
    default:
      strcpy(ucName,"Type unbekannt!");
      break;
  }
  strncpy((char*)p_ucTypeName,ucName,p_iMaxSize>strlen(ucName)?strlen(ucName):p_iMaxSize);
  p_ucTypeName[strlen(ucName)]='\0';
  return true;
}

// Wandelt eine 32-Bit Adresse in ein 4-Byte unsigned char array um
void AddressHexToChar(unsigned char *p_ucAddress, unsigned long p_ulAddress)
{
  int i;
  char *x=(char*)&p_ulAddress;
  for(i=0;i<4;i++)
    p_ucAddress[i]=x[3-i];
}

// Wandelt ein 4-Byte char array in eine 32-Bit Adresse um
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress)
{
  int i;
  char *x=(char*)p_ulAddress;
  for(i=0;i<4;i++)
    x[i]=p_ucAddress[3-i];
}

// Liest einen Nachrichtenframe 
struct stData *ReadFrame(Com *m_cCom)
{
  unsigned char ucRxEscape=false;
  unsigned char ucRxByte;
  unsigned char ucRxStartByte;
  unsigned char ucRxAddressPointer;
  unsigned char ucRxAddresslen;
  unsigned char ucRxReceiverAddress[4];
  unsigned char ucRxSenderAddress[4];
  unsigned char ucRxControlByte;
  unsigned char ucRxDataLength;
  unsigned char ucRxFramePointer;
  unsigned char ucRxFrameData[MAX_RX_FRAME_LENGTH];
  unsigned int  uiLocalCRCRegister;
  //unsigned int  i;
    
  while(true)
  {
    if(m_cCom->read(&ucRxByte,1))
    {
      if (ucRxByte==ESCAPE_CHAR && !ucRxEscape)
      {
        ucRxEscape=true;
        continue;   
      }
      if (ucRxByte==FRAME_START_LONG)     // Startzeichen 0xFD empfangen
      {
        ucRxStartByte=FRAME_START_LONG;
        ucRxEscape=false;
        ucRxAddressPointer=0;
        ucRxAddresslen=4;
        ucRxFramePointer=0;
        crc16_init();
        crc16_shift(ucRxByte);
      }
      else if (ucRxByte==FRAME_START_SHORT) // Startzeichen 0xFE empfangen
      {
        ucRxStartByte=FRAME_START_SHORT;
        ucRxEscape=false;
        ucRxAddressPointer=0;
        ucRxAddresslen=1;
        ucRxFramePointer=0;
        crc16_init();
        crc16_shift(ucRxByte);
      }
      else  // Frameinhalt empfangen
      {
        if (ucRxEscape)
        {
          ucRxByte|=0x80;
          ucRxEscape=false;
        }
        if (ucRxAddressPointer<ucRxAddresslen)    // Adressbytes empfangen
        {
          ucRxReceiverAddress[ucRxAddressPointer]=ucRxByte;
          ucRxAddressPointer++;
          crc16_shift(ucRxByte);
          //printf("Addressbyte empfangen\n");
        }
        else if (ucRxAddressPointer==ucRxAddresslen)  // Controllbyte empfangen
        {
          ucRxControlByte=ucRxByte;
          ucRxAddressPointer++;
          crc16_shift(ucRxByte);
          //printf("Controllbyte empfangen\n");
        }
        else if (CONTAINS_SENDER(ucRxControlByte) && (ucRxAddressPointer < (2*ucRxAddresslen+1)))
        {
          ucRxSenderAddress[ucRxAddressPointer-ucRxAddresslen-1]=ucRxByte;
          ucRxAddressPointer++;
          crc16_shift(ucRxByte);
          //printf("Absenderadresse empfangen\n");
        }
        else if (ucRxAddressPointer!=0xFF)    // Länge empfangen
        {
          ucRxAddressPointer=0xFF;
          ucRxDataLength=ucRxByte;
          crc16_shift(ucRxByte);
          //printf("Länge empfangen: %d\n",ucRxDataLength);
        }
        else  // Daten empfangen
        {
          //printf("Daten empfangen: Datalength: %d\n",ucRxFramePointer);
          if (ucRxFramePointer==ucRxDataLength-2)   // CRC Prüfsumme folgt
            uiLocalCRCRegister=crc16_register;
          ucRxFrameData[ucRxFramePointer]=ucRxByte;
          crc16_shift(ucRxByte);
          if (ucRxFramePointer==(ucRxDataLength-1))   // Daten komplett empfangen
          {
            crc16_shift(0);
            crc16_shift(0);
            ucRxFramePointer=ucRxAddressPointer=0;
            // Checksumme überprüfen
            if (crc16_register!=0)
            { 
              printf("Fehler in der Checksumme\n");
            }
            else
            {   // Daten ausgeben
              //printf("Daten empfangen\n");
              struct stData *pNewData=new struct stData;
              SYSTEMTIME sysTime;
              GetSystemTime(&pNewData->sysTime);
              pNewData->ucStartByte=ucRxStartByte;
              memcpy(pNewData->ucReceiverAddress,ucRxReceiverAddress,4);
              pNewData->ucControlByte=ucRxControlByte;
              memcpy(pNewData->ucSenderAddress,ucRxSenderAddress,4);
              pNewData->ucDataLength=ucRxDataLength;
              memcpy(pNewData->ucFrameData,ucRxFrameData,ucRxDataLength);
              return pNewData;
            }
          }
          if (ucRxFramePointer==MAX_RX_FRAME_LENGTH)
          {
            printf("Maximale Framelänge überschritten!\n");
          }
          ucRxFramePointer++;
        }
      }
    }
    else
      return NULL;
  }
}

bool ShowType(unsigned char ucEvent)
{
  switch(ucEvent&0x33)
  {
    case 0x00:
      printf("KEY_PRESS");
      break;
    case 0x01:
      printf("KEY_HOLD");
      break;
    case 0x02:
      printf("KEY_RELEASE");
      break;
    case 0x10:
      printf("KEY_UP_PRESS");
      break;
    case 0x11:
      printf("KEY_UP_HOLD");
      break;
    case 0x12:
      printf("KEY_UP_RELEASE");
      break;
    case 0x20:
      printf("KEY_DOWN_PRESS");
      break;
    case 0x21:
      printf("KEY_DOWN_HOLD");
      break;      
    case 0x22:
      printf("KEY_DOWN_RELEASE");
      break;
  }
}

void ShowData(Com *p_cCom, int p_iAnzeigeMode)
{
  int i;
  struct stData *pFrame;
  printf("Bus-Daten werden mitgeloggt. Ende mit CRTL-C\n\n");
  p_cCom->setTimeOut(100);
  while(true)
  {
    if ( pFrame = ReadFrame(p_cCom) )
    {
      if (p_iAnzeigeMode == 0)    // Anzeige vollständig
      {
        printf("*******************************************************************************\n");
        printf("Zeit            : %02i:%02i:%02i.%03i\n",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        printf("Startzeichen    : 0x%02x\n",pFrame->ucStartByte);
        if(pFrame->ucStartByte==FRAME_START_LONG)
          printf("Zieladresse     : 0x%02x%02x%02x%02x\n", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
        else
          printf("Zieladresse     : 0x%02x\n",pFrame->ucReceiverAddress[0]);
        if(CONTAINS_SENDER(pFrame->ucControlByte))    // Sender enthalten
        {
          if(pFrame->ucStartByte==FRAME_START_LONG)
            printf("Absenderadresse : 0x%02x%02x%02x%02x\n", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
          else
            printf("Absenderadresse : 0x%02x\n",pFrame->ucSenderAddress[0]);
        }         
        printf("Controllbyte    : 0x%02x ",pFrame->ucControlByte);
        if(IS_IFRAME(pFrame->ucControlByte))      // I-Nachricht
        {
          printf("I-Nachricht\n");
          printf("    Sendefolgenummer    : %d\n",(pFrame->ucControlByte>>1)&0x03);
          printf("    Empfangsfolgenummer : %d\n",(pFrame->ucControlByte>>5)&0x03);
          printf("    SYN-Bit             : %s\n",(pFrame->ucControlByte&0x80)?"gesetzt":"");
          printf("    FIN-Bit             : %s\n",(pFrame->ucControlByte&0x10)?"gesetzt":"");
          printf("    Absenderadresse     : %s\n",(pFrame->ucControlByte&0x08)?"enthalten":"");
        }
        else if(IS_ACK(pFrame->ucControlByte))        // ACK-Nachricht
        {
          printf("ACK-Nachricht\n");
          printf("    Empfangsfolgenummer : %d\n",(pFrame->ucControlByte>>5)&0x03);
          printf("    Absenderadresse     : %s\n",(pFrame->ucControlByte&0x08)?"enthalten":"");
        }
        else if(IS_DISCOVERY(pFrame->ucControlByte))    // Discovery-Nachricht
        {
          printf("Discovery-Nachricht\n");
          printf("    Adressmaske         : %d\n",((pFrame->ucControlByte&0xF8)>>3)+1);
        }
        printf("Daten           : ");         // Daten
        for(i=0;i<pFrame->ucDataLength;i++)
          printf("%02x ",pFrame->ucFrameData[i]);
        for(i=0;i<pFrame->ucDataLength;i++)
          printf("%c",(pFrame->ucFrameData[i]<' ' || pFrame->ucFrameData[i]>'~')?'.':pFrame->ucFrameData[i]);
        printf("\n");
        if(pFrame->ucFrameData[0]=='K')
        {
          printf("Daten-Type: Key-Event\n");
          printf("\tSensor-Nr       : %d\n",pFrame->ucFrameData[1]);
          printf("\tZielaktor-Nr    : %d\n",pFrame->ucFrameData[2]);
          printf("\tEvent           : 0x%02x -> Toggle: %d Type: ",pFrame->ucFrameData[3],(pFrame->ucFrameData[3]>>2)&0x03);
          ShowType(pFrame->ucFrameData[3]);
          printf("\n");
        }
      }
      else if(p_iAnzeigeMode==1)        // Anzeige Zeilenweise
      {
        printf("%02i:%02i:%02i.%03i ",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        if(IS_IFRAME(pFrame->ucControlByte))        // I-Nachricht
        {
          printf("I-Frame[%d,Ack:%d%s%s] ",(pFrame->ucControlByte>>1)&0x03,(pFrame->ucControlByte>>5)&0x03,(pFrame->ucControlByte&0x80)?",S":"  ",(pFrame->ucControlByte&0x10)?",F":"  ");
          if(pFrame->ucControlByte&0x08)
          {
            if(pFrame->ucStartByte==FRAME_START_LONG)
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            else
              printf("%02x",pFrame->ucSenderAddress[0]);
          }
          if(pFrame->ucStartByte==FRAME_START_LONG){
            printf("->%02x%02x%02x%02x:", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],  pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }else
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
          for(i=0;i<pFrame->ucDataLength;i++)
            printf("%02x ",pFrame->ucFrameData[i]);
          for(i=0;i<pFrame->ucDataLength;i++)
            printf("%c",(pFrame->ucFrameData[i]<' ' || pFrame->ucFrameData[i]>'~')?'.':pFrame->ucFrameData[i]);
          
        }
        else if(IS_ACK(pFrame->ucControlByte))        // Discovery-Nachricht
        {
          printf("Ack-Frame[Ack:%d    ] ",(pFrame->ucControlByte>>5)&0x03);
          if(pFrame->ucControlByte&0x08)
          {
            if(pFrame->ucStartByte==FRAME_START_LONG)
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            else
              printf("%02x",pFrame->ucSenderAddress[0]);
          }
          if(pFrame->ucStartByte==FRAME_START_LONG)
            printf("->%02x%02x%02x%02x: ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          else
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
        }
        else if(IS_DISCOVERY(pFrame->ucControlByte))    // Discovery-Nachricht
        {
          printf("Discovery[Maske: %d] ",((pFrame->ucControlByte&0xF8)>>3)+1);
          if(pFrame->ucStartByte==FRAME_START_LONG){
            printf("%02x%02x", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1]);
            printf("%02x%02x", pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }else{
            printf("%02x",pFrame->ucReceiverAddress[0]);
          }
        }
        printf("\n");
      }
      // Anzeige Zeilenweise, ohne ASCII:
      else if(p_iAnzeigeMode==2)        
      {
        printf("%02i:%02i:%02i.%03i ",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        if(IS_IFRAME(pFrame->ucControlByte))        // I-Nachricht
        {
          printf("I-Frame[%d,Ack:%d%s%s] ",(pFrame->ucControlByte>>1)&0x03,(pFrame->ucControlByte>>5)&0x03,(pFrame->ucControlByte&0x80)?",S":"  ",(pFrame->ucControlByte&0x10)?",F":"  ");
          if(pFrame->ucControlByte&0x08)
          {
            if(pFrame->ucStartByte==FRAME_START_LONG)
            {
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            }
            else
            {
              printf("%02x",pFrame->ucSenderAddress[0]);
            }
          }
          if(pFrame->ucStartByte==FRAME_START_LONG)
          {
            printf("->%02x%02x%02x%02x:", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],  pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
          {
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
          }
          for(i=0;i<pFrame->ucDataLength;i++)
          {
            printf("%02x ",pFrame->ucFrameData[i]);
          }
        }
        else if(IS_ACK(pFrame->ucControlByte))        // Discovery-Nachricht
        {
          printf("Ack-Frame[Ack:%d    ] ",(pFrame->ucControlByte>>5)&0x03);
          if(pFrame->ucControlByte&0x08)
          {
            if(pFrame->ucStartByte==FRAME_START_LONG)
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            else
              printf("%02x",pFrame->ucSenderAddress[0]);
          }
          if(pFrame->ucStartByte==FRAME_START_LONG)
            printf("->%02x%02x%02x%02x: ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          else
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
        }
        else if(IS_DISCOVERY(pFrame->ucControlByte))    // Discovery-Nachricht
        {
          printf("Discovery[Maske: %d] ",((pFrame->ucControlByte&0xF8)>>3)+1);
          if(pFrame->ucStartByte==FRAME_START_LONG){
            printf("%02x%02x", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1]);
            printf("%02x%02x", pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }else{
            printf("%02x",pFrame->ucReceiverAddress[0]);
          }
        }
        printf("\n");
      }
      // Anzeige Zeilenweise, undecodiert, ohne ASCII:
      else if (p_iAnzeigeMode == 3)       
      {
        printf("%02i:%02i:%02i.%03i ",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        if (IS_IFRAME(pFrame->ucControlByte))       // I-Nachricht
        {
          printf("%02x ",pFrame->ucStartByte);
          if (pFrame->ucControlByte&0x08)
          {
            if (pFrame->ucStartByte == FRAME_START_LONG)
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            else
              printf("%02x",pFrame->ucSenderAddress[0]);
          }
          if(pFrame->ucStartByte == FRAME_START_LONG)
          {
            printf("->%02x%02x%02x%02x:", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],  pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
          printf("%02x ",pFrame->ucControlByte);
          for (i = 0; i < pFrame->ucDataLength; i++)
            printf("%02x ",pFrame->ucFrameData[i]);
        }
        else if (IS_ACK(pFrame->ucControlByte))       // Discovery-Nachricht
        {
          printf("Ack-Frame[Ack:%d    ] ",(pFrame->ucControlByte>>5)&0x03);
          if (pFrame->ucControlByte&0x08)
          {
            if (pFrame->ucStartByte == FRAME_START_LONG)
              printf("%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            else
              printf("%02x",pFrame->ucSenderAddress[0]);
          }
          if (pFrame->ucStartByte == FRAME_START_LONG)
            printf("->%02x%02x%02x%02x: ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          else
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
        }
        else if (IS_DISCOVERY(pFrame->ucControlByte))   // Discovery-Nachricht
        {
          printf("Discovery[Maske: %d] ",((pFrame->ucControlByte&0xF8)>>3)+1);
          if (pFrame->ucStartByte == FRAME_START_LONG)
          {
            printf("%02x%02x", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1]);
            printf("%02x%02x", pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
          {
            printf("%02x",pFrame->ucReceiverAddress[0]);
          }
        }
        printf("\n");
      }
      // Anzeige Zeilenweise, undecodiert, ohne ASCII:
      else if (p_iAnzeigeMode == 4)       
      {
        printf("%02i:%02i:%02i.%03i ",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        // I-Nachricht:
        if (IS_IFRAME(pFrame->ucControlByte))       
        {
          // Start Byte:
          printf("%02x ",pFrame->ucStartByte);
          // Empfaenger Adresse:    
          if(pFrame->ucStartByte == FRAME_START_LONG)
          {
            printf("->%02x%02x%02x%02x ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],  pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
          {
            printf("->%02x ",pFrame->ucReceiverAddress[0]);
          }
          // Control Byte:
          printf("%02x ",pFrame->ucControlByte);
          // Sender Adresse:
          if (pFrame->ucControlByte&0x08)
          {
            if (pFrame->ucStartByte == FRAME_START_LONG)
            {
              printf("<-%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            }
            else
            {
              printf("<-%02x",pFrame->ucSenderAddress[0]);
            }
          }
          // Datenlaenge:
          printf(" %02x:",pFrame->ucDataLength);
          // Daten:
          for (i = 0; i < pFrame->ucDataLength; i++)
          {
            if ( i == pFrame->ucDataLength - 2 )
            {
              printf("CRC:%02x ",pFrame->ucFrameData[i]);
            }
            else
            {
              printf("%02x ",pFrame->ucFrameData[i]);
            }
          } 
        }
        // Discovery-Nachricht:
        else if (IS_ACK(pFrame->ucControlByte))       
        {
          printf("Ack-Frame[Ack:%d    ] ",(pFrame->ucControlByte>>5)&0x03);
          // Empfaenger Adresse:    
          if (pFrame->ucStartByte == FRAME_START_LONG)
          {
            printf("->%02x%02x%02x%02x: ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
          {
            printf("->%02x: ",pFrame->ucReceiverAddress[0]);
          }
          // Sender Adresse:
          if (pFrame->ucControlByte&0x08)
          {
            if (pFrame->ucStartByte == FRAME_START_LONG)
            {
              printf("<-%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
            }
            else
            {
              printf("<-%02x",pFrame->ucSenderAddress[0]);
            }
          }
        }
        // Discovery-Nachricht:
        else if (IS_DISCOVERY(pFrame->ucControlByte))   
        {
          printf("Discovery[Maske: %d] ",((pFrame->ucControlByte&0xF8)>>3)+1);
          // Empfaenger Adresse:    
          if (pFrame->ucStartByte == FRAME_START_LONG)
          {
            printf("%02x%02x", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1]);
            printf("%02x%02x", pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
          }
          else
          {
            printf("%02x",pFrame->ucReceiverAddress[0]);
          }
        }
        printf("\n");
      }
      // Anzeige Zeilenweise, Update Daten decodiert, ohne ASCII:
      else if ( p_iAnzeigeMode == 5 )       
      {
        printf("%02i:%02i:%02i.%03i ",pFrame->sysTime.wHour,pFrame->sysTime.wMinute, pFrame->sysTime.wSecond,pFrame->sysTime.wMilliseconds);
        printf("%02x ",pFrame->ucStartByte);
        if (pFrame->ucStartByte == FRAME_START_LONG)
        {
          printf("->%02x%02x%02x%02x ", pFrame->ucReceiverAddress[0],pFrame->ucReceiverAddress[1],pFrame->ucReceiverAddress[2],pFrame->ucReceiverAddress[3]);
        }
        else
        {
          printf("->%02x: ",pFrame->ucReceiverAddress[0]);
        }
        printf("%02x ",pFrame->ucControlByte);
        if (pFrame->ucStartByte == FRAME_START_LONG)
        {
          printf("<-%02x%02x%02x%02x", pFrame->ucSenderAddress[0],pFrame->ucSenderAddress[1],pFrame->ucSenderAddress[2],pFrame->ucSenderAddress[3]);
        }
        else
        {
          printf("<-%02x",pFrame->ucSenderAddress[0]);
        }
        printf(" %02x:",pFrame->ucDataLength);
        // Daten:
        if ( (pFrame->ucFrameData[0]) == 0x77 )
        {
          printf("Update im Bootloadermodus ");
        }
        printf("Addr:%02x ",pFrame->ucFrameData[1]);
        printf("%02x ",pFrame->ucFrameData[2]);
        printf("Count:%02x ",pFrame->ucFrameData[3]);
        printf("Daten:");
        for (i = 4; i < pFrame->ucDataLength; i++)
        {
          if ( i == pFrame->ucDataLength - 2 )
          {
            printf("CRC:%02x ",pFrame->ucFrameData[i]);
          }
          else
          {
            printf("%02x ",pFrame->ucFrameData[i]);
          }
        } 
        printf("\n");
      }
      delete pFrame;
    }
  }
}

// Sendet ein Byte über die Schnittstelle
bool SendByte(Com *p_cCom, unsigned char p_ucByte)
{
  return p_cCom->write(&p_ucByte,1);
}

// Sendet ein Byte über die Schnittstelle, prüft jedoch auf Sonderzeichen und wandelt diese entsprechend um
bool SendDataByte(Com *p_cCom, unsigned char p_ucByte)
{
  unsigned char c;
  if((p_ucByte==FRAME_START_LONG) || (p_ucByte==FRAME_START_SHORT) || (p_ucByte==ESCAPE_CHAR)){
    c=ESCAPE_CHAR;
    if(!SendByte(p_cCom,c))
      return false;
    p_ucByte &= 0x7f;
  }
  return SendByte(p_cCom,p_ucByte);
}

// Sendet einen Datenframe 
bool SendFrame(Com *p_cCom, struct stData *p_pFrameData)
{
  int i;
  if(!p_pFrameData || !p_cCom)        // Parameter prüfen
    return false;
  if(p_pFrameData->ucStartByte!=FRAME_START_LONG)   // Frame prüfen
    return false;
  crc16_init();
  if(!SendByte(p_cCom,p_pFrameData->ucStartByte))     // Startzeichen
    return false;
  crc16_shift(p_pFrameData->ucStartByte);
  for(i=0;i<4;i++)      // Zieladresse
  {
    if(!SendDataByte(p_cCom,p_pFrameData->ucReceiverAddress[i]))
      return false;
    crc16_shift(p_pFrameData->ucReceiverAddress[i]);
  }
  if(!SendDataByte(p_cCom,p_pFrameData->ucControlByte)) // Controllbyte
    return false;
  crc16_shift(p_pFrameData->ucControlByte);
  if(!(IS_DISCOVERY(p_pFrameData->ucControlByte)))
  {
    if(CONTAINS_SENDER(p_pFrameData->ucControlByte))
    {
      for(i=0;i<4;i++)  // Absenderadresse
      {
        if(!SendDataByte(p_cCom,p_pFrameData->ucSenderAddress[i]))
          return false;
        crc16_shift(p_pFrameData->ucSenderAddress[i]);
      }
    }   
  }
  if(!SendDataByte(p_cCom,p_pFrameData->ucDataLength+2))  // Framelänge
    return false;
  crc16_shift(p_pFrameData->ucDataLength+2);
  for(i=0;i<p_pFrameData->ucDataLength;i++)       // Framedaten
  {
    if(!SendDataByte(p_cCom,p_pFrameData->ucFrameData[i]))
      return false;
    crc16_shift(p_pFrameData->ucFrameData[i]);
  }
  crc16_shift(0);
  crc16_shift(0);
  if(!SendDataByte(p_cCom,(crc16_register>>8)&0xff))  // CRC16-Checksumme
    return false;
  if(!SendDataByte(p_cCom,(crc16_register)&0xff))
    return false;
  return true;
}

bool SendAck(Com *p_cCom, unsigned char* p_ucReceiverAddress, unsigned char *p_ucSenderAddress, unsigned char p_ucEmpfangsfolgenummer)
{
  struct stData stAckData;
  stAckData.ucControlByte=((p_ucEmpfangsfolgenummer&0x03)<<5) | 0x19;
  stAckData.ucDataLength=0;
  stAckData.ucStartByte=FRAME_START_LONG;
  memcpy(stAckData.ucSenderAddress,p_ucSenderAddress,4);
  memcpy(stAckData.ucReceiverAddress,p_ucReceiverAddress,4);
  SendFrame(p_cCom,&stAckData);
}


/* Sendet eine Nachricht und wartet auf die Bestätigung
   Rückgabewert: Falls Bestätigung I-Frame, dann wird der I-Frame zurückgegeben
*/
struct stData *Send(Com *p_cCom, struct stData *p_pFrameData)
{
  int iRetryCounter=3;      // Maximale Anzahl an Wiederholungen
  if ((!p_cCom) || (!p_pFrameData))    // Prüfen der Existenz der Übergabeparameter
    return NULL;
  p_cCom->setTimeOut(100);  // Maximale Zeit für die Antwort vom Modul
  while (iRetryCounter)
  {
    struct stData *pReturnFrame;
    
    if (!SendFrame(p_cCom,p_pFrameData))   // Nachricht senden
      return NULL;
    if (IS_IFRAME(p_pFrameData->ucControlByte))
    {   
      if (pReturnFrame = ReadFrame(p_cCom))    // Auf Antwort warten
      {
        unsigned long ulAddress1,ulAddress2;
        AddressCharToHex(pReturnFrame->ucReceiverAddress,&ulAddress1);   // Umwandeln der empfangenen Adresse in unsigned long
        AddressCharToHex(p_pFrameData->ucSenderAddress,&ulAddress2);     // Umwandeln der empfangenen Adresse in unsigned long     
        //printf("ucReceiverAddress: 0x%02x%02x%02x%02x\n", pReturnFrame->ucReceiverAddress[0], pReturnFrame->ucReceiverAddress[1], pReturnFrame->ucReceiverAddress[2], pReturnFrame->ucReceiverAddress[3]);
        //printf("ucSenderAddress  : 0x%02x%02x%02x%02x\n", p_pFrameData->ucSenderAddress[0], p_pFrameData->ucSenderAddress[1], p_pFrameData->ucSenderAddress[2], p_pFrameData->ucSenderAddress[3]);
        printf("  ucControlByte (answer): 0x%01x\n", pReturnFrame->ucControlByte);
        if ( IS_IFRAME( pReturnFrame->ucControlByte ))
        {
          printf("  ucControlByte (answer): I-Nachricht \n");  
        }
        if ( IS_ACK( pReturnFrame->ucControlByte ))
        {
          printf("  ucControlByte (answer): ACK-Nachricht \n");  
        }
        if ( IS_DISCOVERY( pReturnFrame->ucControlByte ))
        {
          printf("  ucControlByte (answer): Discovery-Nachricht \n");  
        }
          
        // Bit B (Bit 3 - Eigene Sender-Adresse) gesetzt? 
        if (((pReturnFrame->ucControlByte>>3)&0x01) == 1 )
        {
          printf("Mit Absenderadresse 1\n");
	        if ((ulAddress1 == ulAddress2) && pReturnFrame 
             && (IS_IFRAME(pReturnFrame->ucControlByte) || IS_ACK(pReturnFrame->ucControlByte))) // Bestätigung vom angesprochenen Modul? (I-Frame oder ACK)
	        {
            printf("Mit Absenderadresse 2\n");
            printf("ucReceiverAddress (answer): 0x%08x\n", ulAddress1);
            printf("ucSenderAddress   (sends) : 0x%08x\n", ulAddress2);
	          if (((pReturnFrame->ucControlByte>>5)&0x03) == ((p_pFrameData->ucControlByte>>1)&0x03)) // Stimmt Sendefolgenummer mit Empfangsfolgenummer überein?     
	          {
	            if (IS_IFRAME(pReturnFrame->ucControlByte))    // Wenn I-Frame, dann Antwort schicken
	            {
	              SendAck(p_cCom,pReturnFrame->ucSenderAddress, pReturnFrame->ucReceiverAddress,  (pReturnFrame->ucControlByte>>1)&0x03);
	              return pReturnFrame;
	            }
	            else
	              return NULL;
	          }
	        }
        }
        else
        {
          printf("Ohne Absenderadresse 1\n");
          if (pReturnFrame 
             && (IS_IFRAME(pReturnFrame->ucControlByte) || IS_ACK(pReturnFrame->ucControlByte))) // Bestätigung vom angesprochenen Modul? (I-Frame oder ACK)
          {
            printf("Ohne Absenderadresse 2\n");
            if (((pReturnFrame->ucControlByte>>5)&0x03) == ((p_pFrameData->ucControlByte>>1)&0x03)) // Stimmt Sendefolgenummer mit Empfangsfolgenummer überein?     
            {
              if (IS_IFRAME(pReturnFrame->ucControlByte))    // Wenn I-Frame, dann Antwort schicken
              {
                SendAck(p_cCom,pReturnFrame->ucSenderAddress, pReturnFrame->ucReceiverAddress,  (pReturnFrame->ucControlByte>>1)&0x03);
                return pReturnFrame;
              }
              else
                return NULL;
            }
          }
        }
        delete pReturnFrame;
      }
      iRetryCounter--;
    }
    else
      break;
  }
  return NULL;
}

// Setzt den Zustand eines Ausgangs an einem Module
void SetOutput(Com *p_cCom)
{ 
  struct stData pFrame;
  unsigned long ulAddress;
  unsigned int uiActor;
  unsigned int uiValue;
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  // Adresse angeben
  printf("\nAdresse(Hex): ");
  scanf("%x",&ulAddress);
  // Aktor
  printf("Aktor(Hex): ");
  scanf("%x",&uiActor);
  // Zustand; je nach Modultype unterschiedlich-> siehe Protokollbeschreibung
  printf("HS485S  0x00 Aus\n\t0x01 An\n\t0xFF Toggeln\n");
  printf("HS485D  0-0x10 Helligkeit\n\t0x11 runterdimmen\n\t0x12 hochdimmen\n\t0x13 hoch-/runterdimmen\n\t0x14 an/aus\n\t0x15 alte Helligkeit\n");
  printf("Wert(Hex): ");
  scanf("%x",&uiValue);
  // Senden u an alle:
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;
  pFrame.ucDataLength=4;
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  pFrame.ucFrameData[0]='s';
  pFrame.ucFrameData[1]=0x00;       // Sensor; hier nur Dummy, da nicht erforderlich
  pFrame.ucFrameData[2]=uiActor&0xFF;   // Zielaktor
  pFrame.ucFrameData[3]=uiValue&0xFF;   // Wert
  Send(p_cCom,&pFrame);
  printf("\n");
}

/************************************************************
    Bootloadermodus / Updatemodus beenden:
************************************************************/  
void ExitBootloadermode(FILE *fp, Com *p_cCom)  
{
  struct stData pFrame;
  unsigned long ulAddress;  
  
  printf("Bootloadermodus beenden\n");
  printf(" Senden von '0' (Paketgroesse abfragen) an Modul 0x%08x\n",ulAddress);
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x59;
  pFrame.ucDataLength=0;
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  SendFrame(p_cCom,&pFrame);
  
  printf(" Senden von 'u' 10mal an alle \n");
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0xD0;
  pFrame.ucDataLength=2;
  AddressHexToChar(pFrame.ucReceiverAddress,0xFFFFFFFF);
  pFrame.ucFrameData[0]='g';		// 0x67
  pFrame.ucFrameData[1]=0x00;     // Sensor; hier nur Dummy, da nicht erforderlich
  for (int i=0;i<10;i++)
  {
    SendFrame(p_cCom,&pFrame);
  }
  fclose(fp);
  printf("\nUPDATE beendet !\n");
}

/************************************************************
 Firmwareupdate von einem Modul
************************************************************/
int FirmwareUpdate(Com *p_cCom)
{ 
  struct stData pFrame;
  unsigned long ulAddress;
  unsigned long FlashAddress = 0;
  unsigned int  uiActor;
  unsigned int  uiValue;
  FILE          *fp;
  unsigned long iZeile = 0;
  unsigned int  data_len = 64;
  unsigned int  hs485_databyte = 0;
  unsigned int  reset_hs485_databyte = 0;
  unsigned int  sendefolgenummer = 0;
  struct stData *pReturnFrame;
  // state_checksum: 
  // -1: checksum is not correct
  //  0: checksum is unknown
  //  1: checksum is correct
  unsigned int  state_checksum = 0;
  
  p_cCom->setTimeOut(1000);   // Timeout auf 1 Sekunde setzen, weil Antwort von HS485 PCI unregelmäßig kommt
  printf("Modul wird upgedated\n");
  crc16_init();
  
  //fp = fopen("d:\\Projekte\\HAUS_BUS\\HS485\\Module\\HS485S\\Firmware-ATmega8\\hs485s_hw1_sw2_00.hex","r");
  fp = fopen("hs485s_hw1_sw2_00.hex","r");
  if ( fp == NULL )
  {
    printf("\nERROR: Fehler beim Oeffnen des Files %s\n ",fp) ;
    return 1;
  }

  printf("Oeffnen des Files %s\n ",fp);
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  
  // Adresse angeben
  // printf("\nAdresse(Hex): ");
  // scanf("%x",&ulAddress);
  ulAddress = 0x1028;
  printf("Senden von 'u' 10mal an alle \n");
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x94;
  pFrame.ucDataLength=2;
  AddressHexToChar(pFrame.ucReceiverAddress,0xFFFFFFFF);
  pFrame.ucFrameData[0]='u';		// 0x75
  pFrame.ucFrameData[1]=0x00;   // Sensor; hier nur Dummy, da nicht erforderlich
  for (int i=0;i<10;i++)
  {
    SendFrame(p_cCom,&pFrame);
  }

  printf("Senden von 'p' (Paketgroesse abfragen) an Modul 0x%08x\n",ulAddress);
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x94;
  pFrame.ucDataLength=1;
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  pFrame.ucFrameData[0]='p';		// 0x70
  if( pReturnFrame = Send(p_cCom,&pFrame) )
  {   
    printf("Antwort vom Modul mit Adresse 0x%08x\n",ulAddress);
    printf("Datenbyte 1 : 0x%x\n",pReturnFrame->ucFrameData[0]);
    printf("Datenbyte 2 : 0x%x\n",pReturnFrame->ucFrameData[1]);
    delete(pReturnFrame);
  }
  else
  {
    printf("\nERROR: Keine Antwort vom Modul, oder Fehler beim Senden ==> EXIT \n");
    ExitBootloadermode(fp, p_cCom);
    return 1;
  }   

  printf("Senden von '0' (Paketgroesse abfragen) an Modul 0x%08x\n",ulAddress);
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x39;
  pFrame.ucDataLength=0;
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  SendFrame(p_cCom,&pFrame);
  SendFrame(p_cCom,&pFrame);
 
  // Firmwaredaten schreiben 1. 64 Byte:
  // Autor: Klaus Wachtler (mfgkw)
  // Datum: 09.09.2010 22:11 
  // http://www.mikrocontroller.net/topic/189911
  unsigned int bytecount_intel_hex, bytecount_hs485, address_intel_hex, recordtype_intel_hex, checksum;
  bytecount_hs485 = 0;
  
  /* Intel-Hex-File einlesen und per HS485 senden */
  while( "endless" )
  {
    iZeile++;
    unsigned int retval;
    /* Read Header from Intel-Hex-File: */
    retval = fscanf( fp, " :%2x%4x%2x", &bytecount_intel_hex, &address_intel_hex, &recordtype_intel_hex );
    printf( "fscanf: bytecount_intel_hex: 0x%x, address_intel_hex: 0x%x, recordtype_intel_hex: 0x%x \n", bytecount_intel_hex, address_intel_hex, recordtype_intel_hex );
    if( retval != 3 )
    {
      fprintf( stderr, "\n ERROR: Zeilenstart nicht lesbar in Zeile %lu (retval: %lu) \n", iZeile, retval );
      ExitBootloadermode(fp, p_cCom);
      return 1;
    }
    
    /* ERROR: Zeile zu lang: */  
    else if( bytecount_intel_hex > data_len )
    {
      fprintf( stderr, "\n ERROR: Zeile zu lang: bytecount_intel_hex: 0x%2x, data_len: 0x%2x\n", bytecount_intel_hex, data_len );
      ExitBootloadermode(fp, p_cCom);
      return 1;
    }
    
    /* Recordtyp == 0: Data Record / Nutzdaten: */  
    else if( recordtype_intel_hex == 0 )
    {
      bytecount_hs485 = bytecount_intel_hex + bytecount_hs485;
      // Daten aud Intel-Hex-Header merken, bis zum ersten vollen Block ==> fertig zum Senden über RS232:
      // if ( hs485_databyte == 0 && reset_hs485_databyte == 1 )
      if ( hs485_databyte == 0 )
      {
        pFrame.ucStartByte = FRAME_START_LONG;
        // Set Controlbyte:
        if ( FlashAddress == 0)
        {
          pFrame.ucControlByte=0xB6;
        }
        else
        {
          pFrame.ucControlByte = ((sendefolgenummer * 2) & 0x06) + 0x30;
          // printf( "sendefolgenummer: %02x\n", sendefolgenummer );
          // printf( "pFrame.ucControlByte: %02x\n", pFrame.ucControlByte );
          sendefolgenummer++;
          if ( sendefolgenummer >= 4 )
          {
            sendefolgenummer = 0;
          }
        }
        
				printf( " pFrame.ucDataLength: 0x%04x \n", pFrame.ucDataLength );
        AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
        pFrame.ucFrameData[0] = 'w';
        pFrame.ucFrameData[1] = address_intel_hex / 0x100;  // High-Adresse im Flash
        pFrame.ucFrameData[2] = address_intel_hex % 0x100;  // Low-Adresse im Flash
        pFrame.ucFrameData[3] = 0x40;    // Anzahl der Daten-Bytes
        printf( " HS485-Flash-Addr.: %02x%02x \n", pFrame.ucFrameData[1], pFrame.ucFrameData[2] );
				bytecount_hs485 = 0;
      } 
      
      printf( "=>address_intel_hex 0x%04x, FlashAddress: 0x%04x, hs485_databyte: 0x%04x\n", address_intel_hex, FlashAddress, hs485_databyte );
      // Wenn Adresse aus Intel-Hex-File anders, als erwartet ==> HS485 Daten mit 0xFF füllen:
      if ((FlashAddress != address_intel_hex) & (hs485_databyte != 0))  
      {
        // while( bytecount_hs485 < 64 )
        while( hs485_databyte < 64 )
        {
          printf( "hs485_databyte: 0x%02x, bytecount_hs485: 0x%02x == 0xFF\n", hs485_databyte, bytecount_hs485 );
          pFrame.ucFrameData[hs485_databyte + 4] = 0xFF;     // Zu schreibendes Byte
          hs485_databyte++;
          bytecount_hs485++;
        }
        pFrame.ucDataLength = hs485_databyte + 4;   // Datenlaenge (0x46)
        hs485_databyte = 0;
        reset_hs485_databyte = 1; 
        printf( "Set: hs485_databyte = 0 \n" );
        // Send datas per HS485
        printf( "==>Send RS485 - ucDataLength: 0x%02x\n", pFrame.ucDataLength );
        printf( "Type: 0x%02x, HS485-Flash-Addr.: %02x%02x \n", pFrame.ucFrameData[0], pFrame.ucFrameData[1], pFrame.ucFrameData[2] );
        Send(p_cCom,&pFrame);
        state_checksum = 0;

        // HS485-Header neu setzen:
				printf( " pFrame.ucDataLength: 0x%04x \n", pFrame.ucDataLength );
        AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
        pFrame.ucFrameData[0] = 'w';
        pFrame.ucFrameData[1] = address_intel_hex / 0x100;  // High-Adresse im Flash
        pFrame.ucFrameData[2] = address_intel_hex % 0x100;  // Low-Adresse im Flash
        pFrame.ucFrameData[3] = 0x40;    // Anzahl der Daten-Bytes
        printf( " HS485-Flash-Addr.: %02x%02x \n", pFrame.ucFrameData[1], pFrame.ucFrameData[2] );
				bytecount_hs485 = 0;
      }

      unsigned int own_checksum = 0;
      
      FlashAddress = address_intel_hex;
      bytecount_hs485 = bytecount_intel_hex + bytecount_hs485;
      printf( " bytecount_hs485  1: 0x%04x,", bytecount_hs485 );
      printf( " address_intel_hex: 0x%04x\n", address_intel_hex );
      own_checksum -= bytecount_intel_hex;
      own_checksum -= ( address_intel_hex & 0xFF ) + (address_intel_hex >> 8);
      own_checksum -= recordtype_intel_hex;
      printf( "bytecount_intel_hex: 0x%04x \n", bytecount_intel_hex );
      printf( "Write Data Bytes... \n");
      unsigned int iDatabyte;
      for( iDatabyte=0; iDatabyte < bytecount_intel_hex; ++iDatabyte )
      {
        unsigned int c;
      
        // printf( "\n iDatabyte: %04d", iDatabyte );
        if ( fscanf( fp, "%2x", &c ) != 1 )
        {
          fprintf( stderr, "\nERROR: Zeichen %d in Zeile %lu nicht lesbar\n", iDatabyte, iZeile);
          ExitBootloadermode(fp, p_cCom);
          return 1;
        }
        else
        {
          // ok, Zeichen gelesen
          own_checksum -= c;
          pFrame.ucFrameData[hs485_databyte + 4] = c;   // Zu schreibendes Byte
          if ( address_intel_hex >= 0x1600 )
          {  
            // printf( "c: 0x%02x, address_intel_hex: 0x%04x, \n", c, address_intel_hex );
            printf( "c: 0x%02x, address_intel_hex: 0x%04x, ", pFrame.ucFrameData[hs485_databyte + 4], address_intel_hex );
            // printf( "c: 0x%02x, address_intel_hex: 0x%04x, ", pFrame.ucFrameData[6], address_intel_hex );
            printf( "hs485_databyte: 0x%02x, FlashAddress: 0x%02x\n", hs485_databyte, FlashAddress );
          }
          hs485_databyte++;
          FlashAddress++;
          
          if (hs485_databyte > 63)
          {
            pFrame.ucDataLength = hs485_databyte + 4;   // Datenlaenge (0x46)
            hs485_databyte = 0;
            reset_hs485_databyte = 1; 
            printf( "=>HS485-Record complete: hs485_databyte > 63 \n" );
            printf( "=>Send HS485-Record: len:%x, addr:%x%x \n\n", pFrame.ucDataLength, pFrame.ucFrameData[1], pFrame.ucFrameData[2] );
          }
        }  
      }
      
      /* Checksumme vergleichen */  
      if( fscanf( fp, "%2x", &checksum ) != 1 )
      {
        fprintf( stderr, "\n ERROR: Lesefehler bei Checksumme in Zeile %lu\n", iZeile );
        state_checksum = -1;
        ExitBootloadermode(fp, p_cCom);
        return 1;
      }
      else
      {
        state_checksum = 1;
      }
      
      // Send datas per HS485
      if ( hs485_databyte == 0 && state_checksum == 1 )
      {
        printf( "==>Send RS485 - ucDataLength: 0x%02x\n", pFrame.ucDataLength );
        printf( "Type: 0x%02x, HS485-Flash-Addr.: %02x%02x \n", pFrame.ucFrameData[0], pFrame.ucFrameData[1], pFrame.ucFrameData[2] );
        Send(p_cCom,&pFrame);
        state_checksum = 0;
      }
    } // Recordtyp == 0: Data Record / Nutzdaten
    
    /* Satztyp: End of File Record */  
    else if( recordtype_intel_hex == 1 )
    {
      int RestAddress = FlashAddress % 64;
      printf( "-End of file erreicht, FlashAddress:0x%04x, RestAdress:0x%02x\n", FlashAddress, RestAddress );
			printf( "-Send RS485 (End of file): hs485_databyte 0x%02x\n", hs485_databyte );
      if( pReturnFrame = Send(p_cCom,&pFrame) )
      {   
        printf("-Antwort vom Modul mit Adresse 0x%08x\n",ulAddress);
        printf("-Datenbyte 1 : 0x%x\n",pReturnFrame->ucFrameData[0]);
        printf("-Datenbyte 2 : 0x%x\n",pReturnFrame->ucFrameData[1]);
        delete(pReturnFrame);
      }
      else
      {
        printf("\nERROR: Keine Antwort vom Modul, oder Fehler beim Senden ==> EXIT \n");
        ExitBootloadermode(fp, p_cCom);
        return 1;
      }           
      
			if ( RestAddress != 0 )
      {
        int i;
        for( i=0; i < (64 - RestAddress); ++i )       
        {
          printf( "- Write '0xFF' in RestAddress+i: %02d,%02d\n", RestAddress + i, i );
          pFrame.ucFrameData[RestAddress + i + 4] = 0xff;           // Zu schreibenden Bytes
        }
        printf( "- Send RS485: FlashAddress %02x\n", FlashAddress );
        if( pReturnFrame = Send(p_cCom,&pFrame) )
        {   
          printf("- Antwort vom Modul mit Adresse 0x%08x\n",ulAddress);
          printf("- Datenbyte 1 : 0x%x\n",pReturnFrame->ucFrameData[0]);
          printf("- Datenbyte 2 : 0x%x\n",pReturnFrame->ucFrameData[1]);
          delete(pReturnFrame);
        }
        else
        {
          printf("\nERROR: Keine Antwort vom Modul, oder Fehler beim Senden ==> EXIT \n");
          ExitBootloadermode(fp, p_cCom);
          return 1;
        }   
      }
      break;  
      // Mit break können auch Schleifen vorzeitig beendet werden. 
    } // Satztyp = 1: End of File Record 
  }   // while ( "endless" )
  ExitBootloadermode(fp, p_cCom);
}

/* 
  KeyEvent senden
*/ 
void SendKeyEvent(Com *p_cCom)
{
  struct stData pFrame;
  unsigned long ulAddress;
  unsigned int uiActor;
  unsigned int uiToggleBit;
  unsigned int uiEvent;
  int iPos=0;
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  // Adresse angeben
  printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);
   // Aktor
   printf("Aktor(Hex): ");
   scanf("%x",&uiActor);
   // Togglebit
   printf("Togglebit(0-3): ");
   scanf("%x",&uiToggleBit);
   // Event
   printf("Event  Toggle-Press\t0x00\n\tToggle-Hold\t0x01\n\tToggle-Release\t0x02\n\tUp-Press\t0x10\n\tUp-Hold\t0x11\n\tUp-Release\t0x12\n\tDown-Press\t0x20\n\tDown-Hold\t0x21\n\tDown-Release\t0x22\n");
   printf("Auswahl: ");
   scanf("%x",&uiEvent);
   
   // Senden
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;  
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  pFrame.ucFrameData[iPos++]='K';
  pFrame.ucFrameData[iPos++]=0x00;        // Sensor; hier nur Dummy, da nicht erforderlich
  pFrame.ucFrameData[iPos++]=uiActor&0xFF;    // Zielaktor
  pFrame.ucFrameData[iPos++]=uiEvent&0x33 | ((uiToggleBit&0x03)<<2);    // Event
  pFrame.ucDataLength=iPos;
  Send(p_cCom,&pFrame);
  printf("\n");
}

// 
bool GetHS485PCIVersion(Com *p_cCom)
{
  unsigned char i;
  struct stData *pData;
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  p_cCom->setTimeOut(500);    // Timout auf 2 Sekunden setzen, weil Antwort von HS485 PCI unregelmäßig kommt
  crc16_init();
  SendByte(p_cCom,0xfe);      // Startzeichen
  crc16_shift(0xfe);
  SendDataByte(p_cCom,4);     // Länge
  crc16_shift(4);
  SendDataByte(p_cCom,1);     // Befehl
  crc16_shift(1);
  crc16_shift(0);
  crc16_shift(0);
  SendDataByte(p_cCom,(crc16_register>>8)&0xff);
  SendDataByte(p_cCom,crc16_register&0xff);
  if(pData=ReadFrame(p_cCom))
  {
    p_cCom->setTimeOut(50);
    return true;
  }
  p_cCom->setTimeOut(50);
  return false;
}

// Erkennt alle verfügbaren Module
void GetModulList(Com *p_cCom)
{
  unsigned char i;
  struct stData *pData;
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  p_cCom->setTimeOut(2000);   // Timout auf 2 Sekunden setzen, weil Antwort von HS485 PCI unregelmäßig kommt
  printf("Module werden gesucht...\n");
  crc16_init();
  SendByte(p_cCom,0xfe);      // Startzeichen
  crc16_shift(0xfe);
  SendDataByte(p_cCom,4);     // Länge
  crc16_shift(4);
  SendDataByte(p_cCom,0);     // Befehl
  crc16_shift(0);
  crc16_shift(0);
  crc16_shift(0);
  SendDataByte(p_cCom,(crc16_register>>8)&0xff);
  SendDataByte(p_cCom,crc16_register&0xff);
  while(pData=ReadFrame(p_cCom)){
    // Prüfen Adresse 0xffffffff -> Suche beendet
    for(i=0;i<4;i++){
      if(pData->ucFrameData[i+1]!=0xFF)
        break;
    }
    if(i!=4)
      printf("Adresse gefunden: 0x%02x%02x%02x%02x\n",pData->ucFrameData[1],pData->ucFrameData[2],pData->ucFrameData[3],pData->ucFrameData[4]);
    else
      break;
  }
  printf("\n");
  p_cCom->setTimeOut(50);
}

void GetGeraeteZustand(Com *p_cCom)
{
  struct stData pFrame;
  unsigned long ulAddress;
  unsigned int uiActor;
  struct stData *pReturnFrame;
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  // Adresse angeben
  printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);
   // Aktor
   printf("Aktor: ");
   scanf("%x",&uiActor);
   // Senden
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;
  pFrame.ucDataLength=2;
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  pFrame.ucFrameData[0]='S';
  pFrame.ucFrameData[1]=uiActor&0xFF;   // Zielaktor
  if(pReturnFrame=Send(p_cCom,&pFrame)){
    printf("Zustand %d: %02x\n",pReturnFrame->ucFrameData[0],pReturnFrame->ucFrameData[1]);
  }else{
    printf("\nERROR: Keine Antwort\n");
  }
  printf("\n");
}

// Fragt Gerätetype, Hard- und Softwareversion eines Moduls ab
void GetModulVersion(Com *p_cCom)
{
  int i;
  struct stData pFrame;
  struct stData *pReturnFrame;
  unsigned long ulAddress;
  unsigned char ucModulName[50];
  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));
  // Adresse angeben
  printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);
   // Senden
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;
  pFrame.ucDataLength=1;
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  // Hardwaretype und -version abfragen
  pFrame.ucFrameData[0]='h';
  if(pReturnFrame=Send(p_cCom,&pFrame))
  {   
    printf("\nAntwort vom Modul mit Adresse 0x%08x\n",ulAddress);
    GetModulType(pReturnFrame->ucFrameData[0],ucModulName,50);
    printf("Hardwaretype    : %d %s\n",pReturnFrame->ucFrameData[0],ucModulName);
    printf("Hardwareversion : %d\n",pReturnFrame->ucFrameData[1]);
    delete(pReturnFrame);
  }
  else
    printf("\nERROR: Hardware : Keine Antwort vom Modul, oder Fehler beim Senden\n");

  // Softwareversion abfragen
  pFrame.ucFrameData[0]='v';
  if(pReturnFrame=Send(p_cCom,&pFrame)){
    printf("Softwareversion : %d.%d\n",pReturnFrame->ucFrameData[0], pReturnFrame->ucDataLength>=2?pReturnFrame->ucFrameData[1]:0);
  }
  else
    printf("\nERROR: Software : Keine Antwort vom Modul\n");
  printf("\n");
  // pagesize abfragen
  pFrame.ucFrameData[0]='p';
  if(pReturnFrame=Send(p_cCom,&pFrame))
  {
    printf("\nAntwort vom Modul mit Adresse 0x%08x\n",ulAddress);
    printf("Pagesize : %d\n",pReturnFrame->ucFrameData[0]);
    delete(pReturnFrame);
  }
  else
    printf("\nERROR: Pagesize : Keine Antwort vom Modul, oder Fehler beim Senden\n");

  printf("\n");
}


// Liest Temperatur und Luftfeuchtikeit von einem JCU10 TFS und zeit die Werte an
void GetTemperatur(Com *p_cCom)
{
  short          temperatur   = 0;
  float          temperatur_F = 0;
  unsigned char  feuchte      = 0;
  struct stData  pFrame;
  struct stData  *pReturnFrame;
  unsigned long  ulAddress;

  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));

   printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);

   // Senden
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;
  pFrame.ucDataLength=1;
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  //
  pFrame.ucFrameData[0]='F';
  if(pReturnFrame=Send(p_cCom,&pFrame))
   {
      temperatur += pReturnFrame->ucFrameData[0];
      temperatur <<= 8;
      temperatur |= pReturnFrame->ucFrameData[1];
      feuchte = pReturnFrame->ucFrameData[2];

      if(temperatur == (short)0x8002)
      {
         printf("\nERROR: Kommunikation zwischen JCU10 TFS und dem abgesetzten Sensor.\n");
      }

      else
      {
         temperatur_F = (float)temperatur / 10;
         printf("Temperatur:   %1.1f C\n",temperatur_F);
         printf("Feuchtigkeit: %d %%\n",feuchte);
      }
    delete(pReturnFrame);
  }
  else
    printf("\nERROR: Temperatur:Keine Antwort vom Modul\n");
}


// Liest den Helligkeitwert eines HS485 LX1 und zeigt den Wert an
void GetHelligkeit(Com *p_cCom)
{
   unsigned long  helligkeit = 0;
   unsigned long  ulAddress  = 0;
   unsigned char  stufe;
   unsigned short ad_wert;
  struct stData  pFrame;
  struct stData  *pReturnFrame;

  // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
  while(ReadFrame(p_cCom));

  // Adresse angeben
   printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);
   
   // Senden
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x98;
  pFrame.ucDataLength=1;
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  pFrame.ucFrameData[0]='L';
  if(pReturnFrame=Send(p_cCom,&pFrame))
   {
      helligkeit += pReturnFrame->ucFrameData[0];
      helligkeit <<= 8;
      helligkeit += pReturnFrame->ucFrameData[1];
      helligkeit <<= 8;
      helligkeit += pReturnFrame->ucFrameData[2];
      helligkeit <<= 8;
      helligkeit += pReturnFrame->ucFrameData[3];

    printf("Helligkeit:       %10d Lux (nicht geeicht)\n",helligkeit);
    delete(pReturnFrame);
  }
  else
    printf("\nERROR: Helligkeit:Keine Antwort vom Modul\n");
}

// Setzt den Zustand eines Ausgangs an einem Module
void SetActor(Com *p_cCom)
{ 
  unsigned long  ulAddress  = 0;
  unsigned int uiAktor=0;
  unsigned int uiAuswahl=0;
  struct stData pFrame;
  // Adresse angeben
  printf("\nAdresse(Hex): ");
   scanf("%x",&ulAddress);
   // Aktor
   while(true){
     printf("\nAktor(-1: Ende) : ");
     scanf("%d",&uiAktor);
    if(uiAktor==-1)
      break;  
    printf("Zustand:\tAn: \t0x01\n\t\tAus:\t0x00\n\t\tToggle:\t0xFF\nAuswahl: ");
    scanf("%x",&uiAuswahl);
    // Buffer leeren, damit evtl. empfangene Nachrichten gelöscht werden
    while(ReadFrame(p_cCom));
     // Senden
    pFrame.ucStartByte=FRAME_START_LONG;
    pFrame.ucControlByte=0x98;
    pFrame.ucDataLength=4;
    AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
      AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
    pFrame.ucFrameData[0]='s';
    pFrame.ucFrameData[1]=0x00;         // Sensor; hier nur Dummy, da nicht erforderlich
    pFrame.ucFrameData[2]=((unsigned char)uiAktor)&0xFF;    // Zielaktor
    pFrame.ucFrameData[3]=((unsigned char)uiAuswahl)&0xFF;    // Wert
    Send(p_cCom,&pFrame);
  }
}

// Gibt das Menü aus
void ShowMenu()
{
  printf("******************************************************************************");
  printf("\nHS485-Demo Menue\n\n");
  printf("1. Daten am Bus lesen (vollstaendige Darstellung; lang)\n");
  printf("2. Daten am Bus lesen (zeilenweise Darstellung; kurz)\n");
  printf("3. Daten am Bus lesen (zeilenweise Darstellung; kurz - ohne ASCII)\n");
  printf("4. Daten am Bus lesen (zeilenweise Darstellung; kurz - teil-dekodiert)\n");
  printf("5. Daten am Bus lesen (zeilenweise Darstellung; kurz - undekodiert)\n");
  printf("6. Daten am Bus lesen (Updatedaten werden dekodiert)\n");
  printf("9. Ausgang schalten\n");
  printf("P. Version HS485 PCI\n");
  printf("K. Key-Event senden\n");
  printf("S. Zustand abfragen\n");
  printf("s. Ausgang schalten\n");
  printf("F. JCU10 TFS abfragen\n");
  printf("H. HS485 LX1 abfragen\n");
  printf("V. Modulversion anzeigen\n");
  printf("L. Modulliste anzeigen\n");
  printf("C. Bildschirm loeschen\n");
  printf("u. Firmware Update\n");
  printf("\nE. Programmende\n\n");
}

/*
  Hauptprogramm
*/  
int main(int argc, char *argv[])
{
  int i;
  ComPort cCom;
  bool bFound=false;
  unsigned char cBuffer[1];
  unsigned char ucKeyCode;
   char cPortString[20];

   printf("HS485 Demo-Software Version %s\n",FILE_VERSION);
   // Parameter überprüfen
   if(argc!=2)
  {
    printf("HS485 PCI wird gesucht...");
    // Port mit HS485 PCI suchen  
    for(i=0;i<20;i++)
    {   printf(".");
      if(cCom.isPortAvailable(i))
      {
        //sprintf(cPortString,"COM%i",i);
            sprintf(cPortString,"\\\\.\\COM%d",i);
        if(cCom.open(cPortString,19200,100))
        {
          if(GetHS485PCIVersion(&cCom))
          {
            printf(" gefunden auf Port COM%i\n",i);
            bFound=true;
            cCom.close();
            break;
          }
          else
            cCom.close();
        }
      }
    }
    if(!bFound)
    {
      printf("\nDas PC-Interface konnte nicht gefunden werden.\n\n");
      printf("Schnittstelle angeben: %s [COMx]\n\n",argv[0]);     
    }
  }else{
    
    sprintf(cPortString, "\\\\.\\%s",argv[1]);
    bFound=true;
  }
  if(!bFound){
    system("PAUSE");
    return EXIT_SUCCESS;
  }

  // seriellen Port öffnen;
  if(!cCom.open(cPortString,19200,100))
  {
    printf("Fehler! Port %s kann nicht geoeffnet werden\n",cPortString);
    system("PAUSE");
    return 1;
  } 
  while(true)
  {
    ShowMenu();
    ucKeyCode=getch();
    switch(ucKeyCode)
    {
      case '1':
        ShowData(&cCom,0);
        break;
      case '2':
        ShowData(&cCom,1);
        break;
      case '3':
        ShowData(&cCom,2);
        break;
      case '4':
        ShowData(&cCom,3);
        break;
      case '5':
        ShowData(&cCom,4);
        break;
      case '6':
        ShowData(&cCom,5);
        break;
      case '9':
        SetOutput(&cCom);
        break;
      case 'V':
      case 'v':
        GetModulVersion(&cCom);
        break;
      case 'L':
      case 'l':
        GetModulList(&cCom);
        break;
      case 'C':
      case 'c':
        system("cls");
        break;
      case 'K':
      case 'k':
        SendKeyEvent(&cCom);
        break;
      case 'p':
      case 'P':
        GetHS485PCIVersion(&cCom);
        break;
      case 'E':   // Programmende
      case 'e':
        return EXIT_SUCCESS;
      case 'S':     
        GetGeraeteZustand(&cCom);
        break;
      case 's':
        SetActor(&cCom);
        break;
      case 'f':
      case 'F':
        GetTemperatur(&cCom);
        break;
      case 'h':
      case 'H':
        GetHelligkeit(&cCom);
        break;
      case 'u':
        FirmwareUpdate(&cCom);
        break;
    }
  }
  system("PAUSE");
  return EXIT_SUCCESS;
}

