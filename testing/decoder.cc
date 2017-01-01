#include "decoder.h"
#include <iostream>
Decoder::Decoder() {
    ring_ = new RingBuffer(32);
}

RingBuffer* Decoder::getRingbuffer() {
    return ring_;
}

Decoder::~Decoder() {
    delete ring_;
}

bool Decoder::decodeValue(uint64_t& ret) {
  std::cout << "\t\t\tentered decodeValue" << std::endl;
    int count = 0;
    uint64_t mul = 1;
    ret = 0;
    uint8_t b = 0;
    do {
        if(ring_->getNumUnread() == 0) {
            ret = -1;
            return false;
        }
        b = ring_->readNext();
        std::cout << "\t\t\t\tread value " << (int)b << std::endl;
        count++;
        ret = ret | (b*mul);
        mul = mul << 7;
    } while(b < 0x80);
    
    ret &= ~mul;
    std::cout << "\t\t\texited decodeValue" << std::endl;
    return true;
}

bool Decoder::decodeString(std::string& ret) {
  std::cout << "\t\t\tentered decodeString" << std::endl;
        uint64_t length;
        if(!decodeValue(length)) {
            std::cout << "\tDecode_string: length failed";
            return false;
        }
        std::cout << "\tExtracting string, length " << length << std::endl;
        if(length > ring_->getNumUnread()) {
            std::cout << "\tDecode_string: length(" << length << ") is longer then number of unread(" << ring_->getNumUnread() << ")\n";
            return false;
        }
        ret = ring_->extractString(length);
        std::cout << "\t\t\texited decodeString" << std::endl;
        return true;
}

void Decoder::oneStep() {
  std::cout << "entered oneStep" << std::endl;
    enum EventCode
    {
      BeginStream     = 1,
      EndStream,

      HeapCreate = 18,
      HeapDestroy,

      HeapAddCore,
      HeapRemoveCore,

      HeapAllocate,
      HeapFree,
    };
    size_t num_events = 0;
    // v8::Local<v8::Object> js_obj = Nan::New<v8::Object>();
    do {
      std::cout << "\tentered decodeString - Do" << std::endl;
        ring_->setRollback();
          size_t current_code;
          std::cout << "\tDecoding current_code" << std::endl;
          if(!decodeValue(current_code)) {
            std::cout << "\treading current_code failed" << std::endl;
              return;
          }
          std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
          std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;

          size_t time_stamp;
          std::cout << "\tDecoding timestamp" << std::endl;
          if(!decodeValue(time_stamp)) {
            std::cout << "\treading timestamp failed" << std::endl;
              return;
          }
          std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
          std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;
          //std::cout << std::showbase;
          //std::cout << "Event with code: " << current_code << " time: " << time_stamp << "\n";
          switch(current_code) {
            case BeginStream :
            {
              std::cout << "\t Entering BeginStream" << std::endl;
              std::string platform;
              size_t system_frequency;
              size_t stream_magic;
              std::cout << "\t\tDecoding stream_magic" << std::endl;
              if(!decodeValue(stream_magic)) {
                std::cout << "\t\tDecode stream_magic failed" << std::endl;
                return;
              }
              std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
              std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;
              std::cout << "\tDecoding platform" << std::endl;
              if(!decodeString(platform)) {
                std::cout << "\tDecode platform failed" << std::endl;
                return;
              }
              std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
              std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;
              std::cout << "\tDecoding system_frequency" << std::endl;
              if(!decodeValue(system_frequency)) {
                std::cout << "\tDecode system_frequency failed" << std::endl;
                std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
                std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;
                return;
              }
              std::cout << "\t\t numunread: " << ring_->getNumUnread() << std::endl;
              std::cout << "\t\t readPosition: " << ring_->getReadPosition() << std::endl;
              std::cout << "\tBeginStream\nplatform: " << platform << "\nsystem frequency: " << system_frequency << std::endl;
              num_events++;
              std::cout << "\t Exiting BeginStream" << std::endl;
              break;
            }

            case EndStream :
              {
                //std::cout << "Endstream: Do nothing";
                num_events++;
              }
            break;

            case HeapCreate :
            { 
              std::cout << "\t Entering HeapCreate" << std::endl;
              std::string name;
              size_t id;
              std::cout << "\t\t Decoding Id" << std::endl;
              if(!decodeValue(id)) {
                return;
              }
              std::cout << "\t\t Decoding Name" << std::endl;
              if(!decodeString(name)) {
                return;
              }
              //std::cout << "HeapCreate\nId: " << id << "\nName: " << name;
              num_events++;
              std::cout << "\t Exiting HeapCreate" << std::endl;
              break;
            }

            case HeapDestroy :
            {
              std::cout << "\t Entering HeapDestroy" << std::endl;
              size_t id;
              std::cout << "\t\t Decoding Id" << std::endl;
              if(!decodeValue(id)) {
                return ;
              }
              //std::cout << "HeapDestroy\nId: " << id;
              num_events++;
              std::cout << "\t Exiting HeapDestroy" << std::endl;
              break;
            }

            case HeapAddCore :
            {
              std::cout << "\t Entering HeapAddCore" << std::endl;
              size_t id;
              size_t pointer;
              size_t size_bytes;
              std::cout << "\t\t Decoding Id" << std::endl;
              if(!decodeValue(id)) {
                return ;
              }
              std::cout << "\t\t Decoding pointer" << std::endl;
              if(!decodeValue(pointer)) {
                return ;
              }
              std::cout << "\t\t Decoding size" << std::endl;
              if(!decodeValue(size_bytes)) {
                return;
              }
              //std::cout << "HeapAddCore\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              std::cout << "\t Exiting HeapAddCore" << std::endl;
              num_events++;
              break;
            }

            case HeapRemoveCore :
            {  
              std::cout << "\t Entering HeapRemoveCore" << std::endl;
              size_t id;
              size_t pointer;
              size_t size_bytes;
              std::cout << "\t\t Decoding Id" << std::endl;
              if(!decodeValue(id)) {
                return ;
              }
              std::cout << "\t\t Decoding pointer" << std::endl;
              if(!decodeValue(pointer)) {
                return ;
              }
              std::cout << "\t\t Decoding size" << std::endl;
              if(!decodeValue(size_bytes)) {
                return ;
              }
              //std::cout << "HeapRemoveCore\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              num_events++;
              std::cout << "\t Exiting HeapRemoveCore" << std::endl;
              break;
            }
            case HeapAllocate:
            {
              size_t id;
              size_t pointer;
              size_t size_bytes;
              if(!decodeValue(id)) {
                return ;
              }
              if(!decodeValue(pointer)) {
                return ;
              }
              if(!decodeValue(size_bytes)) {
                return ;
              }
              //std::cout << "HeapAllocate\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              num_events++;
              break;
            }

            case HeapFree:
            { 
              size_t id;
              size_t pointer;
              if(!decodeValue(id)) {
                return ;
              }
              if(!decodeValue(pointer)) {
                return ;
              }
              //std::cout << "HeapFree\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec; 
              num_events++;
              break;
            }
            default:
              std::cout << "Unhandled Event " << current_code << ", num unread: " << ring_->getNumUnread() << "\n";
              return;
            break;
          } //switch(current code)
    }while(ring_->getNumUnread());
    ring_->doRollback();
}