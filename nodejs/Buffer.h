#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>

class Buffer
{
  public:
   std::size_t WriteMemoryCallback(const char* ptr, size_t size
                              , size_t nmemb);
   std::string result() const;
   void reset();

  private:
   std::vector<char> _buf;
};

//
// Clears the buffer ready for expected new data
//
inline void Buffer::reset() 
{ 
    _buf.clear(); 
}

#endif // BUFFER_H
