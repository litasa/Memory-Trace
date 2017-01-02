#include "decoder.h"
#include <iostream>
Decoder::Decoder() {
    ring_ = new RingBuffer();
    memory_state_ = new MemoryState();
    registerd_events = 0;
}

RingBuffer* Decoder::getRingbuffer() {
    return ring_;
}

Decoder::~Decoder() {
    delete ring_;
    delete memory_state_;
}

bool Decoder::decodeValue(uint64_t& ret) {
 //std::cout << "\t\t\tDecodeValue: \n";
    uint64_t mul = 1;
    ret = 0;
    uint8_t b = 0;
    do {
        if(ring_->getNumUnread() == 0) {
            ret = -1;
            return false;
        }
        b = ring_->readNext();
        //std::cout << "\t\t\t\tRead value: " << (int)(uint8_t)(b) << ", readPos was: " << ring_->getReadPosition() - 1 << ", write_position: " << ring_->getWritePosition() << ", num unread was: " << ring_->getNumUnread() + 1 << "\n";
        ret = ret | (b*mul);
        mul = mul << 7;
    } while(b < 0x80);
    
    ret &= ~mul;
    return true;
}

bool Decoder::decodeString(std::string& ret) {

        uint64_t length;
        if(!decodeValue(length)) {
            return false;
        }
        if(length > ring_->getNumUnread()) {

            return false;
        }
        ret = ring_->extractString(length);

        return true;
}

void Decoder::oneStep() {
    do {
     
        ring_->setRollback();
          size_t current_code;

          if(!decodeValue(current_code)) {
            //std::cout << "\treading current_code failed" << std::endl;
              return;
          }


          size_t time_stamp;

          if(!decodeValue(time_stamp)) {
            //std::cout << "\treading time_stamp failed" << std::endl;
              return;
          }

          switch(current_code) {
            case BeginStream :
            {

              std::string platform;
              size_t system_frequency;
              size_t stream_magic;

              if(!decodeValue(stream_magic)) {
                //std::cout << "\t\tDecode stream_magic failed" << std::endl;
                return;
              }
              if(!decodeString(platform)) {
                //std::cout << "\tDecode platform failed" << std::endl;
                return;
              }
              if(!decodeValue(system_frequency)) {
                //std::cout << "\tDecode system_frequency failed" << std::endl;
                return;
              }
              registerd_events++;
              //ss << "(" << registerd_events << ")BeginStream\n\ttime_stamp: " << time_stamp << "\n\tplatform: " << platform << "\n\tsystem frequency: " << system_frequency << "\n";
              break;
            }

            case EndStream :
              {
                registerd_events++;
                //ss << "(" << registerd_events << ")Endstream\n\ttime_stamp: " << time_stamp << "\n\tDo nothing\n";
              }
            break;

            case HeapCreate :
            { 
              std::string name;
              size_t id;
              if(!decodeValue(id)) {
                return;
              }
              if(!decodeString(name)) {
                return;
              }
              registerd_events++;
              memory_state_->addHeap(id,name, time_stamp);
              if(id == 246) {
                std::cout << "ringbuffer read pos: " << ring_->getReadPosition() << std::endl;
              }
              //ss << "(" << registerd_events << ")HeapCreate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n\tName: " << name << "\n";
              break;
            }

            case HeapDestroy :
            {
              size_t id;
              if(!decodeValue(id)) {
                return ;
              }
              memory_state_->removeHeap(id, time_stamp);
              registerd_events++;
              //ss << "(" << registerd_events << ")HeapDestroy\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n";
              break;
            }

            case HeapAddCore :
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
                return;
              }
              registerd_events++;
              memory_state_->addCore(id,pointer,size_bytes,time_stamp);
              //ss << "(" << registerd_events << ")HeapAddCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
              break;
            }

            case HeapRemoveCore :
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
              memory_state_->removeCore(id,pointer,size_bytes, time_stamp);
              registerd_events++;
              //ss << "(" << registerd_events << ")HeapRemoveCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
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
              registerd_events++;
              memory_state_->addAllocation(id,pointer,size_bytes, time_stamp);
              //ss << "(" << registerd_events << ")HeapAllocate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
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
              registerd_events++;
              memory_state_->removeAllocation(id,pointer, time_stamp);
              //ss << "(" << registerd_events << ")HeapFree\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n"; 
              break;
            }
            default:
              //ss << "Unhandled Event " << current_code << ", time_stamp: " << time_stamp << " num unread: " << ring_->getNumUnread() << "\n";
              return;
            break;
          } //switch(current code)
    }while(ring_->getNumUnread());
    ring_->doRollback();
}