#include "Adafruit_Fingerprint.h"
#include <util/delay.h>
#if (ARDUINO >= 100)
  #include <SoftwareSerial.h>
#else
  #include <NewSoftSerial.h>
#endif

#include <Streaming.h>

//static SoftwareSerial mySerial = SoftwareSerial(2, 3);

Adafruit_Fingerprint::Adafruit_Fingerprint(SoftwareSerial *ss) {

    thePassword = 0x0;
    theAddress = 0xFFFFFFFF;

    mySerial = ss;
}


void Adafruit_Fingerprint::begin(uint16_t baudrate) {
    mySerial->begin(baudrate);
}

boolean Adafruit_Fingerprint::verifyPassword(void) 
{
    uint8_t packet[] = {FINGERPRINT_VERIFYPASSWORD,
        (thePassword >> 24), (thePassword >> 16),
    (thePassword >> 8), thePassword};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 7, packet);
    int len = getReply(packet);

    if(0xff == len)
    {
        //cout << "time out" << endl;
        return false;
    }
    //cout << "len = " << len << endl;
    //cout << "packer[0] = " << packet[0] << '\t' << "packet[1] = " << packet[1] << endl;
    if ((len == 1) && (packet[0] == FINGERPRINT_ACKPACKET) && (packet[1] == FINGERPRINT_OK))
    return true;

    return false;
}

uint8_t Adafruit_Fingerprint::getImage(void) {
    uint8_t packet[] = {FINGERPRINT_GETIMAGE};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;
    return packet[1];
}

uint8_t Adafruit_Fingerprint::image2Tz(uint8_t slot) {
    uint8_t packet[] = {FINGERPRINT_IMAGE2TZ, slot};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;
    return packet[1];
}


void Adafruit_Fingerprint::setKey(unsigned long key_t)
{
    thePassword = key_t;
}
void Adafruit_Fingerprint::setAddr(unsigned long addr_t)
{
    theAddress = addr_t;
}
uint8_t Adafruit_Fingerprint::createModel(void) {
    uint8_t packet[] = {FINGERPRINT_REGMODEL};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;
    return packet[1];
}


uint8_t Adafruit_Fingerprint::storeModel(uint16_t id) {
    uint8_t packet[] = {FINGERPRINT_STORE, 0x01, id >> 8, id & 0xFF};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;
    return packet[1];
}

uint8_t Adafruit_Fingerprint::emptyDatabase(void) {
    uint8_t packet[] = {FINGERPRINT_EMPTY};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;
    return packet[1];
}

uint8_t Adafruit_Fingerprint::fingerFastSearch(void) {
    fingerID = 0xFFFF;
    confidence = 0xFFFF;
    // high speed search of slot #1 starting at page 0x0000 and page #0x00A3
    uint8_t packet[] = {FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;

    fingerID = packet[2];
    fingerID <<= 8;
    fingerID |= packet[3];

    confidence = packet[4];
    confidence <<= 8;
    confidence |= packet[5];

    return packet[1];
}

uint8_t Adafruit_Fingerprint::getTemplateCount(void) {
    templateCount = 0xFFFF;
    // get number of templates in memory
    uint8_t packet[] = {FINGERPRINT_TEMPLATECOUNT};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint8_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
    return -1;

    templateCount = packet[2];
    templateCount <<= 8;
    templateCount |= packet[3];

    return packet[1];
}

void Adafruit_Fingerprint::writePacket(uint32_t addr, uint8_t packettype, uint16_t len, uint8_t *packet) {
#ifdef FINGERPRINT_DEBUG
    Serial.print("---> 0x");
    Serial.print((uint8_t)(FINGERPRINT_STARTCODE >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)FINGERPRINT_STARTCODE, HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 24), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 16), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)packettype, HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(len >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(len), HEX);
#endif


    mySerial->write((uint8_t)(FINGERPRINT_STARTCODE >> 8));
    mySerial->write((uint8_t)FINGERPRINT_STARTCODE);
    mySerial->write((uint8_t)(addr >> 24));
    mySerial->write((uint8_t)(addr >> 16));
    mySerial->write((uint8_t)(addr >> 8));
    mySerial->write((uint8_t)(addr));
    mySerial->write((uint8_t)packettype);
    mySerial->write((uint8_t)(len >> 8));
    mySerial->write((uint8_t)(len));

    uint16_t sum = (len>>8) + (len&0xFF) + packettype;
    for (uint8_t i=0; i< len-2; i++) {

        mySerial->write((uint8_t)(packet[i]));
#ifdef FINGERPRINT_DEBUG
        Serial.print(" 0x"); Serial.print(packet[i], HEX);
#endif
        sum += packet[i];
    }
#ifdef FINGERPRINT_DEBUG
    //Serial.print("Checksum = 0x"); Serial.println(sum);
    Serial.print(" 0x"); Serial.print((uint8_t)(sum>>8), HEX);
    Serial.print(" 0x"); Serial.println((uint8_t)(sum), HEX);
#endif
#if ARDUINO >= 100
    mySerial->write((uint8_t)(sum>>8));
    mySerial->write((uint8_t)sum);
    
   // cout << "sum = " << sum << endl;
#else
    mySerial->print((uint8_t)(sum>>8), BYTE);
    mySerial->print((uint8_t)sum, BYTE);
#endif
}


uint8_t Adafruit_Fingerprint::getReply(uint8_t packet[], uint16_t timeout) 
{
    uint8_t reply[20], idx;
    uint16_t timer=0;

    idx = 0;
#ifdef FINGERPRINT_DEBUG
    Serial.print("<--- ");
#endif
    while (true) 
    {
        while (!mySerial->available()) 
        {
            delay(1);
            timer++;
            if (timer >= timeout) return FINGERPRINT_TIMEOUT;
        }
        // something to read!
        
        reply[idx] = mySerial->read();
        
#ifdef FINGERPRINT_DEBUG
        Serial.print(" 0x"); Serial.print(reply[idx], HEX);
#endif
        if ((idx == 0) && (reply[0] != (FINGERPRINT_STARTCODE >> 8)))           // wait head
        {
            continue;
        }
        
        idx++;

        // check packet!
        if (idx >= 9) 
        {
            if ( (reply[0] != (FINGERPRINT_STARTCODE >> 8)) || (reply[1] != (FINGERPRINT_STARTCODE & 0xFF)) )   // head err
            {
                return FINGERPRINT_BADPACKET;
            }
            
            uint8_t packettype = reply[6];
            
            uint16_t len = reply[7];
            
            len <<= 8;
            len |= reply[8];
            
            len -= 2;                   // packge len without checksum

            if (idx <= (len+10)) continue;
            
            packet[0] = packettype;
            for (uint8_t i=0; i<len; i++) 
            {
                packet[1+i] = reply[9+i];
            }
#ifdef FINGERPRINT_DEBUG
            Serial.println();
#endif
            return len;
        }
    }
}
