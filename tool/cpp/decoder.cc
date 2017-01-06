#include "decoder.h"

#include <iostream>
#include <iomanip>

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
    uint64_t mul = 1;
    ret = 0;
    uint8_t b = 0;
    do {
        if(!ring_->readNext(b)) {
            ret = -1;
            return false;
        }
        ret = ret | (b*mul);
        mul = mul << 7;
    } while(b < 0x80);
    
    ret &= ~mul;
    return true;
}

bool Decoder::decodeValue(int& ret) {
  uint64_t val;
  bool decode = decodeValue(val);
  ret = (int)val;
  return decode;
}

bool Decoder::decodeString(std::string& ret) {

        uint64_t length;
        if(!decodeValue(length)) {
            return false;
        }
        if(!ring_->extractString(ret, length)) {
            return false;
        }
        return true;
}

void Decoder::oneStep() {

    do {
      ring_->setRollback();
          int current_code;
          size_t count;

          if(!decodeValue(current_code)) {
            //std::cout << "\treading current_code failed" << std::endl;
              return;
          }
          if(!decodeValue(count)) {
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
              //ss << "(" << registerd_events << ")BeginStream\n\ttime_stamp: " << time_stamp << "\n\tplatform: " << platform << "\n\tsystem frequency: " << system_frequency << "\n";
              break;
            }

            case EndStream :
              {
                //ss << "(" << registerd_events << ")Endstream\n\ttime_stamp: " << time_stamp << "\n\tDo nothing\n";
              }
            break;

            case HeapCreate :
            { 
              std::string name;
              int id;
              if(!decodeValue(id)) {
                return;
              }
              if(!decodeString(name)) {
                return;
              }
              memory_state_->addHeap(id,name, time_stamp);
              //ss << "(" << registerd_events << ")HeapCreate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n\tName: " << name << "\n";
              break;
            }

            case HeapDestroy :
            {
              int id;
              if(!decodeValue(id)) {
                return ;
              }
              memory_state_->removeHeap(id, time_stamp);
              //ss << "(" << registerd_events << ")HeapDestroy\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n";
              break;
            }

            case HeapAddCore :
            {
              int id;
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
              memory_state_->addCore(id,pointer,size_bytes,time_stamp);
              //ss << "(" << registerd_events << ")HeapAddCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
              break;
            }

            case HeapRemoveCore :
            {  
              int id;
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
              //ss << "(" << registerd_events << ")HeapRemoveCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
              break;
            }
            case HeapAllocate:
            {
              int id;
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
              memory_state_->addAllocation(id,pointer,size_bytes, time_stamp);
              //ss << "(" << registerd_events << ")HeapAllocate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; 
              break;
            }

            case HeapFree:
            { 
              int id;
              size_t pointer;
              if(!decodeValue(id)) {
                return ;
              }
              if(!decodeValue(pointer)) {
                return ;
              }
              memory_state_->removeAllocation(id,pointer, time_stamp);
              //ss << "(" << registerd_events << ")HeapFree\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n"; 
              break;
            }
            default:
              //ss << "Unhandled Event " << current_code << ", time_stamp: " << time_stamp << " num unread: " << ring_->getNumUnread() << "\n";
              return;
            break;
          } //switch(current code)
         
          registerd_events++;
          ring_->setRollback();
    }while(ring_->getNumUnread());
    ring_->doRollback();
}

std::vector<Heap*> Decoder::getMemoryState() {
  return memory_state_->getHeaps();
}