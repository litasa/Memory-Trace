#include "decoder.h"

#include <iostream>
#include <iomanip>

Decoder::Decoder() {
    ring_ = new RingBuffer();
    memory_state_ = new MemoryState();
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

bool Decoder::getHeader(int& current_code, size_t& count, size_t& time_stamp) {

  size_t counter;
  if(!decodeValue(counter)) {
    return false;
  }

  if(counter != event_counter_) {
    std::cout << "event counter missmatch, " << "recieved: " << counter << " is on: " << event_counter_ << std::endl;
    ring_->printStats();
    return false;
  }

  if(!decodeValue(current_code)) {
      return false;
  }

  if(!decodeValue(time_stamp)) {
      return false;
  }
  return true;
}

void Decoder::saveBuffer(char* buff, size_t size) {
  saved_buffers_.push_back(std::make_pair(buff,size));
}

bool Decoder::tryGettingEvents(unsigned int times) {
  std::cout << "trying to get " << times <<" successful events" << std::endl;
  unsigned int successes = 0;
  size_t saved_time = last_timestamp;
  for(unsigned int i = 0; i < times; ++i) {
    if(oneEvent(false)) {
      successes++;
    }
  }
  last_timestamp = saved_time;
  event_counter_ -= successes;
  if(successes == times) {
    std::cout << "yey" << std::endl;
    return true;
  }
  else {
    std::cout << "ney" << std::endl;
    return false;
  }
  
}

void Decoder::decodeEvents() {

    do {
      ring_->setRollback();

      oneEvent();
      
      ring_->setRollback();
    }while(ring_->getNumUnread());
    ring_->doRollback();
}

std::vector<Heap*> Decoder::getMemoryState() {
  return memory_state_->getHeaps();
}

bool Decoder::oneEvent(bool recording) {
  int current_code;
      size_t time_stamp;
      size_t count;

      if(!getHeader(current_code, count, time_stamp)) {
        return false;
      }
      last_timestamp = time_stamp;

      switch(current_code) {
          case BeginStream :
          {
            std::string platform;
            size_t system_frequency;
            size_t stream_magic;

            if(!decodeValue(stream_magic)) {
              return false;
            }
            if(!decodeString(platform)) {
              return false;
            }
            if(!decodeValue(system_frequency)) {
              return false;
            }
            break;
          }

          case EndStream :
          {
            break;
          }
          
          case HeapCreate :
          { 
            std::string name;
            int id;
            if(!decodeValue(id)) {
              return false;
            }
            if(!decodeString(name)) {
              return false;
            }
            if(recording) {
              memory_state_->addHeap(id, name, time_stamp, event_counter_);
            }
            
            break;
          }

          case HeapDestroy :
          {
            int id;
            if(!decodeValue(id)) {
              return false;
            }
            if(recording) {
              memory_state_->removeHeap(id, time_stamp, event_counter_);
            }
            break;
          }

          case HeapAddCore :
          {
            int id;
            size_t pointer;
            size_t size_bytes;
            if(!decodeValue(id)) {
              return false;
            }
            if(!decodeValue(pointer)) {
              return false;
            }
            if(!decodeValue(size_bytes)) {
              return false;
            }
            if(recording) {
              memory_state_->addCore(id,pointer,size_bytes,time_stamp, event_counter_);
            }
            
            break;
          }

          case HeapRemoveCore :
          {  
            int id;
            size_t pointer;
            size_t size_bytes;
            if(!decodeValue(id)) {
              return false;
            }
            if(!decodeValue(pointer)) {
              return false;
            }
            if(!decodeValue(size_bytes)) {
              return false;
            }
            if(recording) {
              memory_state_->removeCore(id,pointer,size_bytes, time_stamp, event_counter_);
            }
            
            break;
          }
          case HeapAllocate:
          {
            int id;
            size_t pointer;
            size_t size_bytes;
            if(!decodeValue(id)) {
              return false;
            }
            if(!decodeValue(pointer)) {
              return false;
            }
            if(!decodeValue(size_bytes)) {
              return false;
            }
            if(recording) {
              memory_state_->addAllocation(id,pointer,size_bytes, time_stamp, event_counter_);
            }
            
            break;
          }

          case HeapFree:
          { 
            int id;
            size_t pointer;
            if(!decodeValue(id)) {
              return false;
            }
            if(!decodeValue(pointer)) {
              return false;
            }
            if(recording) {
              memory_state_->removeAllocation(id,pointer, time_stamp, event_counter_);
            }
            
            break;
          }
          default:
            //ss << "Unhandled Event " << current_code << ", time_stamp: " << time_stamp << " num unread: " << ring_->getNumUnread() << "\n";
            return false;
          break;
      } //switch(current code)
      if(!recording) {
        std::cout << "finished event: " << current_code << std::endl;
      }
      
      if( last_timestamp > time_stamp) {
        std::cout << "recieved wrong timestamp" << std::endl;
      }
      last_timestamp = time_stamp;
      event_counter_++;
      return true;
}