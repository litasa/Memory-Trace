#include "decoder.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include "event.h"

Decoder::Decoder() {
    ring_ = new RingBuffer();
    memory_state_ = new MemoryState();
    registerd_events = 0;
    outfile.open(filename);
}

RingBuffer* Decoder::getRingbuffer() {
    return ring_;
    outfile.flush();
    outfile.close();    
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

bool Decoder::decodeString(std::string& ret) {
  uint64_t length;
  if(!decodeValue(length)) {
      return false;
  }
  if(length == 0) {
    ret = "";
    return true;
  }
  if(!ring_->extractString(ret, length)) {
      return false;
  }
  return true;
}

bool Decoder::decodeBool(bool& ret) {
  uint64_t val;
  bool decode = decodeValue(val);
  if(val > 2) {
    return false;
  }
  ret = (bool)val;
  return true;
}

void Decoder::trySteps() {
  ring_->saveRollback();
  size_t current_count = registerd_events;
  size_t current_time = last_timestamp;
  size_t num_throwaway = 0;

  do{
    ring_->saveRollback();
    Event::Event* success = oneStep(false);
    
    if(success == nullptr) {
      ring_->loadRollback();
      uint8_t throwaway;
      ring_->readNext(throwaway);
      num_throwaway++;
    }
    else {
      delete success;      
      break;
    }
  }while(ring_->getNumUnread() > 0);

  if(current_count + 1 != registerd_events && last_timestamp < current_time) {
    throw;
  }

  ring_->loadRollback(); //go back to right before successful event
  ring_->saveRollback(); //and save it

  registerd_events = current_count;
  last_timestamp = current_time;
}

bool Decoder::decodeHeader(std::stringstream& ss,uint64_t& count, uint64_t& current_code, uint64_t& time_stamp, uint64_t& thread_id) {

  if(!decodeValue(count)) {
    if(print_error()) {std::cout << "\treading count failed" << std::endl;}
    return false;
  }

  if(count != registerd_events) {
    if(print_error()) {std::cout << "\tgatherd event != registrd_events" << count << "!=" << registerd_events << std::endl;}
    return false;
  }

  if(!decodeValue(current_code)) {
    if(print_error()) {std::cout << "\treading current_code failed" << std::endl;}
      return false;
  }

  if(!decodeValue(time_stamp)) {
    if(print_error()) {std::cout << "\treading time_stamp failed" << std::endl;}
      return false;
  }

  if(!decodeValue(thread_id)) {
    if(print_error()) {std::cout << "\treading thread_id failed" << std::endl;}
    return false;
  }

  return true;
}

bool Decoder::print_ok() {
  return false; //registerd_events > 133965; //registerd_events > 20686300;
}

bool Decoder::print_error() {
  return print_ok();
}

void Decoder::saveToFile(bool save, Event::Event* event) {
  if(save) {
    std::stringstream ss;
    event->getAsCSV(ss);
    if(outfile.good()) {
      outfile << ss.str().c_str();
      outfile.flush();      
    }
    else {
      std::cout << "file error" << std::endl;
    }
  }
}

Event::Event* Decoder::oneStep(bool save_to_file) {
  size_t current_code;
  size_t count;
  size_t time_stamp;
  size_t thread_id;
  Event::Event* event;
  std::stringstream ss;

  if(!decodeHeader(ss, count, current_code, time_stamp, thread_id)) {
    return nullptr;
  }

  switch(current_code) {
    case Event::Code::BeginStream :
    {
      std::string platform;
      size_t system_frequency;
      size_t stream_magic;

      if(!decodeValue(stream_magic)) {
        if(print_error()) {std::cout << "\t\tDecode stream_magic failed" << std::endl;}
        return nullptr;
      }
      if(!decodeString(platform)) {
        if(print_error()) {std::cout << "\tDecode platform failed" << std::endl;}
        return nullptr;
      }
      if(!decodeValue(system_frequency)) {
        if(print_error()) {std::cout << "\tDecode system_frequency failed" << std::endl;}
        return nullptr;
      }
      event = new Event::InitStream(count, current_code, time_stamp, stream_magic, platform, system_frequency);
      if(print_ok()){event->getAsVerbose(ss);}
      break;
    }

    case Event::Code::EndStream :
      {
        event = new Event::StopStream(count, current_code, time_stamp);
        if(print_ok()){event->getAsVerbose(ss);}
      }
    break;

    case Event::Code::HeapCreate :
    {
      std::string name;
      std::string type;
      size_t id;
      bool own_core;
      if(!decodeValue(id)) {
        if(print_error()) {   std::cout << "decode Add Heap id failed\n";}
        return nullptr;
      }
      if(!decodeString(type)) {
        if(print_error()) { std::cout << "decode Add Heap type failed\n"; }
        return nullptr;
      }
      if(!decodeString(name)) {
        if(print_error()) {   std::cout << "decode Add Heap name failed\n";}
        return nullptr;
      }

      event = new Event::AddHeap(count, current_code, time_stamp, id, type, name);
      if(print_ok()){event->getAsVerbose(ss);}
      break;
    }

    case Event::Code::HeapDestroy :
    {
      size_t id;
      if(!decodeValue(id)) {
        if(print_error()) {   std::cout << "decode Heap Destroy id failed\n";}
        return nullptr;
      }
   
      event = new Event::RemoveHeap(count, current_code, time_stamp, id);
      if(print_ok()){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapAddCore :
    {
      size_t id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error()) {   std::cout << "decode Add Core id failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) {  std::cout << "decode Add Core pointer failed\n";}
        return nullptr;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error()) {  std::cout << "decode Add Core size failed\n";}
        return nullptr;
      }
 
      event = new Event::AddCore(count, current_code, time_stamp, id, pointer, size_bytes);
      if(print_ok()){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapGrowCore :
    {
      size_t id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error()) {   std::cout << "decode HeapGrowCore id failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) {  std::cout << "decode HeapGrowCore pointer failed\n";}
        return nullptr;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error()) {  std::cout << "decode HeapGrowCore size failed\n";}
        return nullptr;
      }
 
      event = new Event::GrowCore(count, current_code, time_stamp, id, pointer, size_bytes);
      if(print_ok()){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapRemoveCore :
    {  
      size_t id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error()) {  std::cout << "decode Remove Core error failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) {  std::cout << "decode Remove Core pointer failed\n";}
        return nullptr;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error()) { std::cout << "decode Remove Core size failed\n";}
        return nullptr;
      }
    
      event = new Event::RemoveCore(count, current_code, time_stamp, id, pointer, size_bytes);
      if(print_ok()){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapShrinkCore :
    {
      size_t id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error()) {   std::cout << "decode HeapShrinkCore id failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) {  std::cout << "decode HeapShrinkCore pointer failed\n";}
        return nullptr;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error()) {  std::cout << "decode HeapShrinkCore size failed\n";}
        return nullptr;
      }
 
      event = new Event::ShrinkCore(count, current_code, time_stamp, id, pointer, size_bytes);
      if(true){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapAllocate:
    {
      size_t id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error()) { std::cout << "decode Heap Allocate id failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) { std::cout << "decode Heap Allocate pointer failed\n";}
        return nullptr;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error()) { std::cout << "decode Heap Allocate size failed\n";}
        return nullptr;
      }

      event = new Event::AddAllocation(count, current_code, time_stamp, id, pointer, size_bytes);
      if(print_ok()) {event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapFree:
    { 
      size_t id;
      size_t pointer;
      if(!decodeValue(id)) {
        if(print_error()) { std::cout << "decode HeapFree id failed\n";}
        return nullptr;
      }
      if(!decodeValue(pointer)) {
        if(print_error()) { std::cout << "decode HeapFree pointer failed\n";}
        return nullptr;
      }

      event = new Event::RemoveAllocation(count, current_code, time_stamp, id, pointer);
      if(print_ok()){event->getAsVerbose(ss);}

      break;
    }

    case Event::Code::HeapFreeAll:
    {
      size_t id;
      if(!decodeValue(id)) {
        if(print_error()) { std::cout << "decode HeapFreeAll id failed \n"; }
        return nullptr;
      }

      event = new Event::RemoveAllAllocations(count, current_code, time_stamp, id);
      if(print_ok()){ event->getAsVerbose(ss); }
      break;
    }

    case Event::Code::EventStart:
    {
        std::string eventName;
        if(!decodeString(eventName)) {
          if(print_error()) { std::cout << "decode EventStart failed\n";}
          return nullptr;
        }
        event = new Event::StartEvent(count, current_code, time_stamp, eventName);
       if(print_ok()){ event->getAsVerbose(ss); }
       break;
    }

    case Event::Code::EventEnd:
    {
        std::string eventName;
        if(!decodeString(eventName)) {
          if(print_error()) { std::cout << "decode EventStart failed\n";}
          return nullptr;
        }
        event = new Event::EndEvent(count, current_code, time_stamp, eventName);
       if(print_ok()){ event->getAsVerbose(ss); }
       break;
    }

    case Event::Code::SetBackingAllocator:
    {
      uint64_t for_heap;
      uint64_t set_to_heap;
      if(!decodeValue(for_heap)) {
        if(print_error()) { std::cout << "decode SetBackingAllocator failed at for_heap\n";}
        return nullptr;
      }
      if(!decodeValue(set_to_heap)) {
        if(print_error()) { std::cout << "decode SetBackingAllocator failed at set_to_heap\n";}
        return nullptr;
      }
      event = new Event::HeapSetBackingAllocator(count,current_code, time_stamp,for_heap,set_to_heap);
      break;
    }
    default:
      std::cout << "Unhandled Event " << current_code << ", time_stamp: " << time_stamp << " num unread: " << ring_->getNumUnread() << "\n";
      return nullptr;
    break;
  } //switch(current code)
  last_timestamp = time_stamp;
  registerd_events++;
  saveToFile(save_to_file, event);
  std::cout << ss.str();
  return event;
}

std::vector<Heap*> Decoder::getMemoryState() {
  return memory_state_->getHeaps();
}

